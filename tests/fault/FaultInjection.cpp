// LD_PRELOAD fault injector for the secure-write tests (issue #18,
// TC-SEC-04). Linux only; loaded into tests/fault/FaultWriter.cpp by
// tests/SecureWriteFaultTests.cpp, never into the game.
//
// SNAKE_FAULT picks one injection. It only touches the temporary file that
// SecureFileStore::writeJson creates ("<file>.tmp.<nonce>"), so every other
// file operation of the process runs normally. That includes the lock
// sidecar "<file>.tmp.<nonce>.lock" that the store may create before the
// temporary file: it is never treated as the temporary file.
//
//   crash-after-open    abort() as soon as the temporary file is created  (STP point a)
//   crash-after-write   abort() when its written bytes are about to be
//                       flushed                                           (b)
//   crash-after-flush   abort() just before the rename                    (c)
//   crash-after-rename  abort() just after the rename                     (d)
//   fail-write          write() to it fails with ENOSPC
//   fail-flush          fsync() of it fails with EIO
//   fail-rename         rename() of it fails with EACCES
//
// The crash modes are real process aborts at those points; the fail modes
// are real failed system calls. Neither needs any change to product code.

#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

namespace {

int temporaryFd = -1;

bool faultIs(const char* name) {
    const char* mode = std::getenv("SNAKE_FAULT");
    return mode != nullptr && std::strcmp(mode, name) == 0;
}

bool isTemporary(const char* path) {
    if (path == nullptr || std::strstr(path, ".tmp.") == nullptr) {
        return false;
    }
    const std::size_t length = std::strlen(path);
    const std::size_t suffix = std::strlen(".lock");
    return length < suffix || std::strcmp(path + length - suffix, ".lock") != 0;
}

// The next definition of `name` after this library (normally libc's).
template <typename Function>
Function nextSymbol(const char* name) {
    const void* symbol = dlsym(RTLD_NEXT, name);
    Function function = nullptr;
    std::memcpy(&function, &symbol, sizeof function);
    return function;
}

using OpenFunction = int (*)(const char*, int, ...);

int openFile(const char* name, const char* path, int flags, mode_t mode) {
    const int fd = nextSymbol<OpenFunction>(name)(path, flags, mode);
    if (fd >= 0 && isTemporary(path) && (flags & O_CREAT) != 0) {
        temporaryFd = fd;
        if (faultIs("crash-after-open")) {
            std::abort();
        }
    }
    return fd;
}

mode_t modeArgument(int flags, va_list arguments) {
    return (flags & O_CREAT) != 0 ? static_cast<mode_t>(va_arg(arguments, int)) : 0;
}

} // namespace

extern "C" {

int open(const char* path, int flags, ...) {
    va_list arguments;
    va_start(arguments, flags);
    const mode_t mode = modeArgument(flags, arguments);
    va_end(arguments);
    return openFile("open", path, flags, mode);
}

int open64(const char* path, int flags, ...) {
    va_list arguments;
    va_start(arguments, flags);
    const mode_t mode = modeArgument(flags, arguments);
    va_end(arguments);
    return openFile("open64", path, flags, mode);
}

ssize_t write(int fd, const void* buffer, size_t count) {
    if (fd >= 0 && fd == temporaryFd && faultIs("fail-write")) {
        errno = ENOSPC;
        return -1;
    }
    return nextSymbol<ssize_t (*)(int, const void*, size_t)>("write")(fd, buffer, count);
}

int fsync(int fd) {
    if (fd >= 0 && fd == temporaryFd) {
        if (faultIs("crash-after-write")) {
            std::abort();
        }
        if (faultIs("fail-flush")) {
            errno = EIO;
            return -1;
        }
    }
    return nextSymbol<int (*)(int)>("fsync")(fd);
}

int close(int fd) {
    if (fd >= 0 && fd == temporaryFd) {
        temporaryFd = -1;
    }
    return nextSymbol<int (*)(int)>("close")(fd);
}

int rename(const char* from, const char* to) noexcept {
    if (isTemporary(from)) {
        if (faultIs("crash-after-flush")) {
            std::abort();
        }
        if (faultIs("fail-rename")) {
            errno = EACCES;
            return -1;
        }
    }
    const int result = nextSymbol<int (*)(const char*, const char*)>("rename")(from, to);
    if (result == 0 && isTemporary(from) && faultIs("crash-after-rename")) {
        std::abort();
    }
    return result;
}

} // extern "C"

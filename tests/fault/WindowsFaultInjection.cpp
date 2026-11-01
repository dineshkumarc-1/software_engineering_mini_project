// Test-only Windows API fault injector for TC-SEC-04.
// This DLL is loaded only by secure_write_fault_writer_windows, a child test
// process. It patches that child's import address table; the game executable
// and production targets are never loaded with this DLL.

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <cstring>
#include <string>

namespace {

constexpr DWORD InjectedCrashExitCode = 0xE241;

using CreateFileWFn = HANDLE(WINAPI*)(LPCWSTR, DWORD, DWORD,
    LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
using WriteFileFn = BOOL(WINAPI*)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
using FlushFileBuffersFn = BOOL(WINAPI*)(HANDLE);
using MoveFileExWFn = BOOL(WINAPI*)(LPCWSTR, LPCWSTR, DWORD);
using CloseHandleFn = BOOL(WINAPI*)(HANDLE);

CreateFileWFn realCreateFileW = nullptr;
WriteFileFn realWriteFile = nullptr;
FlushFileBuffersFn realFlushFileBuffers = nullptr;
MoveFileExWFn realMoveFileExW = nullptr;
CloseHandleFn realCloseHandle = nullptr;

std::wstring faultMode;
std::wstring targetPath;
HANDLE temporaryHandle = INVALID_HANDLE_VALUE;

[[noreturn]] void terminateAtFault() noexcept {
    if (!::TerminateProcess(::GetCurrentProcess(), InjectedCrashExitCode)) {
        ::ExitProcess(InjectedCrashExitCode);
    }
    for (;;) {
        ::Sleep(INFINITE);
    }
}

bool equalsOrdinalIgnoreCase(const wchar_t* left, const std::wstring& right) {
    return left != nullptr &&
        ::CompareStringOrdinal(left, -1, right.c_str(), -1, TRUE) == CSTR_EQUAL;
}

bool isTargetTemporary(LPCWSTR candidate) {
    if (candidate == nullptr || targetPath.empty()) {
        return false;
    }

    const std::wstring prefix = targetPath + L".tmp.";
    const std::wstring path(candidate);
    if (path.size() <= prefix.size() ||
        ::CompareStringOrdinal(path.data(), static_cast<int>(prefix.size()),
            prefix.data(), static_cast<int>(prefix.size()), TRUE) != CSTR_EQUAL) {
        return false;
    }

    const std::wstring suffix = path.substr(prefix.size());
    return suffix.find(L".lock") == std::wstring::npos;
}

template <typename Function>
Function functionFromAddress(FARPROC address) {
    Function function{};
    static_assert(sizeof(function) == sizeof(address), "Unexpected function pointer size");
    std::memcpy(&function, &address, sizeof(function));
    return function;
}

template <typename Function>
FARPROC addressFromFunction(Function function) {
    FARPROC address{};
    static_assert(sizeof(function) == sizeof(address), "Unexpected function pointer size");
    std::memcpy(&address, &function, sizeof(address));
    return address;
}

bool patchImport(HMODULE image, const char* importedName, FARPROC replacement) {
    auto* base = reinterpret_cast<unsigned char*>(image);
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        return false;
    }

    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        return false;
    }

    const auto& directory = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (directory.VirtualAddress == 0 || directory.Size == 0) {
        return false;
    }

    auto* descriptor = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
        base + directory.VirtualAddress);
    bool found = false;

    for (; descriptor->Name != 0; ++descriptor) {
        if (descriptor->OriginalFirstThunk == 0) {
            continue; // This executable is expected to retain its import-name table.
        }

        auto* names = reinterpret_cast<IMAGE_THUNK_DATA*>(
            base + descriptor->OriginalFirstThunk);
        auto* iat = reinterpret_cast<IMAGE_THUNK_DATA*>(
            base + descriptor->FirstThunk);

        for (; names->u1.AddressOfData != 0; ++names, ++iat) {
            if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal)) {
                continue;
            }

            const auto* importByName = reinterpret_cast<const IMAGE_IMPORT_BY_NAME*>(
                base + names->u1.AddressOfData);
            if (std::strcmp(reinterpret_cast<const char*>(importByName->Name),
                    importedName) != 0) {
                continue;
            }

            DWORD oldProtection = 0;
            auto* slot = reinterpret_cast<ULONG_PTR*>(&iat->u1.Function);
            if (!::VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &oldProtection)) {
                return false;
            }

            ULONG_PTR replacementValue = 0;
            static_assert(sizeof(replacementValue) == sizeof(replacement),
                "Unexpected import pointer size");
            std::memcpy(&replacementValue, &replacement, sizeof(replacementValue));
            *slot = replacementValue;

            DWORD ignored = 0;
            (void)::VirtualProtect(slot, sizeof(*slot), oldProtection, &ignored);
            found = true;
        }
    }

    return found;
}

HANDLE WINAPI hookedCreateFileW(LPCWSTR fileName, DWORD desiredAccess,
    DWORD shareMode, LPSECURITY_ATTRIBUTES securityAttributes,
    DWORD creationDisposition, DWORD flagsAndAttributes, HANDLE templateFile) {
    const HANDLE handle = realCreateFileW(fileName, desiredAccess, shareMode,
        securityAttributes, creationDisposition, flagsAndAttributes, templateFile);

    if (handle != INVALID_HANDLE_VALUE && isTargetTemporary(fileName) &&
        creationDisposition == CREATE_NEW) {
        temporaryHandle = handle;
        if (faultMode == L"crash-after-open") {
            terminateAtFault();
        }
    }
    return handle;
}

BOOL WINAPI hookedWriteFile(HANDLE file, LPCVOID buffer, DWORD bytesToWrite,
    LPDWORD bytesWritten, LPOVERLAPPED overlapped) {
    if (file == temporaryHandle && faultMode == L"fail-write") {
        if (bytesWritten != nullptr) {
            *bytesWritten = 0;
        }
        ::SetLastError(ERROR_DISK_FULL);
        return FALSE;
    }

    const BOOL result = realWriteFile(file, buffer, bytesToWrite, bytesWritten, overlapped);
    if (result && file == temporaryHandle && faultMode == L"crash-after-write") {
        terminateAtFault();
    }
    return result;
}

BOOL WINAPI hookedFlushFileBuffers(HANDLE file) {
    if (file == temporaryHandle && faultMode == L"fail-flush") {
        ::SetLastError(ERROR_WRITE_FAULT);
        return FALSE;
    }

    const BOOL result = realFlushFileBuffers(file);
    if (result && file == temporaryHandle && faultMode == L"crash-after-flush") {
        terminateAtFault();
    }
    return result;
}

BOOL WINAPI hookedMoveFileExW(LPCWSTR existingName, LPCWSTR newName, DWORD flags) {
    const bool targetRename = isTargetTemporary(existingName) &&
        equalsOrdinalIgnoreCase(newName, targetPath);

    if (targetRename && faultMode == L"fail-rename") {
        ::SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    const BOOL result = realMoveFileExW(existingName, newName, flags);
    if (result && targetRename && faultMode == L"crash-after-rename") {
        terminateAtFault();
    }
    return result;
}

BOOL WINAPI hookedCloseHandle(HANDLE handle) {
    if (handle == temporaryHandle) {
        temporaryHandle = INVALID_HANDLE_VALUE;
    }
    return realCloseHandle(handle);
}

} // namespace

// Called explicitly by the test child after LoadLibraryW. Doing the patch here
// avoids doing PE parsing or VirtualProtect work from DllMain under loader lock.
extern "C" __declspec(dllexport) BOOL WINAPI SnakeInstallFaultHooks(
    LPCWSTR requestedMode, LPCWSTR requestedTarget) {
    if (requestedMode == nullptr || requestedTarget == nullptr ||
        *requestedMode == L'\0' || *requestedTarget == L'\0') {
        return FALSE;
    }

    faultMode = requestedMode;
    targetPath = requestedTarget;

    HMODULE kernel32 = ::GetModuleHandleW(L"kernel32.dll");
    HMODULE executable = ::GetModuleHandleW(nullptr);
    if (kernel32 == nullptr || executable == nullptr) {
        return FALSE;
    }

    realCreateFileW = functionFromAddress<CreateFileWFn>(
        ::GetProcAddress(kernel32, "CreateFileW"));
    realWriteFile = functionFromAddress<WriteFileFn>(
        ::GetProcAddress(kernel32, "WriteFile"));
    realFlushFileBuffers = functionFromAddress<FlushFileBuffersFn>(
        ::GetProcAddress(kernel32, "FlushFileBuffers"));
    realMoveFileExW = functionFromAddress<MoveFileExWFn>(
        ::GetProcAddress(kernel32, "MoveFileExW"));
    realCloseHandle = functionFromAddress<CloseHandleFn>(
        ::GetProcAddress(kernel32, "CloseHandle"));

    if (realCreateFileW == nullptr || realWriteFile == nullptr ||
        realFlushFileBuffers == nullptr || realMoveFileExW == nullptr ||
        realCloseHandle == nullptr) {
        return FALSE;
    }

    const bool createPatched = patchImport(executable, "CreateFileW",
        addressFromFunction(&hookedCreateFileW));
    const bool writePatched = patchImport(executable, "WriteFile",
        addressFromFunction(&hookedWriteFile));
    const bool flushPatched = patchImport(executable, "FlushFileBuffers",
        addressFromFunction(&hookedFlushFileBuffers));
    const bool movePatched = patchImport(executable, "MoveFileExW",
        addressFromFunction(&hookedMoveFileExW));
    const bool closePatched = patchImport(executable, "CloseHandle",
        addressFromFunction(&hookedCloseHandle));

    return createPatched && writePatched && flushPatched &&
        movePatched && closePatched;
}

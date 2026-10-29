#include "snakextreme/SecureFileStore.hpp"

#include <array>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <fstream>
#include <limits>
#include <random>
#include <string_view>
#include <system_error>
#include <utility>
#include <algorithm>
#include <cctype>
#include <set>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace snake {
namespace {

struct JsonDepthLimitExceeded {};
struct DuplicateJsonKey {};

std::filesystem::path makeTemporaryPath(
    const std::filesystem::path& destination,
    std::uint64_t nonce) {

    const auto parent = destination.has_parent_path()
        ? destination.parent_path()
        : std::filesystem::path{"."};

    const auto name =
        destination.filename().string() +
        ".tmp." + std::to_string(nonce);

    return parent / name;
}

constexpr auto StaleTemporaryAge = std::chrono::hours{24};

bool isDecimalNonce(std::string_view value) {
    if (value.empty()) {
        return false;
    }

    for (const char ch : value) {
        if (ch < '0' || ch > '9') {
            return false;
        }
    }

    return true;
}

// A sidecar lock lets cleanup distinguish a crash leftover from a live
// writer. Windows sharing exclusion and POSIX flock both release on crash.
class TemporaryFileLock {
public:
    TemporaryFileLock() = default;
    TemporaryFileLock(const TemporaryFileLock&) = delete;
    TemporaryFileLock& operator=(const TemporaryFileLock&) = delete;

    ~TemporaryFileLock() {
        release();
    }

    bool createNew(const std::filesystem::path& path) {
        path_ = path;

#ifdef _WIN32
        handle_ = ::CreateFileW(
            path_.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (handle_ == INVALID_HANDLE_VALUE) {
            return false;
        }
#else
        descriptor_ = ::open(
            path_.c_str(),
            O_RDWR | O_CREAT | O_EXCL,
            S_IRUSR | S_IWUSR);

        if (descriptor_ < 0) {
            return false;
        }

        if (::flock(descriptor_, LOCK_EX | LOCK_NB) != 0) {
            (void)::close(descriptor_);
            descriptor_ = -1;
            (void)::unlink(path_.c_str());
            return false;
        }
#endif

        ownsPath_ = true;
        active_ = true;
        return true;
    }

    bool acquireExisting(const std::filesystem::path& path) {
        path_ = path;

#ifdef _WIN32
        handle_ = ::CreateFileW(
            path_.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (handle_ == INVALID_HANDLE_VALUE) {
            return false;
        }
#else
        descriptor_ = ::open(path_.c_str(), O_RDWR);

        if (descriptor_ < 0) {
            return false;
        }

        if (::flock(descriptor_, LOCK_EX | LOCK_NB) != 0) {
            (void)::close(descriptor_);
            descriptor_ = -1;
            return false;
        }
#endif

        ownsPath_ = false;
        active_ = true;
        return true;
    }

    [[nodiscard]] bool ownsPath() const noexcept {
        return ownsPath_;
    }

    void release() noexcept {
        if (!active_) {
            return;
        }

#ifdef _WIN32
        (void)::CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
#else
        (void)::flock(descriptor_, LOCK_UN);
        (void)::close(descriptor_);
        descriptor_ = -1;
#endif

        active_ = false;

        if (ownsPath_) {
            std::error_code ignored;
            std::filesystem::remove(path_, ignored);
            ownsPath_ = false;
        }
    }

private:
    std::filesystem::path path_;
    bool ownsPath_{false};
    bool active_{false};

#ifdef _WIN32
    HANDLE handle_{INVALID_HANDLE_VALUE};
#else
    int descriptor_{-1};
#endif
};

bool isStaleRegularTemporary(
    const std::filesystem::path& path,
    std::filesystem::file_time_type::clock::time_point now) {

    std::error_code statusError;
    const auto status = std::filesystem::symlink_status(path, statusError);

    if (statusError ||
        std::filesystem::is_symlink(status) ||
        !std::filesystem::is_regular_file(status)) {
        return false;
    }

    std::error_code timeError;
    const auto modified = std::filesystem::last_write_time(path, timeError);

    return !timeError && now - modified >= StaleTemporaryAge;
}

// Stale-temp policy: only exact "<destination>.tmp.<decimal nonce>" sibling
// files older than 24 hours are candidates. A lock is acquired/reserved
// before deletion, so an active writer's temporary file is never removed.
// Symlinks, non-regular files, unrelated names and fresh files are ignored.
void removeStaleTemporaryFiles(
    const std::filesystem::path& destination) noexcept {

    try {
        if (destination.empty()) {
            return;
        }

        const auto parent = destination.has_parent_path()
            ? destination.parent_path()
            : std::filesystem::path{"."};
        const auto baseName = destination.filename().string();

        if (baseName.empty() || baseName == "." || baseName == "..") {
            return;
        }

        std::error_code parentError;
        const auto parentStatus =
            std::filesystem::symlink_status(parent, parentError);

        if (parentError ||
            std::filesystem::is_symlink(parentStatus) ||
            !std::filesystem::is_directory(parentStatus)) {
            return;
        }

        std::error_code iteratorError;
        std::filesystem::directory_iterator iterator(parent, iteratorError);
        const std::filesystem::directory_iterator end;

        if (iteratorError) {
            return;
        }

        const std::string prefix = baseName + ".tmp.";
        const auto now =
            std::filesystem::file_time_type::clock::now();

        for (; iterator != end; iterator.increment(iteratorError)) {
            if (iteratorError) {
                break;
            }

            const auto candidate = iterator->path();
            const std::string candidateName =
                candidate.filename().string();

            if (candidateName.compare(0U, prefix.size(), prefix) != 0) {
                continue;
            }

            const std::string nonce = candidateName.substr(prefix.size());
            if (!isDecimalNonce(nonce) ||
                !isStaleRegularTemporary(candidate, now)) {
                continue;
            }

            auto lockPath = candidate;
            lockPath += ".lock";

            std::error_code lockStatusError;
            const auto lockStatus =
                std::filesystem::symlink_status(lockPath, lockStatusError);

            bool existingLock = false;
            TemporaryFileLock lease;

            if (!lockStatusError &&
                std::filesystem::exists(lockStatus)) {
                if (std::filesystem::is_symlink(lockStatus) ||
                    !std::filesystem::is_regular_file(lockStatus) ||
                    !lease.acquireExisting(lockPath)) {
                    continue;
                }

                existingLock = true;
            } else if (
                (lockStatusError ==
                    std::errc::no_such_file_or_directory) ||
                (!lockStatusError &&
                    !std::filesystem::exists(lockStatus))) {
                // Reserve this nonce so a concurrently starting writer cannot
                // create the same temporary file while it is being cleaned.
                if (!lease.createNew(lockPath)) {
                    continue;
                }
            } else {
                continue;
            }

            // Recheck both symlink status and age after acquiring the lock.
            if (!isStaleRegularTemporary(candidate, now)) {
                lease.release();
                continue;
            }

            std::error_code removeError;
            const bool removed =
                std::filesystem::remove(candidate, removeError);

            if (removed && !removeError && existingLock) {
                lease.release();
                std::error_code lockRemoveError;
                std::filesystem::remove(lockPath, lockRemoveError);
            } else {
                // If this cleanup created the lock marker, release() removes it.
                // If a pre-existing lock remains, keep it unless the temp was
                // removed while this cleanup held the lock.
                lease.release();
            }
        }
    } catch (...) {
        // Cleanup is best-effort; a cleanup error must never block normal I/O.
    }
}

// Create a new temporary file exclusively.
// POSIX files are readable/writable only by their owner.
bool writeAndFlushNewFile(
    const std::filesystem::path& path,
    std::string_view bytes) {

#ifdef _WIN32

    HANDLE file = ::CreateFileW(
        path.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }

    bool success = true;
    std::size_t offset = 0;

    while (offset < bytes.size()) {
        const auto remaining = bytes.size() - offset;

        const DWORD chunk = static_cast<DWORD>(
            remaining >
                    static_cast<std::size_t>(
                        std::numeric_limits<DWORD>::max())
                ? std::numeric_limits<DWORD>::max()
                : remaining);

        DWORD written = 0;

        if (!::WriteFile(
                file,
                bytes.data() + offset,
                chunk,
                &written,
                nullptr) ||
            written == 0) {
            success = false;
            break;
        }

        offset += written;
    }

    if (success && !::FlushFileBuffers(file)) {
        success = false;
    }

    if (!::CloseHandle(file)) {
        success = false;
    }

    if (!success) {
        std::error_code ignored;
        std::filesystem::remove(path, ignored);
    }

    return success;

#else

    const int file = ::open(
        path.c_str(),
        O_WRONLY | O_CREAT | O_EXCL,
        S_IRUSR | S_IWUSR);

    if (file < 0) {
        return false;
    }

    bool success = true;
    std::size_t offset = 0;

    while (offset < bytes.size()) {
        const auto written = ::write(
            file,
            bytes.data() + offset,
            bytes.size() - offset);

        if (written < 0 && errno == EINTR) {
            continue;
        }

        if (written <= 0) {
            success = false;
            break;
        }

        offset += static_cast<std::size_t>(written);
    }

    if (success && ::fsync(file) != 0) {
        success = false;
    }

    if (::close(file) != 0) {
        success = false;
    }

    if (!success) {
        (void)::unlink(path.c_str());
    }

    return success;

#endif
}

bool replaceAtomically(
    const std::filesystem::path& temporary,
    const std::filesystem::path& destination) {

#ifdef _WIN32

    return ::MoveFileExW(
        temporary.c_str(),
        destination.c_str(),
        MOVEFILE_REPLACE_EXISTING |
            MOVEFILE_WRITE_THROUGH) != 0;

#else

    if (std::rename(
            temporary.c_str(),
            destination.c_str()) != 0) {
        return false;
    }

    // Best-effort flush of the parent directory metadata.
    const auto parent = destination.has_parent_path()
        ? destination.parent_path()
        : std::filesystem::path{"."};

    const int directory = ::open(parent.c_str(), O_RDONLY);

    if (directory >= 0) {
        (void)::fsync(directory);
        (void)::close(directory);
    }

    return true;

#endif
}

StoreReadResult readFailure(
    StoreReadStatus status,
    std::string message) {

    return StoreReadResult{
        status,
        nlohmann::json{},
        std::move(message)
    };
}

} // namespace

SecureFileStore::SecureFileStore(
    std::filesystem::path filePath)
    : filePath_(std::move(filePath)) {}

StoreReadResult SecureFileStore::readJson(
    std::size_t maxNestingDepth) const {
    if (filePath_.empty()) {
        return readFailure(
            StoreReadStatus::IoError,
            "No file path was configured.");
    }
        if (maxNestingDepth == 0U) {
        return readFailure(
            StoreReadStatus::IoError,
            "JSON nesting limit must be positive.");
    }

    // Callers may use a stricter limit, never a higher one.
    const auto nestingLimit = std::min(
        maxNestingDepth,
        MaxJsonNestingDepth);

    removeStaleTemporaryFiles(filePath_);

    // Inspect the path without following a final-component symlink.
    std::error_code statusError;
    const auto fileStatus =
        std::filesystem::symlink_status(filePath_, statusError);

    if (statusError) {
        if (statusError == std::errc::no_such_file_or_directory) {
            return readFailure(
                StoreReadStatus::NotFound,
                "File does not exist.");
        }

        return readFailure(
            StoreReadStatus::IoError,
            "Could not inspect the file path.");
    }

    if (!std::filesystem::exists(fileStatus)) {
        return readFailure(
            StoreReadStatus::NotFound,
            "File does not exist.");
    }

    if (std::filesystem::is_symlink(fileStatus) ||
        !std::filesystem::is_regular_file(fileStatus)) {
        return readFailure(
            StoreReadStatus::UnsafeFile,
            "Only regular files are allowed; symlinks are rejected.");
    }

    std::ifstream input(filePath_, std::ios::binary);

    if (!input) {
        std::error_code ec;
        const bool exists =
            std::filesystem::exists(filePath_, ec);

        if (!ec && !exists) {
            return readFailure(
                StoreReadStatus::NotFound,
                "File does not exist.");
        }

        return readFailure(
            StoreReadStatus::IoError,
            "Could not open the file for reading.");
    }

    std::string bytes;
    bytes.reserve(MaxFileSizeBytes);

    std::array<char, 4096> buffer{};

    while (input) {
        input.read(
            buffer.data(),
            static_cast<std::streamsize>(buffer.size()));

        const auto count = input.gcount();

        if (count > 0) {
            const auto amount =
                static_cast<std::size_t>(count);

            // This check allows exactly 64 KiB,
            // but rejects even one byte more.
            if (amount >
                MaxFileSizeBytes - bytes.size()) {
                return readFailure(
                    StoreReadStatus::TooLarge,
                    "JSON file exceeds the 64 KiB limit.");
            }

            bytes.append(buffer.data(), amount);
        }
    }

    if (input.bad() || (!input.eof() && input.fail())) {
        return readFailure(
            StoreReadStatus::IoError,
            "Failed while reading the file.");
    }
const bool isEmpty = std::all_of(
    bytes.begin(),
    bytes.end(),
    [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });

if (isEmpty) {
    return readFailure(
        StoreReadStatus::EmptyFile,
        "File contains no JSON data.");
}

    try {
        std::vector<std::set<std::string>> seenKeys;

        const auto callback = [&seenKeys, nestingLimit](
            int depth,
            nlohmann::json::parse_event_t event,
            nlohmann::json& parsed) -> bool {

            using Event = nlohmann::json::parse_event_t;

            // Reject structures nested beyond the configured limit.
            if ((event == Event::object_start ||
                 event == Event::array_start) &&
                static_cast<std::size_t>(depth) >= nestingLimit) {
                throw JsonDepthLimitExceeded{};
            }

            // Object keys are reported one level deeper than
            // the corresponding object_start event.
            if (event == Event::object_start) {
                const auto keysDepth =
                    static_cast<std::size_t>(depth) + 1U;

                if (seenKeys.size() <= keysDepth) {
                    seenKeys.resize(keysDepth + 1U);
                }

                seenKeys[keysDepth].clear();
            }

            // Reject duplicate keys before the parser can
            // silently replace an earlier value.
            if (event == Event::key) {
                const auto keysDepth =
                    static_cast<std::size_t>(depth);

                if (seenKeys.size() <= keysDepth) {
                    seenKeys.resize(keysDepth + 1U);
                }

                const auto& key =
                    parsed.get_ref<const std::string&>();

                if (!seenKeys[keysDepth].insert(key).second) {
                    throw DuplicateJsonKey{};
                }
            }

            // Release the key set when the object closes.
            if (event == Event::object_end) {
                const auto keysDepth =
                    static_cast<std::size_t>(depth) + 1U;

                if (seenKeys.size() > keysDepth) {
                    seenKeys[keysDepth].clear();
                }
            }

            return true;
        };

        auto parsed = nlohmann::json::parse(bytes, callback);

        return StoreReadResult{
            StoreReadStatus::Ok,
            std::move(parsed),
            {}
        };
    } catch (const JsonDepthLimitExceeded&) {
        return readFailure(
            StoreReadStatus::UnsafeJson,
            "JSON nesting exceeds the permitted depth.");
    } catch (const DuplicateJsonKey&) {
        return readFailure(
            StoreReadStatus::UnsafeJson,
            "JSON contains a duplicate object key.");
    } catch (const nlohmann::json::exception&) {
        return readFailure(
            StoreReadStatus::InvalidJson,
            "File does not contain valid JSON.");
    }

}

bool SecureFileStore::writeJson(
    const nlohmann::json& value) const {

    if (filePath_.empty()) {
        return false;
    }

    // Reject existing symlinks and non-regular destinations.
    // A destination that does not exist yet is allowed.
    std::error_code statusError;
    const auto destinationStatus =
        std::filesystem::symlink_status(filePath_, statusError);

    if (statusError) {
        if (statusError !=
            std::errc::no_such_file_or_directory) {
            return false;
        }
    } else if (
        std::filesystem::is_symlink(destinationStatus) ||
        (std::filesystem::exists(destinationStatus) &&
         !std::filesystem::is_regular_file(destinationStatus))) {
        return false;
    }


    std::string bytes;

    try {
        bytes = value.dump(2);
    } catch (const nlohmann::json::exception&) {
        return false;
    }

    // Enforce the cap on serialized output too.
    if (bytes.size() > MaxFileSizeBytes) {
        return false;
    }

    const auto parent = filePath_.has_parent_path()
        ? filePath_.parent_path()
        : std::filesystem::path{"."};

    std::error_code ec;
    std::filesystem::create_directories(parent, ec);

    if (ec) {
        return false;
    }

    removeStaleTemporaryFiles(filePath_);

    static std::atomic<std::uint64_t> sequence{0};

    // Every writer holds an exclusive sidecar lock from before temp creation
    // through replacement. Cleanup skips active locks and only reclaims stale
    // crash leftovers.
    for (int attempt = 0; attempt < 8; ++attempt) {
        const auto clockValue =
            static_cast<std::uint64_t>(
                std::chrono::steady_clock::now()
                    .time_since_epoch()
                    .count());

        std::uint64_t randomValue = 0;
        try {
            std::random_device random;
            randomValue = static_cast<std::uint64_t>(random());
        } catch (...) {
            // Clock, sequence and exclusive creation still protect the path.
        }

        const auto nonce =
            clockValue ^
            (randomValue << 32U) ^
            sequence.fetch_add(1, std::memory_order_relaxed);

        const auto temporary = makeTemporaryPath(filePath_, nonce);
        auto lockPath = temporary;
        lockPath += ".lock";

        TemporaryFileLock writerLease;
        if (!writerLease.createNew(lockPath)) {
            continue;
        }

        if (!writeAndFlushNewFile(temporary, bytes)) {
            continue;
        }

        // Only replace after the whole temporary file has been written/flushed.
        if (!replaceAtomically(temporary, filePath_)) {
            std::error_code removeError;
            std::filesystem::remove(temporary, removeError);
            return false;
        }

        // The lease is released on scope exit and removes its own lock marker.
        return true;
    }

    return false;
}

const std::filesystem::path&
SecureFileStore::path() const noexcept {
    return filePath_;
}

} // namespace snake
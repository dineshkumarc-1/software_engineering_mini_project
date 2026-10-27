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

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace snake {
namespace {

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

StoreReadResult SecureFileStore::readJson() const {
    if (filePath_.empty()) {
        return readFailure(
            StoreReadStatus::IoError,
            "No file path was configured.");
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

    try {
        auto parsed = nlohmann::json::parse(bytes);

        return StoreReadResult{
            StoreReadStatus::Ok,
            std::move(parsed),
            {}
        };
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

    static std::atomic<std::uint64_t> sequence{0};

    bool staged = false;
    std::filesystem::path temporary;

    // Try several exclusive temporary-file names to
    // handle an unlikely filename collision.
    for (int attempt = 0; attempt < 8; ++attempt) {
        const auto clockValue =
            static_cast<std::uint64_t>(
                std::chrono::steady_clock::now()
                    .time_since_epoch()
                    .count());

        std::uint64_t randomValue = 0;

        try {
            std::random_device random;
            randomValue =
                static_cast<std::uint64_t>(random());
        } catch (...) {
            // The clock and sequence still provide a
            // candidate name; exclusive creation protects it.
        }

        const auto nonce =
            clockValue ^
            (randomValue << 32U) ^
            sequence.fetch_add(
                1, std::memory_order_relaxed);

        temporary = makeTemporaryPath(filePath_, nonce);

        if (writeAndFlushNewFile(temporary, bytes)) {
            staged = true;
            break;
        }
    }

    if (!staged) {
        return false;
    }

    // Only replace the destination after the entire
    // temporary file has been written and flushed.
    if (!replaceAtomically(temporary, filePath_)) {
        std::filesystem::remove(temporary, ec);
        return false;
    }

    return true;
}

const std::filesystem::path&
SecureFileStore::path() const noexcept {
    return filePath_;
}

} // namespace snake
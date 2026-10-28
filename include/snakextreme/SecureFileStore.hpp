#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>

namespace snake {

enum class StoreReadStatus {
    Ok,
    NotFound,
    EmptyFile,
    TooLarge,
    InvalidJson,
    UnsafeJson,
    UnsafeFile,
    IoError
};

struct StoreReadResult {
    StoreReadStatus status{StoreReadStatus::IoError};
    nlohmann::json value{};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return status == StoreReadStatus::Ok;
    }
};

class SecureFileStore {
public:
    static constexpr std::size_t MaxFileSizeBytes = 64U * 1024U;

    explicit SecureFileStore(std::filesystem::path filePath);

    [[nodiscard]] StoreReadResult readJson() const;

    [[nodiscard]] bool writeJson(
        const nlohmann::json& value) const;

    [[nodiscard]] const std::filesystem::path&
    path() const noexcept;

private:
    std::filesystem::path filePath_;
};

} // namespace snake
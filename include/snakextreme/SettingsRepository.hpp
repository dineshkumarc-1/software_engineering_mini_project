#pragma once
#include "snakextreme/Settings.hpp"
#include <filesystem>
#include <string>

namespace snake {
struct SettingsLoadResult {
    Settings settings{};
    bool usedDefaults{false};
    std::string warning;
};
// C12 schema/validation adapter over C13 SecureFileStore.
// Uses a stricter JSON nesting limit of 8 and retains stable fallback warnings.
class SettingsRepository {
public:
    explicit SettingsRepository(std::filesystem::path path = "config.json") : path_(std::move(path)) {}
    SettingsLoadResult load() const;
    bool save(const Settings& settings, std::string& error) const;
private:
    std::filesystem::path path_;
};
} // namespace snake

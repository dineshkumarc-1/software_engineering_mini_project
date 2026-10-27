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
// C12 interim file adapter; coordinate with C13 (#15) before final integration.
// Bounded read, validated schema, exclusive sibling temp + atomic replacement.
// No claim of disk-fsync durability or final C13 security acceptance.
class SettingsRepository {
public:
    explicit SettingsRepository(std::filesystem::path path = "config.json") : path_(std::move(path)) {}
    SettingsLoadResult load() const;
    bool save(const Settings& settings, std::string& error) const;
private:
    std::filesystem::path path_;
};
} // namespace snake

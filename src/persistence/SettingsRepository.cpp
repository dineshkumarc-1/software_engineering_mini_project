#include "snakextreme/SettingsRepository.hpp"
#include "snakextreme/SecureFileStore.hpp"
#include <nlohmann/json.hpp>
#include <array>
#include <exception>

namespace snake {
namespace {
using Json = nlohmann::json;
SettingsLoadResult defaults(const char* code) { return {Settings{}, true, code}; }
bool integerInRange(const Json& j, int low, int high) {
    return j.is_number_integer() && j >= low && j <= high;
}
}
SettingsLoadResult SettingsRepository::load() const {
    try {
        // C13 owns bounded reads, duplicate-key/depth checks and safe paths.
        const auto loaded = SecureFileStore(path_).readJson(8);
        switch (loaded.status) {
        case StoreReadStatus::Ok: break;
        case StoreReadStatus::NotFound: return defaults("E_FILE_MISSING: defaults loaded");
        case StoreReadStatus::TooLarge: return defaults("E_FILE_TOO_LARGE: defaults loaded");
        case StoreReadStatus::EmptyFile:
        case StoreReadStatus::InvalidJson:
        case StoreReadStatus::UnsafeJson: return defaults("E_PARSE: defaults loaded");
        case StoreReadStatus::UnsafeFile:
        case StoreReadStatus::IoError: return defaults("E_READ_FAILED: defaults loaded");
        }
        const auto& json = loaded.value;
        if (!json.is_object() || !json.contains("version") || !integerInRange(json["version"], 1, 1))
            return defaults("E_SCHEMA: defaults loaded");
        Settings settings;
        constexpr std::array<const char*, 4> names{"key_up", "key_down", "key_left", "key_right"};
        for (std::size_t i = 0; i < names.size(); ++i) {
            if (!json.contains(names[i]) || !integerInRange(json[names[i]], 0, 100)) return defaults("E_SCHEMA: defaults loaded");
            settings.keys[i] = json[names[i]].get<int>();
        }
        for (const auto* name : {"music_volume", "sfx_volume"})
            if (!json.contains(name) || !integerInRange(json[name], 0, 100)) return defaults("E_SCHEMA: defaults loaded");
        settings.musicVolume = json["music_volume"].get<int>();
        settings.sfxVolume = json["sfx_volume"].get<int>();
        if (!json.contains("theme") || !json["theme"].is_string()) return defaults("E_SCHEMA: defaults loaded");
        if (json["theme"] == "Classic") settings.theme = Theme::Classic;
        else if (json["theme"] == "Neon") settings.theme = Theme::Neon;
        else if (json["theme"] == "Retro") settings.theme = Theme::Retro;
        else return defaults("E_SCHEMA: defaults loaded");
        if (!validSettings(settings)) return defaults("E_SCHEMA: defaults loaded");
        return {settings, false, {}};
    } catch (const std::exception&) { return defaults("E_PARSE: defaults loaded"); }
}
bool SettingsRepository::save(const Settings& settings, std::string& error) const {
    error.clear();
    if (!validSettings(settings)) { error = "E_SCHEMA: settings not saved"; return false; }
    try {
        // Preserve the existing Settings save contract: never create parents.
        const auto parent = path_.has_parent_path() ? path_.parent_path() : std::filesystem::path{"."};
        std::error_code ec;
        if (!std::filesystem::is_directory(parent, ec) || ec) {
            error = "E_WRITE_FAILED: existing config retained";
            return false;
        }
        const Json json{{"version", 1}, {"key_up", settings.keys[0]}, {"key_down", settings.keys[1]},
            {"key_left", settings.keys[2]}, {"key_right", settings.keys[3]},
            {"music_volume", settings.musicVolume}, {"sfx_volume", settings.sfxVolume},
            {"theme", themeName(settings.theme)}};
        if (SecureFileStore(path_).writeJson(json)) return true;
    } catch (const std::exception&) {
        // Preserve C12's stable error contract; do not expose player paths.
    }
    error = "E_WRITE_FAILED: existing config retained";
    return false;
}
} // namespace snake

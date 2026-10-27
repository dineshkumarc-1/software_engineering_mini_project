#if defined(_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "snakextreme/SettingsRepository.hpp"
#include <nlohmann/json.hpp>
#include <array>
#include <cstdio>
#include <fstream>
#include <memory>
#include <random>
#include <set>
#include <stdexcept>
#include <system_error>

namespace snake {
namespace {
using Json = nlohmann::json;
constexpr std::size_t MaxBytes = 65536;
SettingsLoadResult defaults(const char* code) { return {Settings{}, true, code}; }
bool integerInRange(const Json& j, int low, int high) {
    return j.is_number_integer() && j >= low && j <= high;
}
}
SettingsLoadResult SettingsRepository::load() const {
    try {
        std::error_code ec;
        const auto status = std::filesystem::symlink_status(path_, ec);
        if (status.type() == std::filesystem::file_type::not_found) return defaults("E_FILE_MISSING: defaults loaded");
        if (ec || !std::filesystem::is_regular_file(status)) return defaults("E_READ_FAILED: defaults loaded");
        std::ifstream input(path_, std::ios::binary);
        if (!input) return defaults("E_READ_FAILED: defaults loaded");
        std::array<char, MaxBytes + 1> buffer{};
        input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        if (input.bad()) return defaults("E_READ_FAILED: defaults loaded");
        const auto length = static_cast<std::size_t>(input.gcount());
        if (length > MaxBytes) return defaults("E_FILE_TOO_LARGE: defaults loaded");
        std::set<std::string> keys;
        const auto callback = [&keys](int depth, Json::parse_event_t event, Json& value) {
            if (depth > 8) throw std::runtime_error("depth limit");
            if (event == Json::parse_event_t::key && depth == 1 &&
                !keys.insert(value.get<std::string>()).second) throw std::runtime_error("duplicate key");
            return true;
        };
        const auto json = Json::parse(buffer.data(), buffer.data() + length, callback);
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
    std::filesystem::path temporary;
    bool ownsTemporary = false;
    try {
        const Json json{{"version", 1}, {"key_up", settings.keys[0]}, {"key_down", settings.keys[1]},
            {"key_left", settings.keys[2]}, {"key_right", settings.keys[3]},
            {"music_volume", settings.musicVolume}, {"sfx_volume", settings.sfxVolume},
            {"theme", themeName(settings.theme)}};
        const std::string bytes = json.dump(2) + "\n";
        std::random_device random;
        std::unique_ptr<std::FILE, decltype(&std::fclose)> file(nullptr, &std::fclose);
        for (int attempt = 0; attempt < 8 && !file; ++attempt) {
            temporary = path_; temporary += ".tmp-" + std::to_string(random());
#if defined(_WIN32)
            std::FILE* handle = nullptr;
            if (_wfopen_s(&handle, temporary.c_str(), L"wbx") == 0) file.reset(handle);
#else
            file.reset(std::fopen(temporary.c_str(), "wbx"));
#endif
        }
        // Never remove a path unless this process successfully created it.
        if (!file) { error = "E_WRITE_FAILED: could not create temporary file"; return false; }
        ownsTemporary = true;
        const bool written = std::fwrite(bytes.data(), 1, bytes.size(), file.get()) == bytes.size();
        const bool flushed = std::fflush(file.get()) == 0;
        const bool closed = std::fclose(file.release()) == 0;
        if (!written || !flushed || !closed) throw std::runtime_error("write failed");
#if defined(_WIN32)
        if (!MoveFileExW(temporary.c_str(), path_.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            throw std::runtime_error("replace failed");
#else
        std::filesystem::rename(temporary, path_);
#endif
        return true;
    } catch (const std::exception&) {
        if (ownsTemporary) {
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
        }
        error = "E_WRITE_FAILED: existing config retained";
        return false;
    }
}
} // namespace snake

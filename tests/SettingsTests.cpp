#include "snakextreme/Settings.hpp"
#include "snakextreme/SettingsRepository.hpp"
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
using namespace snake;
namespace {
struct Fixture {
    std::filesystem::path dir = std::filesystem::temp_directory_path() /
        ("snakextreme-settings-" + std::to_string(std::random_device{}()));
    std::filesystem::path path = dir / "config.json";
    Fixture() { std::filesystem::create_directories(dir); }
    ~Fixture() { std::error_code ec; std::filesystem::remove_all(dir, ec); }
    void write(const std::string& bytes) { std::ofstream out(path, std::ios::binary); out << bytes; }
    std::string read() { std::ifstream in(path); return {std::istreambuf_iterator<char>(in), {}}; }
};
nlohmann::json validJson() {
    return {{"version", 1}, {"key_up", 73}, {"key_down", 74}, {"key_left", 71},
        {"key_right", 72}, {"music_volume", 70}, {"sfx_volume", 70}, {"theme", "Classic"}};
}
}
TEST_CASE("Settings defaults match approved SRS and SAD", "[settings]") {
    Settings s;
    CHECK(s.keys == std::array<int, 4>{73, 74, 71, 72});
    CHECK(s.musicVolume == 70); CHECK(s.sfxVolume == 70);
    CHECK(s.theme == Theme::Classic); CHECK(validSettings(s));
}
TEST_CASE("Remaps preserve WASD and remove old primaries", "[settings][TC-SET-01]") {
    Settings s; s.keys = {8, 10, 9, 11}; // I K J L
    REQUIRE(validSettings(s));
    constexpr int aliases[] = {22, 18, 0, 3};
    for (std::size_t i = 0; i < 4; ++i) {
        CHECK(movementForKey(s, s.keys[i]) == i);
        CHECK(movementForKey(s, aliases[i]) == i);
        CHECK_FALSE(movementForKey(s, Settings{}.keys[i]));
    }
    CHECK_FALSE(movementForKey(s, -1)); CHECK_FALSE(movementForKey(s, 12));
}
TEST_CASE("Duplicate, reserved and contradictory alias bindings are rejected", "[settings][TC-SET-01]") {
    for (int key : {-1, 12, 26, 58, 1000, 74, 18, 0, 3}) {
        SettingsEditor editor; editor.open(Settings{}); editor.activate();
        CHECK_FALSE(editor.bind(key)); CHECK(editor.capturing()); CHECK(editor.value() == Settings{});
    }
    SettingsEditor editor; editor.open(Settings{}); editor.activate();
    REQUIRE(editor.bind(8)); CHECK_FALSE(editor.capturing()); CHECK(editor.value().keys[0] == 8);
    editor.move(1); editor.activate(); CHECK_FALSE(editor.bind(8));
    editor.cancelCapture(); CHECK_FALSE(editor.capturing());
}
TEST_CASE("Editor clamps independent volume channels", "[settings][TC-SET-02]") {
    SettingsEditor editor; editor.open(Settings{});
    editor.selectRow(4); editor.setVolume(-25); CHECK(editor.value().musicVolume == 0);
    CHECK(editor.value().sfxVolume == 70);
    editor.setVolume(125); CHECK(editor.value().musicVolume == 100);
    editor.adjust(-5); CHECK(editor.value().musicVolume == 95);
    editor.selectRow(5); editor.setVolume(0); CHECK(editor.value().sfxVolume == 0);
    editor.adjust(100); CHECK(editor.value().sfxVolume == 100);
    CHECK(editor.value().musicVolume == 95);
}
TEST_CASE("Themes cycle and reset remains a draft", "[settings][TC-SET-03]") {
    Settings original; SettingsEditor editor; editor.open(original);
    editor.selectRow(6); editor.adjust(1); CHECK(editor.value().theme == Theme::Neon);
    editor.adjust(1); CHECK(editor.value().theme == Theme::Retro);
    editor.adjust(1); CHECK(editor.value().theme == Theme::Classic);
    editor.adjust(-1); CHECK(editor.value().theme == Theme::Retro);
    CHECK(original == Settings{});
    editor.selectRow(7); editor.activate(); CHECK(editor.value() == Settings{});
}
TEST_CASE("Editor selection and capture do not mutate unrelated fields", "[settings]") {
    SettingsEditor editor; editor.open(Settings{}); editor.move(-1); CHECK(editor.row() == 9);
    editor.move(1); CHECK(editor.row() == 0);
    editor.activate(); editor.move(1); editor.selectRow(5); editor.adjust(5); editor.setVolume(0);
    CHECK(editor.row() == 0); CHECK(editor.value() == Settings{});
    editor.cancelCapture(); editor.selectRow(100); CHECK(editor.row() == 0);
    editor.open(Settings{}); CHECK(editor.message().empty()); CHECK_FALSE(editor.capturing());
}
TEST_CASE("Settings persist all fields across fresh repository instances", "[settings][TC-SET-04]") {
    Fixture f; Settings s; s.keys = {8, 10, 9, 11}; s.musicVolume = 0; s.sfxVolume = 100;
    for (auto theme : {Theme::Classic, Theme::Neon, Theme::Retro}) {
        s.theme = theme; std::string error;
        REQUIRE(SettingsRepository(f.path).save(s, error)); CHECK(error.empty());
        const auto loaded = SettingsRepository(f.path).load();
        CHECK_FALSE(loaded.usedDefaults); CHECK(loaded.settings == s);
        const auto json = nlohmann::json::parse(f.read()); CHECK(json.size() == 8);
        CHECK(json["version"] == 1); CHECK(json["key_up"] == 8);
        CHECK(json["music_volume"] == 0); CHECK(json["sfx_volume"] == 100);
    }
    CHECK(std::distance(std::filesystem::directory_iterator(f.dir), std::filesystem::directory_iterator{}) == 1);
}
TEST_CASE("Missing file and malformed config fall back without throwing", "[settings][TC-REL-01]") {
    Fixture f;
    CHECK(SettingsRepository(f.path).load().usedDefaults);
    CHECK_FALSE(std::filesystem::exists(f.path)); // application regenerates after reporting
    for (const auto& bytes : {"", "{", "[]", "null", "{}", "{\"version\":1,}", "{} trailing", "{\"version\":1,\"version\":1}"}) {
        f.write(bytes); const auto loaded = SettingsRepository(f.path).load();
        CHECK(loaded.usedDefaults); CHECK(loaded.settings == Settings{}); CHECK_FALSE(loaded.warning.empty());
    }
}
TEST_CASE("Settings schema validates every mandatory field", "[settings][TC-REL-01]") {
    Fixture f;
    const auto base = validJson();
    for (const auto& item : base.items()) {
        auto json = base; json.erase(item.key()); f.write(json.dump());
        CHECK(SettingsRepository(f.path).load().usedDefaults);
        json = base; json[item.key()] = nullptr; f.write(json.dump());
        CHECK(SettingsRepository(f.path).load().usedDefaults);
    }
    for (const auto& name : {"music_volume", "sfx_volume", "version", "key_up"}) {
        for (const auto& invalid : {nlohmann::json(-1), nlohmann::json(101), nlohmann::json(1.5),
              nlohmann::json(true), nlohmann::json("70"), nlohmann::json(18446744073709551615ull)}) {
            auto json = base; json[name] = invalid; f.write(json.dump());
            CHECK(SettingsRepository(f.path).load().usedDefaults);
        }
    }
}
TEST_CASE("Invalid theme, duplicate keys and alias conflicts fall back", "[settings]") {
    Fixture f; auto json = validJson();
    json["theme"] = "neon"; f.write(json.dump()); CHECK(SettingsRepository(f.path).load().usedDefaults);
    json = validJson(); json["key_up"] = 74; f.write(json.dump()); CHECK(SettingsRepository(f.path).load().usedDefaults);
    json["key_up"] = 18; f.write(json.dump()); CHECK(SettingsRepository(f.path).load().usedDefaults);
    json["key_up"] = 12; f.write(json.dump()); CHECK(SettingsRepository(f.path).load().usedDefaults);
}
TEST_CASE("Unknown fields ignored and exactly 64 KiB accepted", "[settings]") {
    Fixture f; auto json = validJson(); json["future"] = {{"value", true}};
    auto bytes = json.dump(); bytes.resize(65536, ' '); f.write(bytes);
    CHECK_FALSE(SettingsRepository(f.path).load().usedDefaults);
    bytes += ' '; f.write(bytes); CHECK(SettingsRepository(f.path).load().usedDefaults);
}
TEST_CASE("Deep unknown JSON and duplicate root fields rejected", "[settings]") {
    Fixture f; auto bytes = validJson().dump(); bytes.pop_back();
    f.write(bytes + ",\"future\":" + std::string(40, '[') + "0" + std::string(40, ']') + "}");
    CHECK(SettingsRepository(f.path).load().usedDefaults);
    f.write(bytes + ",\"music_volume\":0}"); CHECK(SettingsRepository(f.path).load().usedDefaults);
}
TEST_CASE("Rejected save retains previous file and reports error", "[settings]") {
    Fixture f; std::string error; SettingsRepository repository(f.path);
    REQUIRE(repository.save(Settings{}, error)); const auto before = f.read();
    auto invalid = Settings{}; invalid.musicVolume = -1;
    CHECK_FALSE(repository.save(invalid, error)); CHECK_FALSE(error.empty()); CHECK(f.read() == before);
    invalid = Settings{}; invalid.keys[0] = invalid.keys[1];
    CHECK_FALSE(repository.save(invalid, error)); CHECK(f.read() == before);
}
TEST_CASE("Failed replacement cleans temporary file and retains target", "[settings]") {
    Fixture f; std::filesystem::create_directory(f.path); std::string error;
    CHECK_FALSE(SettingsRepository(f.path).save(Settings{}, error)); CHECK_FALSE(error.empty());
    CHECK(std::filesystem::is_directory(f.path));
    CHECK(std::distance(std::filesystem::directory_iterator(f.dir), std::filesystem::directory_iterator{}) == 1);
    CHECK(SettingsRepository(f.path).load().usedDefaults);
    CHECK_FALSE(SettingsRepository(f.dir / "missing" / "config.json").save(Settings{}, error));
}
TEST_CASE("Corrupt config can be regenerated using fallback defaults", "[settings][TC-REL-01]") {
    Fixture f; f.write("broken"); const auto result = SettingsRepository(f.path).load();
    REQUIRE(result.usedDefaults); std::string error;
    REQUIRE(SettingsRepository(f.path).save(result.settings, error));
    CHECK_FALSE(SettingsRepository(f.path).load().usedDefaults);
    CHECK(SettingsRepository(f.path).load().settings == Settings{});
}
TEST_CASE("Symbolic-link config is rejected without reading it", "[settings]") {
    Fixture f; const auto source = f.dir / "source.json"; std::ofstream(source) << validJson().dump();
    std::error_code ec; std::filesystem::create_symlink(source, f.path, ec);
    if (ec) { SUCCEED("Symlink privileges unavailable; platform check deferred"); return; }
    CHECK(SettingsRepository(f.path).load().usedDefaults);
}

TEST_CASE("Shared storage keeps Settings depth eight and stable fallback codes", "[settings][integration]") {
    Fixture f;
    auto document = validJson();
    nlohmann::json value = 0;
    for (int i = 0; i < 7; ++i) value = nlohmann::json::array({value});
    document["future"] = value;
    f.write(document.dump());
    CHECK_FALSE(SettingsRepository(f.path).load().usedDefaults);
    document["future"] = nlohmann::json::array({value});
    f.write(document.dump());
    CHECK(SettingsRepository(f.path).load().warning == "E_PARSE: defaults loaded");
    auto bytes = validJson().dump(); bytes.pop_back();
    f.write(bytes + ",\"future\":{\"x\":1,\"x\":2}}");
    CHECK(SettingsRepository(f.path).load().warning == "E_PARSE: defaults loaded");
    f.write(std::string(65537, ' '));
    CHECK(SettingsRepository(f.path).load().warning == "E_FILE_TOO_LARGE: defaults loaded");
}

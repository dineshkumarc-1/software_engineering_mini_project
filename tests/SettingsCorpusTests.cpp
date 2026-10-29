// Issue #18: the config.json half of the JSON corpus (TC-SEC-03, SEC-007)
// against the C12 SettingsRepository. CMake builds this suite only when the
// snake_settings module exists (issue #20).
//
//  - 25 valid fixtures load exactly as written.
//  - 25 malformed fixtures fall back to the defaults with an E_ code.
//  - Exactly 64 KiB loads; 64 KiB + 1 byte and a huge file are refused.
// Each load must finish within 1 s.

#include "snakextreme/Settings.hpp"
#include "snakextreme/SettingsRepository.hpp"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

namespace {

const fs::path CorpusDir{SNAKE_JSON_CORPUS_DIR};

class TempDir {
public:
    TempDir() : path_(uniquePath()) {
        fs::create_directories(path_);
    }
    ~TempDir() {
        std::error_code ignored;
        fs::remove_all(path_, ignored);
    }
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    fs::path config() const {
        return path_ / "config.json";
    }

private:
    static fs::path uniquePath() {
        std::random_device random;
        return fs::temp_directory_path() /
               ("snakextreme-config-corpus-" + std::to_string(random()) + "-" + std::to_string(random()));
    }

    fs::path path_;
};

std::vector<fs::path> fixtures(const std::string& set) {
    std::vector<fs::path> files;
    for (const auto& item : fs::directory_iterator(CorpusDir / "config" / set)) {
        if (item.path().extension() == ".json") {
            files.push_back(item.path());
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

std::string readBytes(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

// True when the file still holds exactly `expected`.
bool holds(const fs::path& path, const std::string& expected) {
    return readBytes(path) == expected;
}

void writeBytes(const fs::path& path, const std::string& bytes) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << bytes;
}

struct Timed {
    snake::SettingsLoadResult result;
    std::chrono::milliseconds took{0};
};

Timed timedLoad(const fs::path& config) {
    const auto start = std::chrono::steady_clock::now();
    Timed loaded{snake::SettingsRepository(config).load(), {}};
    loaded.took = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    return loaded;
}

snake::Theme themeFromText(const std::string& text) {
    if (text == "Neon") {
        return snake::Theme::Neon;
    }
    if (text == "Retro") {
        return snake::Theme::Retro;
    }
    return snake::Theme::Classic;
}

} // namespace

TEST_CASE("Corpus: all 25 valid config files load exactly as written",
          "[corpus][TC-SEC-03][settings]") {
    const auto files = fixtures("valid");
    REQUIRE(files.size() == 25);

    for (const fs::path& fixture : files) {
        CAPTURE(fixture.filename().string());
        TempDir dir;
        fs::copy_file(fixture, dir.config());

        const Timed loaded = timedLoad(dir.config());
        REQUIRE(loaded.took < 1s);
        CAPTURE(loaded.result.warning);
        REQUIRE_FALSE(loaded.result.usedDefaults);

        // The expected settings, read independently from the fixture.
        const auto expected = nlohmann::json::parse(readBytes(fixture));
        const snake::Settings& settings = loaded.result.settings;
        REQUIRE(settings.keys[0] == expected.at("key_up").get<int>());
        REQUIRE(settings.keys[1] == expected.at("key_down").get<int>());
        REQUIRE(settings.keys[2] == expected.at("key_left").get<int>());
        REQUIRE(settings.keys[3] == expected.at("key_right").get<int>());
        REQUIRE(settings.musicVolume == expected.at("music_volume").get<int>());
        REQUIRE(settings.sfxVolume == expected.at("sfx_volume").get<int>());
        REQUIRE(settings.theme == themeFromText(expected.at("theme").get<std::string>()));
    }
}

TEST_CASE("Corpus: the 25 malformed config files fall back to the defaults",
          "[corpus][TC-SEC-03][settings]") {
    const auto files = fixtures("malformed");
    REQUIRE(files.size() == 25);

    for (const fs::path& fixture : files) {
        CAPTURE(fixture.filename().string());
        TempDir dir;
        fs::copy_file(fixture, dir.config());
        const std::string before = readBytes(dir.config());

        const Timed loaded = timedLoad(dir.config());
        REQUIRE(loaded.took < 1s);
        CAPTURE(loaded.result.warning);
        REQUIRE(loaded.result.usedDefaults);
        REQUIRE(loaded.result.settings == snake::Settings{});
        REQUIRE(loaded.result.warning.rfind("E_", 0) == 0);
        // Loading never changes the file.
        REQUIRE(holds(dir.config(), before));
    }
}

TEST_CASE("Boundary: a 64 KiB config loads, 64 KiB + 1 byte and huge files are refused",
          "[corpus][boundary][SEC-007][settings]") {
    const std::string valid = readBytes(CorpusDir / "config" / "valid" / "08-theme-neon.json");
    TempDir dir;

    writeBytes(dir.config(), valid + std::string(65536 - valid.size(), ' '));
    REQUIRE(fs::file_size(dir.config()) == 65536);
    const Timed exact = timedLoad(dir.config());
    REQUIRE_FALSE(exact.result.usedDefaults);
    REQUIRE(exact.result.settings.theme == snake::Theme::Neon);

    writeBytes(dir.config(), valid + std::string(65537 - valid.size(), ' '));
    REQUIRE(fs::file_size(dir.config()) == 65537);
    const Timed over = timedLoad(dir.config());
    REQUIRE(over.result.usedDefaults);
    REQUIRE(over.result.warning.rfind("E_FILE_TOO_LARGE", 0) == 0);
    REQUIRE(over.took < 1s);

    // 256 MiB: refused without reading it all.
    fs::resize_file(dir.config(), std::uintmax_t{256} * 1024 * 1024);
    const Timed huge = timedLoad(dir.config());
    REQUIRE(huge.result.usedDefaults);
    REQUIRE(huge.result.warning.rfind("E_FILE_TOO_LARGE", 0) == 0);
    REQUIRE(huge.took < 1s);
}

#include "snakextreme/AudioModel.hpp"
#include "snakextreme/Settings.hpp"
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <array>
#include <cstdlib>
using namespace snake;
TEST_CASE("Music and SFX gains clamp independently", "[audio][TC-SET-02]") {
    AudioLevels levels; CHECK(levels.music() == 70); CHECK(levels.sfx() == 70);
    for (int music : {-20, 0, 1, 70, 99, 100, 120})
        for (int sfx : {-20, 0, 1, 70, 99, 100, 120}) {
            levels.set(music, sfx);
            CHECK(levels.music() == std::clamp(music, 0, 100));
            CHECK(levels.sfx() == std::clamp(sfx, 0, 100));
        }
}
TEST_CASE("Preview levels are transient and cancellation restores saved levels", "[audio][TC-SET-02]") {
    Settings saved; saved.musicVolume = 25; saved.sfxVolume = 80;
    SettingsEditor editor; editor.open(saved); AudioLevels levels;
    editor.selectRow(4); editor.setVolume(0);
    levels.set(editor.value().musicVolume, editor.value().sfxVolume);
    CHECK(levels.music() == 0); CHECK(levels.sfx() == 80);
    editor.selectRow(5); editor.setVolume(100);
    levels.set(editor.value().musicVolume, editor.value().sfxVolume);
    CHECK(levels.music() == 0); CHECK(levels.sfx() == 100);
    levels.set(saved.musicVolume, saved.sfxVolume);
    CHECK(levels.music() == 25); CHECK(levels.sfx() == 80);
}
TEST_CASE("Music PCM is deterministic bounded and click-free at loop endpoints", "[audio]") {
    const auto samples = makeMusicSamples(); REQUIRE(samples.size() == AudioSampleRate * 4);
    CHECK(samples == makeMusicSamples()); CHECK(samples.front() == 0); CHECK(samples.back() == 0);
    CHECK(std::any_of(samples.begin(), samples.end(), [](auto s) { return s != 0; }));
    CHECK(std::all_of(samples.begin(), samples.end(), [](auto s) { return std::abs(static_cast<int>(s)) <= 3500; }));
    for (std::size_t i = 0; i < 8; ++i) {
        CHECK(samples[i * AudioSampleRate / 2] == 0);
        CHECK(samples[(i + 1) * AudioSampleRate / 2 - 1] == 0);
    }
}
TEST_CASE("SFX are bounded distinct mono PCM with silent endpoints", "[audio]") {
    const std::array<SoundCue, 3> cues{SoundCue::Preview, SoundCue::Food, SoundCue::GameOver};
    const std::array<std::size_t, 3> milliseconds{100, 140, 350};
    for (std::size_t i = 0; i < cues.size(); ++i) {
        const auto samples = makeSoundSamples(cues[i]);
        REQUIRE(samples.size() == AudioSampleRate * milliseconds[i] / 1000);
        CHECK(samples == makeSoundSamples(cues[i])); CHECK(samples.front() == 0); CHECK(samples.back() == 0);
        CHECK(std::any_of(samples.begin(), samples.end(), [](auto s) { return s != 0; }));
        CHECK(std::all_of(samples.begin(), samples.end(), [](auto s) { return std::abs(static_cast<int>(s)) <= 6000; }));
    }
    CHECK(makeSoundSamples(SoundCue::Food) != makeSoundSamples(SoundCue::Preview));
}

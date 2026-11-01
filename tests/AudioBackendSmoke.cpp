#include "snakextreme/AudioService.hpp"
#include <cstdlib>
#include <iostream>
#include <cmath>

int main() {
    int checks = 0;
    const auto check = [&checks](bool pass, const char* what) {
        ++checks;
        if (!pass) { std::cerr << "FAIL: " << what << '\n'; std::exit(EXIT_FAILURE); }
    };
    snake::AudioService audio;
    // #22: constructing the service must not open the device, and every call
    // before start() must be a safe no-op (the app starts audio after its
    // first frame, while key handling may already call these).
    check(!audio.started() && !audio.available() && !audio.musicPlaying(), "nothing opened before start()");
    audio.setVolumes(40, 60);
    audio.playSfx(snake::SoundCue::Food);
    audio.setPaused(true);
    audio.setPaused(false);
    check(!audio.started() && !audio.available(), "calls before start() stay no-ops");
    if (!audio.start(70, 70)) { std::cerr << "Audio backend not available\n"; return EXIT_FAILURE; }
    check(audio.started() && audio.available(), "start() opens the device");
    check(audio.musicPlaying(), "loop starts");
    audio.setVolumes(0, 100);
    check(audio.musicVolume() == 0.f && audio.sfxVolume() == 100.f, "mute music independently");
    audio.playSfx(snake::SoundCue::Preview);
    audio.setVolumes(100, 0);
    check(audio.musicVolume() == 100.f && audio.sfxVolume() == 0.f, "mute SFX independently");
    audio.playSfx(snake::SoundCue::Food);
    audio.setVolumes(-10, 120);
    check(audio.musicVolume() == 0.f && audio.sfxVolume() == 100.f, "backend boundary clamp");
    audio.setPaused(true); check(!audio.musicPlaying(), "pause stops music progression");
    audio.setPaused(false); check(audio.musicPlaying(), "resume restarts music");
    audio.playSfx(snake::SoundCue::GameOver);
    audio.setVolumes(25, 80);
    std::cout << "Restored backend gains: music=" << audio.musicVolume()
              << " sfx=" << audio.sfxVolume() << '\n';
    check(std::abs(audio.musicVolume() - 25.f) < 0.01f && std::abs(audio.sfxVolume() - 80.f) < 0.01f,
          "cancel/restored gain routing");
    std::cout << checks << " real SFML/OpenAL audio backend checks passed\n";
}

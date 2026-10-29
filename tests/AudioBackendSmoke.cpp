#include "snakextreme/AudioService.hpp"
#include <cstdlib>
#include <iostream>
#include <cmath>

int main() {
    snake::AudioService audio;
    if (!audio.start(70, 70)) { std::cerr << "Audio backend not available\n"; return EXIT_FAILURE; }
    int checks = 0;
    const auto check = [&checks](bool pass, const char* what) {
        ++checks;
        if (!pass) { std::cerr << "FAIL: " << what << '\n'; std::exit(EXIT_FAILURE); }
    };
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

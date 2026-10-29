#include "snakextreme/AudioModel.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace snake {
namespace {
constexpr double Pi = 3.14159265358979323846;
std::int16_t tone(std::size_t index, std::size_t count, double frequency, double amplitude) {
    const double time = static_cast<double>(index) / AudioSampleRate;
    const double remaining = static_cast<double>(count - index - 1) / AudioSampleRate;
    const double envelope = std::max(0.0, std::min({1.0, time / 0.01, remaining / 0.025}));
    return static_cast<std::int16_t>(amplitude * envelope * std::sin(2.0 * Pi * frequency * time));
}
}
void AudioLevels::set(int music, int sfx) noexcept {
    music_ = std::clamp(music, 0, 100);
    sfx_ = std::clamp(sfx, 0, 100);
}
std::vector<std::int16_t> makeMusicSamples() {
    constexpr std::array<double, 8> notes{261.63, 329.63, 392.0, 329.63, 293.66, 349.23, 392.0, 523.25};
    constexpr std::size_t noteSamples = AudioSampleRate / 2;
    std::vector<std::int16_t> samples(noteSamples * notes.size());
    for (std::size_t note = 0; note < notes.size(); ++note)
        for (std::size_t i = 0; i < noteSamples; ++i)
            samples[note * noteSamples + i] = tone(i, noteSamples, notes[note], 3500.0);
    return samples;
}
std::vector<std::int16_t> makeSoundSamples(SoundCue cue) {
    const unsigned durationMs = cue == SoundCue::GameOver ? 350 : cue == SoundCue::Food ? 140 : 100;
    const double frequency = cue == SoundCue::GameOver ? 164.81 : cue == SoundCue::Food ? 783.99 : 523.25;
    const std::size_t count = static_cast<std::size_t>(AudioSampleRate) * durationMs / 1000;
    std::vector<std::int16_t> samples(count);
    for (std::size_t i = 0; i < count; ++i) samples[i] = tone(i, count, frequency, 6000.0);
    return samples;
}
} // namespace snake

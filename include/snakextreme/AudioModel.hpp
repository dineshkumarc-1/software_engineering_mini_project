#pragma once
#include <cstdint>
#include <vector>

namespace snake {
enum class SoundCue { Preview, Food, GameOver };
inline constexpr unsigned AudioSampleRate = 22050;
// Bounded, deterministic original tones: no external downloads or assets.
std::vector<std::int16_t> makeMusicSamples();
std::vector<std::int16_t> makeSoundSamples(SoundCue cue);
class AudioLevels {
public:
    void set(int music, int sfx) noexcept;
    int music() const noexcept { return music_; }
    int sfx() const noexcept { return sfx_; }
private:
    int music_{70};
    int sfx_{70};
};
} // namespace snake

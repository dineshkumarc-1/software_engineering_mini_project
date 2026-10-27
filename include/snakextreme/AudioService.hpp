#pragma once
#include "snakextreme/AudioModel.hpp"
#include <SFML/Audio.hpp>

namespace snake {
// C5 presentation adapter. Buffers outlive sounds; SFML handles device resources.
// At most one SFX voice plus one looping music voice, with independent gains.
class AudioService {
public:
    bool start(int musicVolume, int sfxVolume);
    ~AudioService();
    void setVolumes(int musicVolume, int sfxVolume);
    void setPaused(bool paused);
    void playSfx(SoundCue cue);
    bool available() const noexcept { return ready_; }
    bool musicPlaying() const { return music_.getStatus() == sf::Sound::Playing; }
    float musicVolume() const { return music_.getVolume(); }
    float sfxVolume() const { return sfx_.getVolume(); }
private:
    sf::SoundBuffer musicBuffer_, previewBuffer_, foodBuffer_, gameOverBuffer_;
    sf::Sound music_, sfx_;
    AudioLevels levels_;
    bool ready_{false};
    bool paused_{false};
};
} // namespace snake

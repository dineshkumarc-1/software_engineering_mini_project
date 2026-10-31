#pragma once
#include "snakextreme/AudioModel.hpp"
#include <memory>

namespace snake {
// C5 presentation adapter. Buffers outlive sounds; SFML handles device resources.
// At most one SFX voice plus one looping music voice, with independent gains.
//
// Constructing an AudioService does not touch the audio device. The OpenAL
// device is opened by the first start(), so the application can show its first
// frame before paying for device start-up (#22). Until start() succeeds every
// call is a safe no-op apart from remembering the requested gains.
class AudioService {
public:
    AudioService();
    ~AudioService();
    AudioService(const AudioService&) = delete;
    AudioService& operator=(const AudioService&) = delete;
    bool start(int musicVolume, int sfxVolume);
    void setVolumes(int musicVolume, int sfxVolume);
    void setPaused(bool paused);
    void playSfx(SoundCue cue);
    bool available() const noexcept { return ready_; }
    bool started() const noexcept { return backend_ != nullptr; }
    bool musicPlaying() const;
    float musicVolume() const;
    float sfxVolume() const;
private:
    struct Backend;
    std::unique_ptr<Backend> backend_;
    AudioLevels levels_;
    bool ready_{false};
    bool paused_{false};
};
} // namespace snake

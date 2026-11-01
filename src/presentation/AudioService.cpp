#include "snakextreme/AudioService.hpp"
#include <SFML/Audio.hpp>
#include <exception>
#include <new>

namespace snake {
// Declared buffers first so the sounds that use them are destroyed first.
// Creating any of these SFML objects opens the shared OpenAL device.
struct AudioService::Backend {
    sf::SoundBuffer musicBuffer, previewBuffer, foodBuffer, gameOverBuffer;
    sf::Sound music, sfx;
};
namespace {
bool load(sf::SoundBuffer& buffer, const std::vector<std::int16_t>& samples) {
    static_assert(sizeof(sf::Int16) == sizeof(std::int16_t), "PCM format mismatch");
    return buffer.loadFromSamples(samples.data(), static_cast<sf::Uint64>(samples.size()), 1, AudioSampleRate);
}
}
AudioService::AudioService() = default;
bool AudioService::start(int musicVolume, int sfxVolume) {
    ready_ = false; paused_ = false;
    levels_.set(musicVolume, sfxVolume);
    try {
        if (backend_) { backend_->music.stop(); backend_->sfx.stop(); }
        else backend_ = std::make_unique<Backend>();
        Backend& audio = *backend_;
        if (!load(audio.musicBuffer, makeMusicSamples()) ||
            !load(audio.previewBuffer, makeSoundSamples(SoundCue::Preview)) ||
            !load(audio.foodBuffer, makeSoundSamples(SoundCue::Food)) ||
            !load(audio.gameOverBuffer, makeSoundSamples(SoundCue::GameOver))) {
            audio.music.stop(); audio.sfx.stop();
            return false;
        }
        audio.music.setBuffer(audio.musicBuffer); audio.sfx.setBuffer(audio.previewBuffer);
        audio.music.setLoop(true); audio.sfx.setLoop(false);
        audio.music.setVolume(static_cast<float>(levels_.music()));
        audio.sfx.setVolume(static_cast<float>(levels_.sfx()));
        audio.music.play();
        ready_ = audio.music.getStatus() == sf::Sound::Playing;
    } catch (const std::exception&) { ready_ = false; }
    if (!ready_ && backend_) { backend_->music.stop(); backend_->sfx.stop(); }
    return ready_;
}
AudioService::~AudioService() { if (backend_) { backend_->music.stop(); backend_->sfx.stop(); } }
void AudioService::setVolumes(int musicVolume, int sfxVolume) {
    levels_.set(musicVolume, sfxVolume);
    if (!ready_) return;
    backend_->music.setVolume(static_cast<float>(levels_.music()));
    backend_->sfx.setVolume(static_cast<float>(levels_.sfx()));
}
void AudioService::setPaused(bool paused) {
    if (!ready_ || paused == paused_) return;
    paused_ = paused;
    if (paused) { backend_->music.pause(); backend_->sfx.stop(); }
    else backend_->music.play();
}
void AudioService::playSfx(SoundCue cue) {
    if (!ready_ || paused_) return;
    sf::Sound& sfx = backend_->sfx;
    sfx.stop();
    switch (cue) {
    case SoundCue::Food: sfx.setBuffer(backend_->foodBuffer); break;
    case SoundCue::GameOver: sfx.setBuffer(backend_->gameOverBuffer); break;
    case SoundCue::Preview: sfx.setBuffer(backend_->previewBuffer); break;
    }
    sfx.play();
}
bool AudioService::musicPlaying() const {
    return backend_ && backend_->music.getStatus() == sf::Sound::Playing;
}
float AudioService::musicVolume() const { return backend_ ? backend_->music.getVolume() : 0.f; }
float AudioService::sfxVolume() const { return backend_ ? backend_->sfx.getVolume() : 0.f; }
} // namespace snake

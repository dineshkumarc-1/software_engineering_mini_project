#include "snakextreme/AudioService.hpp"
#include <exception>

namespace snake {
namespace {
bool load(sf::SoundBuffer& buffer, const std::vector<std::int16_t>& samples) {
    static_assert(sizeof(sf::Int16) == sizeof(std::int16_t), "PCM format mismatch");
    return buffer.loadFromSamples(samples.data(), static_cast<sf::Uint64>(samples.size()), 1, AudioSampleRate);
}
}
bool AudioService::start(int musicVolume, int sfxVolume) {
    music_.stop(); sfx_.stop(); ready_ = false; paused_ = false;
    try {
        if (!load(musicBuffer_, makeMusicSamples()) ||
            !load(previewBuffer_, makeSoundSamples(SoundCue::Preview)) ||
            !load(foodBuffer_, makeSoundSamples(SoundCue::Food)) ||
            !load(gameOverBuffer_, makeSoundSamples(SoundCue::GameOver))) return false;
        music_.setBuffer(musicBuffer_); sfx_.setBuffer(previewBuffer_);
        music_.setLoop(true); sfx_.setLoop(false);
        levels_.set(musicVolume, sfxVolume);
        music_.setVolume(static_cast<float>(levels_.music()));
        sfx_.setVolume(static_cast<float>(levels_.sfx()));
        music_.play();
        ready_ = music_.getStatus() == sf::Sound::Playing;
    } catch (const std::exception&) { ready_ = false; }
    if (!ready_) { music_.stop(); sfx_.stop(); }
    return ready_;
}
AudioService::~AudioService() { if (ready_) { music_.stop(); sfx_.stop(); } }
void AudioService::setVolumes(int musicVolume, int sfxVolume) {
    levels_.set(musicVolume, sfxVolume);
    if (!ready_) return;
    music_.setVolume(static_cast<float>(levels_.music()));
    sfx_.setVolume(static_cast<float>(levels_.sfx()));
}
void AudioService::setPaused(bool paused) {
    if (!ready_ || paused == paused_) return;
    paused_ = paused;
    if (paused) { music_.pause(); sfx_.stop(); }
    else music_.play();
}
void AudioService::playSfx(SoundCue cue) {
    if (!ready_ || paused_) return;
    sfx_.stop();
    switch (cue) {
    case SoundCue::Food: sfx_.setBuffer(foodBuffer_); break;
    case SoundCue::GameOver: sfx_.setBuffer(gameOverBuffer_); break;
    case SoundCue::Preview: sfx_.setBuffer(previewBuffer_); break;
    }
    sfx_.play();
}
} // namespace snake

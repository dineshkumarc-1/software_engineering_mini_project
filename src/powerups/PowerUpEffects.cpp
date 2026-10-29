#include "snakextreme/PowerUp.hpp"

#include "snakextreme/GameConfig.hpp"

namespace snake {

// ---- Speed (FR-013) ---------------------------------------------------------

PowerUpType SpeedEffect::type() const noexcept {
    return PowerUpType::Speed;
}

std::chrono::milliseconds SpeedEffect::duration() const noexcept {
    return std::chrono::milliseconds{config::SpeedBoostDurationMs};
}

void SpeedEffect::apply(IPowerUpTarget& target) const {
    target.setTickScale({config::SpeedBoostNumerator, config::SpeedBoostDenominator});
}

void SpeedEffect::revert(IPowerUpTarget& target) const {
    target.setTickScale(TickScale{});
}

// ---- Multiplier (FR-014) ----------------------------------------------------

PowerUpType MultiplierEffect::type() const noexcept {
    return PowerUpType::Multiplier;
}

std::chrono::milliseconds MultiplierEffect::duration() const noexcept {
    return std::chrono::milliseconds{config::MultiplierDurationMs};
}

void MultiplierEffect::apply(IPowerUpTarget& target) const {
    target.setScoreMultiplier(config::ScoreMultiplier);
}

void MultiplierEffect::revert(IPowerUpTarget& target) const {
    target.setScoreMultiplier(1);
}

// ---- Shrink (FR-015) --------------------------------------------------------

PowerUpType ShrinkEffect::type() const noexcept {
    return PowerUpType::Shrink;
}

std::chrono::milliseconds ShrinkEffect::duration() const noexcept {
    return std::chrono::milliseconds{0};
}

void ShrinkEffect::apply(IPowerUpTarget& target) const {
    target.shrink(config::ShrinkSegments, config::MinimumSnakeLength);
}

void ShrinkEffect::revert(IPowerUpTarget& /*target*/) const {
    // Instant effect: the removed segment is not given back.
}

// ---- Lookup and HUD text ----------------------------------------------------

const IPowerUpEffect& effectFor(PowerUpType type) noexcept {
    static const SpeedEffect speed;
    static const MultiplierEffect multiplier;
    static const ShrinkEffect shrink;

    switch (type) {
    case PowerUpType::Speed:
        return speed;
    case PowerUpType::Multiplier:
        return multiplier;
    case PowerUpType::Shrink:
        return shrink;
    }
    return speed;
}

std::string formatEffectTime(std::chrono::microseconds remaining) {
    constexpr long long MicrosPerTenth = 100000;
    const long long micros = remaining.count() > 0 ? remaining.count() : 0;
    const long long tenths = (micros + MicrosPerTenth - 1) / MicrosPerTenth;
    return std::to_string(tenths / 10) + "." + std::to_string(tenths % 10) + "s";
}

std::string describeEffect(const ActiveEffect& effect) {
    std::string name;
    switch (effect.type) {
    case PowerUpType::Speed:
        name = "Speed boost";
        break;
    case PowerUpType::Multiplier:
        name = "Score x" + std::to_string(config::ScoreMultiplier);
        break;
    case PowerUpType::Shrink:
        name = "Shrink";
        break;
    }
    return name + " " + formatEffectTime(effect.remaining);
}

} // namespace snake

#pragma once

// C8 power-ups: types, timed effects and spawn timing (SAD 3.4 and 4.1,
// ADR-003; FR-012..015; issue #14). SFML-free.
//
// Effects are interchangeable strategies that act on an IPowerUpTarget, so
// neither the engine nor the score service needs to know which power-ups
// exist. GameplayController provides the target for the running game.

#include "snakextreme/GameConfig.hpp"
#include "snakextreme/GameEngine.hpp" // GridPosition

#include <array>
#include <chrono>
#include <cstddef>
#include <string>
#include <string_view>

namespace snake {

enum class PowerUpType {
    Speed,      // FR-013: faster movement for 5 s
    Multiplier, // FR-014: food points x2 for 10 s
    Shrink      // FR-015: one tail segment removed, never below 3
};

inline constexpr std::array<PowerUpType, 3> AllPowerUpTypes{
    PowerUpType::Speed, PowerUpType::Multiplier, PowerUpType::Shrink};

constexpr std::string_view toString(PowerUpType type) noexcept {
    switch (type) {
    case PowerUpType::Speed:
        return "Speed";
    case PowerUpType::Multiplier:
        return "Multiplier";
    case PowerUpType::Shrink:
        return "Shrink";
    }
    return "Speed";
}

// Scale of the movement interval as an exact fraction: 1/1 normally, 3/5
// during a Speed boost (ADR-002: tick interval = difficulty interval x boost
// factor), so 200 / 120 / 70 ms become exactly 120 / 72 / 42 ms.
struct TickScale {
    int numerator{1};
    int denominator{1};
};

constexpr bool operator==(TickScale a, TickScale b) noexcept {
    return a.numerator * b.denominator == b.numerator * a.denominator;
}

constexpr bool operator!=(TickScale a, TickScale b) noexcept {
    return !(a == b);
}

constexpr std::chrono::microseconds scaled(std::chrono::microseconds interval,
                                           TickScale scale) noexcept {
    return interval * scale.numerator / scale.denominator;
}

// A power-up on the grid, waiting to be collected.
struct PowerUp {
    PowerUpType type{PowerUpType::Speed};
    GridPosition cell{};
    // Time left before it disappears uncollected.
    std::chrono::microseconds remaining{0};
};

// The timed effect currently on the snake.
struct ActiveEffect {
    PowerUpType type{PowerUpType::Speed};
    std::chrono::microseconds remaining{0};
};

// When power-ups appear and how long an uncollected one stays (FR-012, SRS
// Addendum v1.1 section 4): the next one appears a random 8-15 s after the
// previous one appeared, and one that is not collected disappears after 5 s.
// The defaults are the SRS values.
struct PowerUpSchedule {
    std::chrono::milliseconds minGap{config::PowerUpSpawnGapMinMs};
    std::chrono::milliseconds maxGap{config::PowerUpSpawnGapMaxMs};
    std::chrono::milliseconds lifetime{config::PowerUpLifetimeMs};
    // False: no power-ups appear on their own (tests that place them).
    bool randomSpawns{true};
};

// What a power-up effect may change in the running game.
class IPowerUpTarget {
public:
    virtual ~IPowerUpTarget() = default;

    // FR-013: scale of the movement interval.
    virtual void setTickScale(TickScale scale) = 0;
    // FR-014: multiplier on the points for each food.
    virtual void setScoreMultiplier(int multiplier) = 0;
    // FR-015: removes up to `segments` tail segments without going below
    // `minimumLength`.
    virtual void shrink(std::size_t segments, std::size_t minimumLength) = 0;
};

// One power-up effect (Strategy, ADR-003). Effects are stateless.
class IPowerUpEffect {
public:
    virtual ~IPowerUpEffect() = default;

    virtual PowerUpType type() const noexcept = 0;
    // How long the effect lasts; zero for an instant effect (Shrink).
    virtual std::chrono::milliseconds duration() const noexcept = 0;
    virtual void apply(IPowerUpTarget& target) const = 0;
    virtual void revert(IPowerUpTarget& target) const = 0;
};

// The snake moves every 0.6 x interval for 5 s.
class SpeedEffect final : public IPowerUpEffect {
public:
    PowerUpType type() const noexcept override;
    std::chrono::milliseconds duration() const noexcept override;
    void apply(IPowerUpTarget& target) const override;
    void revert(IPowerUpTarget& target) const override;
};

// Food is worth twice its points for 10 s.
class MultiplierEffect final : public IPowerUpEffect {
public:
    PowerUpType type() const noexcept override;
    std::chrono::milliseconds duration() const noexcept override;
    void apply(IPowerUpTarget& target) const override;
    void revert(IPowerUpTarget& target) const override;
};

// One tail segment is removed at once (minimum length 3); nothing to revert.
class ShrinkEffect final : public IPowerUpEffect {
public:
    PowerUpType type() const noexcept override;
    std::chrono::milliseconds duration() const noexcept override;
    void apply(IPowerUpTarget& target) const override;
    void revert(IPowerUpTarget& target) const override;
};

// The (shared, stateless) effect of a power-up type.
const IPowerUpEffect& effectFor(PowerUpType type) noexcept;

// Seconds left as shown on the HUD, rounded up to tenths so it reads 5.0s
// right after pick-up and never 0.0s while the effect still runs: "4.9s".
std::string formatEffectTime(std::chrono::microseconds remaining);

// HUD and window-title text for an active effect, e.g. "Speed boost 4.9s"
// or "Score x2 9.9s".
std::string describeEffect(const ActiveEffect& effect);

} // namespace snake

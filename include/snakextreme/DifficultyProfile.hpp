#pragma once

#include "snakextreme/GameConfig.hpp"
#include "snakextreme/GameMode.hpp"

#include <chrono>

namespace snake {

// FR-009 / FR-002: a difficulty level fixes the snake's base movement speed.
// Power-up speed changes (FR-013) scale this interval later; they never
// change the profile itself.
struct DifficultyProfile {
    Difficulty difficulty{DefaultDifficulty};
    // Time between two one-cell moves.
    std::chrono::milliseconds tickInterval{config::MediumMoveIntervalMs};
};

constexpr DifficultyProfile difficultyProfile(Difficulty difficulty) noexcept {
    switch (difficulty) {
    case Difficulty::Easy:
        return {Difficulty::Easy,
                std::chrono::milliseconds{config::EasyMoveIntervalMs}};
    case Difficulty::Medium:
        return {Difficulty::Medium,
                std::chrono::milliseconds{config::MediumMoveIntervalMs}};
    case Difficulty::Hard:
        return {Difficulty::Hard,
                std::chrono::milliseconds{config::HardMoveIntervalMs}};
    }
    return {};
}

constexpr std::chrono::milliseconds tickInterval(Difficulty difficulty) noexcept {
    return difficultyProfile(difficulty).tickInterval;
}

// Compile-time guard on the SRS values (Easy 200 / Medium 120 / Hard 70 ms).
static_assert(tickInterval(Difficulty::Easy) == std::chrono::milliseconds{200});
static_assert(tickInterval(Difficulty::Medium) == std::chrono::milliseconds{120});
static_assert(tickInterval(Difficulty::Hard) == std::chrono::milliseconds{70});
static_assert(tickInterval(Difficulty::Easy) > tickInterval(Difficulty::Medium) &&
                  tickInterval(Difficulty::Medium) > tickInterval(Difficulty::Hard),
              "harder difficulties must move faster");

} // namespace snake

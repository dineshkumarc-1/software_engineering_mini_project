#pragma once

// C7 ModeManager - per-session mode and difficulty state (SAD section 3.4).
// Owns the selected strategy, the difficulty speed, the Timed countdown and
// the Obstacle-mode cells. It does not move the snake: the engine does that,
// and the caller passes obstacles / time-up into the engine (see
// docs/modes-build-ci.md, "Wiring").

#include "snakextreme/DifficultyProfile.hpp"
#include "snakextreme/GameMode.hpp"
#include "snakextreme/ModeStrategy.hpp"

#include <chrono>
#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <random>

namespace snake {

struct SessionConfig {
    GameMode mode{DefaultGameMode};
    Difficulty difficulty{DefaultDifficulty};
};

class ModeManager {
public:
    // Random seed taken from std::random_device.
    explicit ModeManager(SessionConfig config = {});
    // Fixed seed for deterministic tests (ADR-005).
    ModeManager(SessionConfig config, std::uint32_t seed);

    // Starts or restarts a session: resets the countdown and, in Obstacle
    // mode, places obstacles away from the given snake and food. The layout
    // never splits the board.
    //
    // Throws std::runtime_error if the mode's obstacles cannot all be placed
    // safely; the previous session state is then left unchanged. That needs a
    // nearly full board, so it cannot happen when a new game starts. Callers
    // should go back to the menu rather than start the game.
    void startSession(const std::deque<GridPosition>& snake,
                      std::optional<GridPosition> food);

    // Advances the Timed countdown by `elapsed`. Call only while gameplay is
    // running, so pausing freezes the timer (FR-024). Negative values and
    // calls in untimed modes are ignored. Microsecond resolution, so a 60 Hz
    // step (16.667 ms, e.g. sf::Time::asMicroseconds()) does not drift.
    void update(std::chrono::microseconds elapsed) noexcept;

    const SessionConfig& config() const noexcept;
    GameMode mode() const noexcept;
    Difficulty difficulty() const noexcept;
    const IModeStrategy& strategy() const noexcept;

    // Base movement interval for the selected difficulty (FR-009).
    std::chrono::milliseconds tickInterval() const noexcept;
    int foodPoints() const noexcept;

    bool hasTimeLimit() const noexcept;
    // Time left in Timed mode, rounded up to whole milliseconds, so it reads
    // zero only once the time is really up. Never negative; empty in other
    // modes.
    std::optional<std::chrono::milliseconds> remainingTime() const noexcept;
    // True once the Timed countdown has reached zero (FR-010).
    bool isTimeUp() const noexcept;

    const Obstacles& obstacles() const noexcept;
    bool isObstacle(GridPosition cell) const noexcept;
    // Wall or obstacle at `cell` under the current mode.
    bool isFatal(GridPosition cell) const noexcept;

private:
    SessionConfig config_;
    std::unique_ptr<IModeStrategy> strategy_;
    std::mt19937 random_;
    std::chrono::microseconds elapsed_{0};
    Obstacles obstacles_;
};

} // namespace snake

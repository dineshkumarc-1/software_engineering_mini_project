#pragma once

// C7 ModeManager - mode strategies (SAD section 4.3.4, ADR-003).
// SFML-free: only plain grid types are used, so every rule is unit-testable.

#include "snakextreme/GameEngine.hpp" // GridPosition (header-only use)
#include "snakextreme/GameMode.hpp"

#include <chrono>
#include <cstddef>
#include <deque>
#include <memory>
#include <optional>
#include <random>
#include <vector>

namespace snake {

using Obstacles = std::vector<GridPosition>;

// One interchangeable strategy per game mode, so adding a mode never changes
// the engine (ADR-003).
class IModeStrategy {
public:
    virtual ~IModeStrategy() = default;

    virtual GameMode mode() const noexcept = 0;

    // Countdown length; empty when the mode has no time limit (FR-010).
    virtual std::optional<std::chrono::milliseconds> timeLimit() const noexcept = 0;

    // Base points per food before any multiplier (FR-016).
    virtual int foodPoints() const noexcept = 0;

    // Static obstacle cells for a new session (FR-011). Modes without
    // obstacles return an empty list.
    virtual Obstacles placeObstacles(std::mt19937& random,
                                     const std::deque<GridPosition>& snake,
                                     std::optional<GridPosition> food) const = 0;

    // True when moving the head into `cell` ends the game because of a
    // boundary wall or a hazard of this mode. Self-collision stays in the
    // engine, which knows whether the tail vacates its cell this tick.
    virtual bool isFatal(GridPosition cell, const Obstacles& obstacles) const noexcept = 0;
};

// FR-008 Classic: no time limit and no obstacles.
class ClassicMode final : public IModeStrategy {
public:
    GameMode mode() const noexcept override;
    std::optional<std::chrono::milliseconds> timeLimit() const noexcept override;
    int foodPoints() const noexcept override;
    Obstacles placeObstacles(std::mt19937& random,
                             const std::deque<GridPosition>& snake,
                             std::optional<GridPosition> food) const override;
    bool isFatal(GridPosition cell, const Obstacles& obstacles) const noexcept override;
};

// FR-010 Timed Challenge: 60 s countdown, no obstacles.
class TimedMode final : public IModeStrategy {
public:
    GameMode mode() const noexcept override;
    std::optional<std::chrono::milliseconds> timeLimit() const noexcept override;
    int foodPoints() const noexcept override;
    Obstacles placeObstacles(std::mt19937& random,
                             const std::deque<GridPosition>& snake,
                             std::optional<GridPosition> food) const override;
    bool isFatal(GridPosition cell, const Obstacles& obstacles) const noexcept override;
};

// FR-011 Obstacle: 8 static cells that end the game on contact.
class ObstacleMode final : public IModeStrategy {
public:
    GameMode mode() const noexcept override;
    std::optional<std::chrono::milliseconds> timeLimit() const noexcept override;
    int foodPoints() const noexcept override;
    Obstacles placeObstacles(std::mt19937& random,
                             const std::deque<GridPosition>& snake,
                             std::optional<GridPosition> food) const override;
    bool isFatal(GridPosition cell, const Obstacles& obstacles) const noexcept override;
};

std::unique_ptr<IModeStrategy> makeModeStrategy(GameMode mode);

bool isInsideGrid(GridPosition cell) noexcept;

// max(|dx|, |dy|): the number of king moves between two cells.
int chebyshevDistance(GridPosition a, GridPosition b) noexcept;

// Picks up to `count` distinct cells uniformly from the cells that are not on
// the snake, not on the food and at least `minHeadDistance` (Chebyshev) from
// the snake head. Placements that would cut the board into separate regions
// are re-drawn, so food can always be reached. Returns fewer cells only when
// fewer eligible cells exist.
Obstacles placeObstacleCells(std::size_t count,
                             std::mt19937& random,
                             const std::deque<GridPosition>& snake,
                             std::optional<GridPosition> food,
                             int minHeadDistance);

// True when every non-obstacle cell can reach every other one by orthogonal
// moves. Snake cells count as passable because the body moves on.
bool keepsBoardConnected(const Obstacles& obstacles);

} // namespace snake

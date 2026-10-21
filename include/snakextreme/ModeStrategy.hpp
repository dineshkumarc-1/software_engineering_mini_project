#pragma once

// C7 ModeManager - mode strategies (SAD section 4.3.4, ADR-003).
// SFML-free: only plain grid types are used, so every rule is unit-testable.

#include "snakextreme/GameConfig.hpp"
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

    // Obstacles a session of this mode needs: 8 in Obstacle mode (FR-011),
    // none in the others.
    virtual std::size_t obstacleCount() const noexcept = 0;

    // Static obstacle cells for a new session (FR-011). The result never
    // splits the board. It holds fewer than obstacleCount() cells only when
    // no further cell can be added safely, which needs a nearly full board.
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
    std::size_t obstacleCount() const noexcept override { return 0; }
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
    std::size_t obstacleCount() const noexcept override { return 0; }
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
    std::size_t obstacleCount() const noexcept override { return config::ObstacleCount; }
    Obstacles placeObstacles(std::mt19937& random,
                             const std::deque<GridPosition>& snake,
                             std::optional<GridPosition> food) const override;
    bool isFatal(GridPosition cell, const Obstacles& obstacles) const noexcept override;
};

std::unique_ptr<IModeStrategy> makeModeStrategy(GameMode mode);

bool isInsideGrid(GridPosition cell) noexcept;

// max(|dx|, |dy|): the number of king moves between two cells.
int chebyshevDistance(GridPosition a, GridPosition b) noexcept;

// Picks up to `count` distinct cells at random from the cells that are not on
// the snake, not on the food and at least `minHeadDistance` (Chebyshev) from
// the snake head. Cells are added one at a time, and a cell is skipped if it
// would cut the board into separate regions, so the result never splits the
// board and food can always be reached. Fewer than `count` cells come back
// only when no further eligible cell can be added safely; callers must treat
// that as a failure, not as a smaller layout to play with.
Obstacles placeObstacleCells(std::size_t count,
                             std::mt19937& random,
                             const std::deque<GridPosition>& snake,
                             std::optional<GridPosition> food,
                             int minHeadDistance);

// True when every non-obstacle cell can reach every other one by orthogonal
// moves. Snake cells count as passable because the body moves on.
bool keepsBoardConnected(const Obstacles& obstacles);

} // namespace snake

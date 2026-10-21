#include "snakextreme/ModeStrategy.hpp"

#include "snakextreme/GameConfig.hpp"

#include <algorithm>
#include <cstdlib>
#include <utility>

namespace snake {

namespace {

constexpr std::size_t CellCount =
    static_cast<std::size_t>(config::GridColumns) *
    static_cast<std::size_t>(config::GridRows);

// Random orders tried before giving up on placing every requested cell. One
// pass always succeeds on a normal board; more passes only matter when the
// board is nearly full, and every pass returns a connected layout.
constexpr int MaxPlacementPasses = 16;

std::size_t indexOf(GridPosition cell) noexcept {
    return static_cast<std::size_t>(cell.y) *
               static_cast<std::size_t>(config::GridColumns) +
           static_cast<std::size_t>(cell.x);
}

bool contains(const Obstacles& cells, GridPosition cell) noexcept {
    return std::find(cells.begin(), cells.end(), cell) != cells.end();
}

std::optional<std::chrono::milliseconds> noTimeLimit() noexcept {
    return std::nullopt;
}

} // namespace

bool isInsideGrid(GridPosition cell) noexcept {
    return cell.x >= 0 && cell.x < config::GridColumns &&
           cell.y >= 0 && cell.y < config::GridRows;
}

int chebyshevDistance(GridPosition a, GridPosition b) noexcept {
    return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y));
}

bool keepsBoardConnected(const Obstacles& obstacles) {
    std::vector<char> blocked(CellCount, 0);
    std::size_t blockedCount = 0;

    for (const GridPosition cell : obstacles) {
        if (isInsideGrid(cell) && blocked[indexOf(cell)] == 0) {
            blocked[indexOf(cell)] = 1;
            ++blockedCount;
        }
    }

    const std::size_t openCount = CellCount - blockedCount;
    if (openCount == 0) {
        return true;
    }

    // Breadth-first flood fill from the first open cell.
    std::vector<char> seen(CellCount, 0);
    std::vector<GridPosition> frontier;
    for (int y = 0; y < config::GridRows && frontier.empty(); ++y) {
        for (int x = 0; x < config::GridColumns; ++x) {
            if (blocked[indexOf({x, y})] == 0) {
                frontier.push_back({x, y});
                seen[indexOf({x, y})] = 1;
                break;
            }
        }
    }

    std::size_t reached = 0;
    while (!frontier.empty()) {
        const GridPosition cell = frontier.back();
        frontier.pop_back();
        ++reached;

        const GridPosition neighbours[] = {
            {cell.x + 1, cell.y}, {cell.x - 1, cell.y},
            {cell.x, cell.y + 1}, {cell.x, cell.y - 1}};

        for (const GridPosition next : neighbours) {
            if (!isInsideGrid(next)) {
                continue;
            }
            const std::size_t index = indexOf(next);
            if (blocked[index] == 0 && seen[index] == 0) {
                seen[index] = 1;
                frontier.push_back(next);
            }
        }
    }

    return reached == openCount;
}

Obstacles placeObstacleCells(std::size_t count,
                             std::mt19937& random,
                             const std::deque<GridPosition>& snake,
                             std::optional<GridPosition> food,
                             int minHeadDistance) {
    if (count == 0) {
        return {};
    }

    std::vector<char> onSnake(CellCount, 0);
    for (const GridPosition segment : snake) {
        if (isInsideGrid(segment)) {
            onSnake[indexOf(segment)] = 1;
        }
    }

    std::vector<GridPosition> eligible;
    eligible.reserve(CellCount);

    for (int y = 0; y < config::GridRows; ++y) {
        for (int x = 0; x < config::GridColumns; ++x) {
            const GridPosition cell{x, y};

            if (onSnake[indexOf(cell)] != 0) {
                continue;
            }
            if (food && *food == cell) {
                continue;
            }
            if (!snake.empty() &&
                chebyshevDistance(cell, snake.front()) < minHeadDistance) {
                continue;
            }
            eligible.push_back(cell);
        }
    }

    // Largest connected layout found so far. Only connected layouts are ever
    // stored here, so whatever is returned is safe to play on.
    Obstacles best;

    for (int pass = 0; pass < MaxPlacementPasses && best.size() < count; ++pass) {
        Obstacles chosen;

        // Visit the eligible cells in a fresh random order (a Fisher-Yates
        // shuffle done one step at a time) and keep each cell that leaves
        // every open cell reachable. The layout stays connected after every
        // step, so it can never end up split.
        for (std::size_t i = 0; i < eligible.size() && chosen.size() < count; ++i) {
            std::uniform_int_distribution<std::size_t> pick(i, eligible.size() - 1);
            std::swap(eligible[i], eligible[pick(random)]);

            chosen.push_back(eligible[i]);
            if (!keepsBoardConnected(chosen)) {
                chosen.pop_back();
            }
        }

        if (chosen.size() > best.size()) {
            best = std::move(chosen);
        }
    }

    return best;
}

// ---- Classic ---------------------------------------------------------------

GameMode ClassicMode::mode() const noexcept {
    return GameMode::Classic;
}

std::optional<std::chrono::milliseconds> ClassicMode::timeLimit() const noexcept {
    return noTimeLimit();
}

int ClassicMode::foodPoints() const noexcept {
    return config::ClassicFoodPoints;
}

Obstacles ClassicMode::placeObstacles(std::mt19937& /*random*/,
                                      const std::deque<GridPosition>& /*snake*/,
                                      std::optional<GridPosition> /*food*/) const {
    return {};
}

bool ClassicMode::isFatal(GridPosition cell, const Obstacles& /*obstacles*/) const noexcept {
    return !isInsideGrid(cell);
}

// ---- Timed Challenge -------------------------------------------------------

GameMode TimedMode::mode() const noexcept {
    return GameMode::Timed;
}

std::optional<std::chrono::milliseconds> TimedMode::timeLimit() const noexcept {
    return std::chrono::milliseconds{config::TimedChallengeDurationMs};
}

int TimedMode::foodPoints() const noexcept {
    return config::TimedFoodPoints;
}

Obstacles TimedMode::placeObstacles(std::mt19937& /*random*/,
                                    const std::deque<GridPosition>& /*snake*/,
                                    std::optional<GridPosition> /*food*/) const {
    return {};
}

bool TimedMode::isFatal(GridPosition cell, const Obstacles& /*obstacles*/) const noexcept {
    return !isInsideGrid(cell);
}

// ---- Obstacle --------------------------------------------------------------

GameMode ObstacleMode::mode() const noexcept {
    return GameMode::Obstacle;
}

std::optional<std::chrono::milliseconds> ObstacleMode::timeLimit() const noexcept {
    return noTimeLimit();
}

int ObstacleMode::foodPoints() const noexcept {
    return config::ObstacleFoodPoints;
}

Obstacles ObstacleMode::placeObstacles(std::mt19937& random,
                                       const std::deque<GridPosition>& snake,
                                       std::optional<GridPosition> food) const {
    return placeObstacleCells(obstacleCount(), random, snake, food,
                              config::ObstacleMinHeadDistance);
}

bool ObstacleMode::isFatal(GridPosition cell, const Obstacles& obstacles) const noexcept {
    return !isInsideGrid(cell) || contains(obstacles, cell);
}

// ---- Factory ---------------------------------------------------------------

std::unique_ptr<IModeStrategy> makeModeStrategy(GameMode mode) {
    switch (mode) {
    case GameMode::Classic:
        return std::make_unique<ClassicMode>();
    case GameMode::Timed:
        return std::make_unique<TimedMode>();
    case GameMode::Obstacle:
        return std::make_unique<ObstacleMode>();
    }
    return std::make_unique<ClassicMode>();
}

} // namespace snake

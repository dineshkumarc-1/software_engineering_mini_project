
#include "snakextreme/GameEngine.hpp"
#include "snakextreme/GameConfig.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace snake {

namespace {

void validateInitialSnake(
    const std::deque<GridPosition>& body
) {
    if (body.empty()) {
        throw std::invalid_argument(
            "Initial snake must not be empty"
        );
    }
}

} // namespace

GameEngine::GameEngine() {
    const int centerX = config::GridColumns / 2;
    const int centerY = config::GridRows / 2;

    snake_.push_back({centerX, centerY});
    snake_.push_back({centerX - 1, centerY});
    snake_.push_back({centerX - 2, centerY});

    spawnFood();
}

GameEngine::GameEngine(
    std::deque<GridPosition> initialSnake,
    Direction initialDirection
)
    : snake_(std::move(initialSnake)),
      direction_(initialDirection) {
    validateInitialSnake(snake_);
    spawnFood();
}

GameEngine::GameEngine(
    std::deque<GridPosition> initialSnake,
    Direction initialDirection,
    GridPosition initialFood
)
    : snake_(std::move(initialSnake)),
      direction_(initialDirection) {
    validateInitialSnake(snake_);

    const bool inside =
        initialFood.x >= 0 &&
        initialFood.x < config::GridColumns &&
        initialFood.y >= 0 &&
        initialFood.y < config::GridRows;

    const bool occupied = std::find(
        snake_.begin(), snake_.end(), initialFood
    ) != snake_.end();

    if (!inside || occupied) {
        throw std::invalid_argument(
            "Initial food must be inside the grid and off the snake"
        );
    }

    food_ = initialFood;
    hasFood_ = true;
}

GameEngine::GameEngine(
    std::deque<GridPosition> initialSnake,
    Direction initialDirection,
    GridPosition initialFood,
    std::function<GridPosition(
        const std::vector<GridPosition>&
    )> foodSelector
)
    : GameEngine(
          std::move(initialSnake),
          initialDirection,
          initialFood
      ) {
    if (!foodSelector) {
        throw std::invalid_argument(
            "Food selector must not be empty"
        );
    }

    foodSelector_ = std::move(foodSelector);
}

const std::deque<GridPosition>& GameEngine::snake() const noexcept {
    return snake_;
}

Direction GameEngine::direction() const noexcept {
    return direction_;
}

GridPosition GameEngine::food() const noexcept {
    return food_;
}

bool GameEngine::hasFood() const noexcept {
    return hasFood_;
}


std::size_t GameEngine::foodEatenCount() const noexcept {
    return foodEatenCount_;
}

void GameEngine::changeDirection(Direction next) { (void)tryQueueDirection(next); }

bool GameEngine::tryQueueDirection(Direction next) {
    if (gameOver_) {
        return false;
    }

    const Direction reference =
        directionQueue_.empty()
            ? direction_
            : directionQueue_.back();

    const bool reversing =
        (reference == Direction::Up && next == Direction::Down) ||
        (reference == Direction::Down && next == Direction::Up) ||
        (reference == Direction::Left && next == Direction::Right) ||
        (reference == Direction::Right && next == Direction::Left);

    if (reversing || next == reference) {
        return false;
    }

    if (directionQueue_.size() < 2) {
        directionQueue_.push_back(next);
        return true;
    }
    return false;
}

bool GameEngine::spawnFood() {
    std::vector<GridPosition> freeCells;

    for (int y = 0; y < config::GridRows; ++y) {
        for (int x = 0; x < config::GridColumns; ++x) {
            const GridPosition cell{x, y};

            const bool occupied = std::find(
                snake_.begin(), snake_.end(), cell
            ) != snake_.end() || isObstacle(cell);

            if (!occupied) {
                freeCells.push_back(cell);
            }
        }
    }

    if (freeCells.empty()) {
        hasFood_ = false;
        gameOver_ = true;
        gameOverReason_ = GameOverReason::BOARD_FULL;
        return false;
    }

    // Keep new food off excluded cells (a power-up) while another cell is
    // free; with no other choice, an excluded cell is still used.
    if (!foodExclusions_.empty()) {
        std::vector<GridPosition> preferred;
        preferred.reserve(freeCells.size());
        std::copy_if(freeCells.begin(), freeCells.end(), std::back_inserter(preferred),
                     [this](const GridPosition& cell) {
                         return std::find(foodExclusions_.begin(), foodExclusions_.end(),
                                          cell) == foodExclusions_.end();
                     });
        if (!preferred.empty()) {
            freeCells = std::move(preferred);
        }
    }

    if (foodSelector_) {
        const GridPosition selected = foodSelector_(freeCells);

        const bool isFree = std::find(
            freeCells.begin(), freeCells.end(), selected
        ) != freeCells.end();

        if (!isFree) {
            throw std::invalid_argument(
                "Food selector must choose a free cell"
            );
        }

        food_ = selected;
    } else {
        std::uniform_int_distribution<std::size_t> choose(
            0, freeCells.size() - 1
        );

        food_ = freeCells[choose(randomEngine_)];
    }

    hasFood_ = true;
    return true;
}

void GameEngine::tick() {
    if (gameOver_) {
        return;
    }

    if (!directionQueue_.empty()) {
        direction_ = directionQueue_.front();
        directionQueue_.pop_front();
        ++appliedDirectionCount_;
    }

    GridPosition nextHead = snake_.front();

    switch (direction_) {
    case Direction::Up:
        --nextHead.y;
        break;

    case Direction::Down:
        ++nextHead.y;
        break;

    case Direction::Left:
        --nextHead.x;
        break;

    case Direction::Right:
        ++nextHead.x;
        break;
    }

    if (nextHead.x < 0 ||
        nextHead.x >= config::GridColumns ||
        nextHead.y < 0 ||
        nextHead.y >= config::GridRows) {
        gameOver_ = true;
        gameOverReason_ = GameOverReason::WALL;
        return;
    }

    if (isObstacle(nextHead)) {
        gameOver_ = true;
        gameOverReason_ = GameOverReason::OBSTACLE;
        return;
    }

    const bool eating = hasFood_ && nextHead == food_;

    // The tail remains occupied when eating; otherwise it vacates.
    const std::size_t segmentsToCheck =
        snake_.size() - (eating ? 0U : 1U);

    for (std::size_t i = 0; i < segmentsToCheck; ++i) {
        if (nextHead == snake_[i]) {
            gameOver_ = true;
            gameOverReason_ = GameOverReason::SELF;
            return;
        }
    }


    snake_.push_front(nextHead);

    if (eating) {
        ++foodEatenCount_;
        spawnFood();
    } else {
        snake_.pop_back();
    }
}  // This closes GameEngine::tick()


bool GameEngine::isGameOver() const noexcept {
    return gameOver_;
}

GameOverReason GameEngine::gameOverReason() const noexcept {
    return gameOverReason_;
}

void GameEngine::setObstacles(const std::vector<GridPosition>& obstacles) {
    std::vector<GridPosition> unique;
    unique.reserve(obstacles.size());

    for (const GridPosition& cell : obstacles) {
        const bool inside =
            cell.x >= 0 &&
            cell.x < config::GridColumns &&
            cell.y >= 0 &&
            cell.y < config::GridRows;

        if (!inside) {
            throw std::invalid_argument(
                "Obstacles must be inside the grid"
            );
        }

        if (std::find(snake_.begin(), snake_.end(), cell) != snake_.end()) {
            throw std::invalid_argument(
                "Obstacles must not overlap the snake"
            );
        }

        if (std::find(unique.begin(), unique.end(), cell) == unique.end()) {
            unique.push_back(cell);
        }
    }

    obstacles_ = std::move(unique);

    if (hasFood_ && isObstacle(food_)) {
        spawnFood();
    }
}

const std::vector<GridPosition>& GameEngine::obstacles() const noexcept {
    return obstacles_;
}

void GameEngine::endGame(GameOverReason reason) noexcept {
    if (gameOver_ || reason == GameOverReason::None) {
        return;
    }

    gameOver_ = true;
    gameOverReason_ = reason;
}

bool GameEngine::isObstacle(GridPosition cell) const noexcept {
    return std::find(obstacles_.begin(), obstacles_.end(), cell) !=
           obstacles_.end();
}

void GameEngine::setFoodExclusions(std::vector<GridPosition> cells) {
    foodExclusions_ = std::move(cells);
}

const std::vector<GridPosition>& GameEngine::foodExclusions() const noexcept {
    return foodExclusions_;
}

std::size_t GameEngine::shrink(std::size_t segments,
                               std::size_t minimumLength) noexcept {
    if (gameOver_) {
        return 0;
    }

    const std::size_t keep = minimumLength > 0 ? minimumLength : 1;
    std::size_t removed = 0;
    while (removed < segments && snake_.size() > keep) {
        snake_.pop_back();
        ++removed;
    }
    return removed;
}

} // namespace snake

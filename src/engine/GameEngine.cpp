#include "snakextreme/GameEngine.hpp"
#include "snakextreme/GameConfig.hpp"

#include <algorithm>
#include <stdexcept>
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
    : snake_(initialSnake), direction_(initialDirection) {
    validateInitialSnake(snake_);
    spawnFood();
}

GameEngine::GameEngine(
    std::deque<GridPosition> initialSnake,
    Direction initialDirection,
    GridPosition initialFood
)
    : snake_(initialSnake), direction_(initialDirection) {
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

void GameEngine::changeDirection(Direction next) {
    if (gameOver_) {
        return;
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
        return;
    }

    if (directionQueue_.size() < 2) {
        directionQueue_.push_back(next);
    }
}

bool GameEngine::spawnFood() {
    std::vector<GridPosition> freeCells;

    for (int y = 0; y < config::GridRows; ++y) {
        for (int x = 0; x < config::GridColumns; ++x) {
            const GridPosition cell{x, y};

            const bool occupied = std::find(
                snake_.begin(), snake_.end(), cell
            ) != snake_.end();

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

    std::uniform_int_distribution<std::size_t> choose(
        0, freeCells.size() - 1
    );

    food_ = freeCells[choose(randomEngine_)];
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
        spawnFood();
    } else {
        snake_.pop_back();
    }
}

bool GameEngine::isGameOver() const noexcept {
    return gameOver_;
}

GameOverReason GameEngine::gameOverReason() const noexcept {
    return gameOverReason_;
}

} // namespace snake
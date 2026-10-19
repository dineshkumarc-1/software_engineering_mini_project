
#include "snakextreme/GameEngine.hpp"
#include "snakextreme/GameConfig.hpp"

namespace snake {

// Default constructor: create a three-segment snake at the grid centre.
GameEngine::GameEngine() {
    const int centerX = config::GridColumns / 2;
    const int centerY = config::GridRows / 2;

    // The head is at the front; the snake initially faces right.
    snake_.push_back({centerX, centerY});
    snake_.push_back({centerX - 1, centerY});
    snake_.push_back({centerX - 2, centerY});
}

// Constructor for testing specific snake configurations.
GameEngine::GameEngine(std::deque<GridPosition> initialSnake,
                       Direction initialDirection)
    : snake_(initialSnake), direction_(initialDirection) {
}

// Return the snake's segments, with the head at the front.
const std::deque<GridPosition>& GameEngine::snake() const noexcept {
    return snake_;
}

// Return the current direction.
Direction GameEngine::direction() const noexcept {
    return direction_;
}

// Queue a direction change while preventing immediate reversals.
void GameEngine::changeDirection(Direction next) {
    // Ignore input after game over.
    if (gameOver_) {
        return;
    }

    // Compare with the last queued direction, if one exists.
    const Direction reference =
        directionQueue_.empty() ? direction_ : directionQueue_.back();

    const bool reversing =
        (reference == Direction::Up && next == Direction::Down) ||
        (reference == Direction::Down && next == Direction::Up) ||
        (reference == Direction::Left && next == Direction::Right) ||
        (reference == Direction::Right && next == Direction::Left);

    if (reversing) {
        return;
    }

    // Ignore duplicate directions.
    if (next == reference) {
        return;
    }

    // Allow at most two pending turns.
    if (directionQueue_.size() < 2) {
        directionQueue_.push_back(next);
    }
}

// Advance the snake by one grid cell.
void GameEngine::tick() {
    // Do not move after game over.
    if (gameOver_) {
        return;
    }

    // Apply at most one queued direction per tick.
    if (!directionQueue_.empty()) {
        direction_ = directionQueue_.front();
        directionQueue_.pop_front();
    }

    // Calculate the next head position.
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

    // Check collision with the grid boundaries.
    if (nextHead.x < 0 ||
        nextHead.x >= config::GridColumns ||
        nextHead.y < 0 ||
        nextHead.y >= config::GridRows) {
        gameOver_ = true;
        return;
    }

    // Check whether the next head position intersects the snake.
    for (const auto& segment : snake_) {
        if (nextHead == segment) {
            gameOver_ = true;
            return;
        }
    }

    // The move is valid: add the new head and remove the tail.
    snake_.push_front(nextHead);
    snake_.pop_back();
}

// Report whether a collision has ended the game.
bool GameEngine::isGameOver() const noexcept {
    return gameOver_;
}

} // namespace snake

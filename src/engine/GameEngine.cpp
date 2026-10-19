
#include "snakextreme/GameEngine.hpp"
#include "snakextreme/GameConfig.hpp"

#include <algorithm>
#include <vector>

namespace snake {

// Default constructor: create a three-segment snake at the grid centre.
GameEngine::GameEngine() {
    const int centerX = config::GridColumns / 2;
    const int centerY = config::GridRows / 2;

    // The head is at the front; the snake initially faces right.
    snake_.push_back({centerX, centerY});
    snake_.push_back({centerX - 1, centerY});
    snake_.push_back({centerX - 2, centerY});

    // Place the first food item on an unoccupied cell.
    spawnFood();
}

// Constructor for testing specific snake configurations.
GameEngine::GameEngine(std::deque<GridPosition> initialSnake,
                       Direction initialDirection)
    : snake_(initialSnake), direction_(initialDirection) {
    spawnFood();
}

// Constructor for tests that specify the initial food position.
GameEngine::GameEngine(std::deque<GridPosition> initialSnake,
                       Direction initialDirection,
                       GridPosition initialFood)
    : snake_(initialSnake), direction_(initialDirection) {
    const bool insideGrid =
        initialFood.x >= 0 &&
        initialFood.x < config::GridColumns &&
        initialFood.y >= 0 &&
        initialFood.y < config::GridRows;

    const bool occupied = std::find(
        snake_.begin(), snake_.end(), initialFood) != snake_.end();

    if (insideGrid && !occupied) {
        food_ = initialFood;
        hasFood_ = true;
    }
}

// Return the snake's segments, with the head at the front.
const std::deque<GridPosition>& GameEngine::snake() const noexcept {
    return snake_;
}

// Return the current direction.
Direction GameEngine::direction() const noexcept {
    return direction_;
}

// Return the current food position.
GridPosition GameEngine::food() const noexcept {
    return food_;
}

// Return whether food currently exists on the board.
bool GameEngine::hasFood() const noexcept {
    return hasFood_;
}

// Spawn food randomly on an unoccupied grid cell.
bool GameEngine::spawnFood() {
    std::vector<GridPosition> availableCells;

    // Collect all cells that are not occupied by the snake.
    for (int y = 0; y < config::GridRows; ++y) {
        for (int x = 0; x < config::GridColumns; ++x) {
            const GridPosition candidate{x, y};

            const bool occupied = std::find(
                snake_.begin(), snake_.end(), candidate) != snake_.end();

            if (!occupied) {
                availableCells.push_back(candidate);
            }
        }
    }

    // If the board is full, no food can be spawned.
    if (availableCells.empty()) {
        hasFood_ = false;
        gameOver_ = true;
        return false;
    }

    // Select one available cell uniformly at random.
    std::uniform_int_distribution<std::size_t> distribution(
        0, availableCells.size() - 1);

    food_ = availableCells[distribution(randomEngine_)];
    hasFood_ = true;

    return true;
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

    // Determine whether the snake is about to eat food.
    const bool eating = hasFood_ && nextHead == food_;

    // Check collision with the snake's body.
    // On a normal move, the tail vacates its cell.
    for (std::size_t i = 0; i < snake_.size(); ++i) {
        if (!eating && i == snake_.size() - 1) {
            continue;
        }

        if (nextHead == snake_[i]) {
            gameOver_ = true;
            return;
        }
    }

    // Add the new head.
    snake_.push_front(nextHead);

    if (eating) {
        // Keep the tail, increasing the length by exactly one.
        // Spawn replacement food immediately.
        spawnFood();
    } else {
        // Normal movement preserves the snake's length.
        snake_.pop_back();
    }
}

// Report whether a collision or a full board has ended the game.
bool GameEngine::isGameOver() const noexcept {
    return gameOver_;
}

} // namespace snake


#include "snakextreme/GameEngine.hpp"
#include "snakextreme/GameConfig.hpp"

namespace snake {

GameEngine::GameEngine() {
    const int centerX = config::GridColumns / 2;
    const int centerY = config::GridRows / 2;

    // The head is at the front; the snake initially faces right.
    snake_.push_back({centerX, centerY});
    snake_.push_back({centerX - 1, centerY});
    snake_.push_back({centerX - 2, centerY});
}

const std::deque<GridPosition>& GameEngine::snake() const noexcept {
    return snake_;
}

Direction GameEngine::direction() const noexcept {
    return direction_;
}

void GameEngine::changeDirection(Direction next) {
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

void GameEngine::tick() {
    // Apply at most one queued direction per movement tick.
    if (!directionQueue_.empty()) {
        direction_ = directionQueue_.front();
        directionQueue_.pop_front();
    }

    GridPosition head = snake_.front();

    switch (direction_) {
    case Direction::Up:
        --head.y;
        break;
    case Direction::Down:
        ++head.y;
        break;
    case Direction::Left:
        --head.x;
        break;
    case Direction::Right:
        ++head.x;
        break;
    }

    snake_.push_front(head);
    snake_.pop_back();
}

} // namespace snake

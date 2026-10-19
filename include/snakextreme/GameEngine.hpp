
#pragma once

#include <cstddef>
#include <deque>

namespace snake {

struct GridPosition {
    int x{};
    int y{};

    bool operator==(const GridPosition& other) const noexcept {
        return x == other.x && y == other.y;
    }

    bool operator!=(const GridPosition& other) const noexcept {
        return !(*this == other);
    }
};

enum class Direction {
    Up,
    Down,
    Left,
    Right
};

class GameEngine {
public:
    GameEngine();

    const std::deque<GridPosition>& snake() const noexcept;
    Direction direction() const noexcept;

    void changeDirection(Direction next);
    void tick();

private:
    std::deque<GridPosition> snake_;
    std::deque<Direction> directionQueue_;
    Direction direction_{Direction::Right};
};

} // namespace snake

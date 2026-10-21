
#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <random>
#include <vector>

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

enum class GameOverReason {
    None,
    WALL,
    SELF,
    BOARD_FULL
};

class GameEngine {
public:
    GameEngine();

    explicit GameEngine(
        std::deque<GridPosition> initialSnake,
        Direction initialDirection
    );

    // Constructor for deterministic initial-food tests.
    GameEngine(
        std::deque<GridPosition> initialSnake,
        Direction initialDirection,
        GridPosition initialFood
    );

    // Constructor allowing deterministic food placement in tests.
    GameEngine(
        std::deque<GridPosition> initialSnake,
        Direction initialDirection,
        GridPosition initialFood,
        std::function<GridPosition(
            const std::vector<GridPosition>&
        )> foodSelector
    );

    const std::deque<GridPosition>& snake() const noexcept;
    Direction direction() const noexcept;

    GridPosition food() const noexcept;
    bool hasFood() const noexcept;

    // Total food items consumed during this engine instance.
    std::size_t foodEatenCount() const noexcept;

    void changeDirection(Direction next);
    void tick();

    bool isGameOver() const noexcept;
    GameOverReason gameOverReason() const noexcept;

private:
    std::deque<GridPosition> snake_;
    std::deque<Direction> directionQueue_;
    Direction direction_{Direction::Right};

    bool gameOver_{false};
    GridPosition food_{};
    bool hasFood_{false};

    // Counts food consumed, starting from zero.
    std::size_t foodEatenCount_{0};

    std::mt19937 randomEngine_{std::random_device{}()};

    std::function<GridPosition(
        const std::vector<GridPosition>&
    )> foodSelector_;

    bool spawnFood();

    GameOverReason gameOverReason_{GameOverReason::None};
};

} // namespace snake

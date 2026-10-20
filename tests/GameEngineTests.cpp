
#include "snakextreme/GameConfig.hpp"
#include "snakextreme/GameEngine.hpp"

#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <deque>

TEST_CASE("Initial snake has the correct state",
          "[engine][movement]") {
    snake::GameEngine game;
    const auto& body = game.snake();

    REQUIRE(body.size() == snake::config::InitialSnakeLength);
    REQUIRE(body.front() == snake::GridPosition{15, 10});
    REQUIRE(body[1] == snake::GridPosition{14, 10});
    REQUIRE(body[2] == snake::GridPosition{13, 10});
    REQUIRE(game.direction() == snake::Direction::Right);
}

TEST_CASE("Snake moves one cell per tick",
          "[engine][movement]") {
    std::deque<snake::GridPosition> body = {
        {5, 5}, {4, 5}, {3, 5}
    };

    snake::GameEngine game(
        body, snake::Direction::Right, {10, 10});

    game.tick();

    REQUIRE(game.snake().front() == snake::GridPosition{6, 5});
    REQUIRE(game.snake().size() == 3);
    REQUIRE(game.snake().back() == snake::GridPosition{4, 5});
}

TEST_CASE("Direct reversal is rejected",
          "[engine][direction]") {
    std::deque<snake::GridPosition> body = {
        {5, 5}, {4, 5}, {3, 5}
    };

    snake::GameEngine game(
        body, snake::Direction::Right, {10, 10});

    game.changeDirection(snake::Direction::Left);
    game.tick();

    REQUIRE(game.direction() == snake::Direction::Right);
    REQUIRE(game.snake().front() == snake::GridPosition{6, 5});
}

TEST_CASE("Two queued turns are applied in order",
          "[engine][direction]") {
    std::deque<snake::GridPosition> body = {
        {5, 5}, {4, 5}, {3, 5}
    };

    snake::GameEngine game(
        body, snake::Direction::Right, {10, 10});

    game.changeDirection(snake::Direction::Up);
    game.changeDirection(snake::Direction::Left);

    game.tick();

    REQUIRE(game.direction() == snake::Direction::Up);
    REQUIRE(game.snake().front() == snake::GridPosition{5, 4});

    game.tick();

    REQUIRE(game.direction() == snake::Direction::Left);
    REQUIRE(game.snake().front() == snake::GridPosition{4, 4});
}

TEST_CASE("Direction queue accepts at most two turns",
          "[engine][direction]") {
    std::deque<snake::GridPosition> body = {
        {5, 5}, {4, 5}, {3, 5}
    };

    snake::GameEngine game(
        body, snake::Direction::Right, {10, 10});

    game.changeDirection(snake::Direction::Up);
    game.changeDirection(snake::Direction::Left);
    game.changeDirection(snake::Direction::Down);

    game.tick();
    REQUIRE(game.direction() == snake::Direction::Up);

    game.tick();
    REQUIRE(game.direction() == snake::Direction::Left);

    game.tick();
    REQUIRE(game.direction() == snake::Direction::Left);
}


TEST_CASE("Reversal of the last queued direction is rejected",
          "[engine][direction]") {
    std::deque<snake::GridPosition> body = {
        {5, 5}, {4, 5}, {3, 5}
    };

    snake::GameEngine game(
        body, snake::Direction::Right, {10, 10});

    game.changeDirection(snake::Direction::Up);
    game.changeDirection(snake::Direction::Down);

    // The Down turn reverses the queued Up turn, so it must be rejected.
    game.tick();

    REQUIRE(game.direction() == snake::Direction::Up);
    REQUIRE(game.snake().front() == snake::GridPosition{5, 4});

    // Check that the rejected reversal does not take effect on the next tick.
    game.tick();

    REQUIRE(game.direction() == snake::Direction::Up);
    REQUIRE(game.snake().front() == snake::GridPosition{5, 3});
}


TEST_CASE("Wall collision ends the game",
          "[engine][collision]") {
    std::deque<snake::GridPosition> body = {
        {snake::config::GridColumns - 1, 5},
        {snake::config::GridColumns - 2, 5},
        {snake::config::GridColumns - 3, 5}
    };

    snake::GameEngine game(
        body, snake::Direction::Right, {10, 10});

    game.tick();

    REQUIRE(game.isGameOver());
    REQUIRE(game.gameOverReason() == snake::GameOverReason::WALL);
}

TEST_CASE("Snake does not move after wall collision",
          "[engine][collision]") {
    std::deque<snake::GridPosition> body = {
        {snake::config::GridColumns - 1, 5},
        {snake::config::GridColumns - 2, 5},
        {snake::config::GridColumns - 3, 5}
    };

    snake::GameEngine game(
        body, snake::Direction::Right, {10, 10});

    game.tick();
    const auto bodyAfterCollision = game.snake();

    game.tick();

    REQUIRE(game.snake() == bodyAfterCollision);
    REQUIRE(game.gameOverReason() == snake::GameOverReason::WALL);
}

TEST_CASE("Wall collision is detected at all four boundaries",
          "[engine][collision][walls]") {
    SECTION("Left boundary") {
        std::deque<snake::GridPosition> body = {
            {0, 5}, {1, 5}, {2, 5}
        };

        snake::GameEngine game(
            body, snake::Direction::Left, {10, 10});

        game.tick();

        REQUIRE(game.isGameOver());
        REQUIRE(game.gameOverReason() == snake::GameOverReason::WALL);
    }

    SECTION("Right boundary") {
        std::deque<snake::GridPosition> body = {
            {snake::config::GridColumns - 1, 5},
            {snake::config::GridColumns - 2, 5},
            {snake::config::GridColumns - 3, 5}
        };

        snake::GameEngine game(
            body, snake::Direction::Right, {10, 10});

        game.tick();

        REQUIRE(game.isGameOver());
        REQUIRE(game.gameOverReason() == snake::GameOverReason::WALL);
    }

    SECTION("Top boundary") {
        std::deque<snake::GridPosition> body = {
            {5, 0}, {5, 1}, {5, 2}
        };

        snake::GameEngine game(
            body, snake::Direction::Up, {10, 10});

        game.tick();

        REQUIRE(game.isGameOver());
        REQUIRE(game.gameOverReason() == snake::GameOverReason::WALL);
    }

    SECTION("Bottom boundary") {
        std::deque<snake::GridPosition> body = {
            {5, snake::config::GridRows - 1},
            {5, snake::config::GridRows - 2},
            {5, snake::config::GridRows - 3}
        };

        snake::GameEngine game(
            body, snake::Direction::Down, {10, 10});

        game.tick();

        REQUIRE(game.isGameOver());
        REQUIRE(game.gameOverReason() == snake::GameOverReason::WALL);
    }
}

TEST_CASE("Self-collision ends the game",
          "[engine][collision]") {
    std::deque<snake::GridPosition> body = {
        {2, 2}, {1, 2}, {1, 1}, {2, 1},
        {3, 1}, {3, 2}, {4, 2}
    };

    snake::GameEngine game(
        body, snake::Direction::Right, {10, 10});

    game.tick();

    REQUIRE(game.isGameOver());
    REQUIRE(game.gameOverReason() == snake::GameOverReason::SELF);
}

TEST_CASE("Moving into the vacating tail cell is safe",
          "[engine][collision]") {
    std::deque<snake::GridPosition> body = {
        {2, 2}, {2, 1}, {1, 1}, {1, 2}
    };

    snake::GameEngine game(
        body, snake::Direction::Left, {10, 10});

    game.tick();

    REQUIRE_FALSE(game.isGameOver());
    REQUIRE(game.snake().front() == snake::GridPosition{1, 2});
    REQUIRE(game.snake().size() == body.size());
}

TEST_CASE("Food spawns inside the grid and off the snake",
          "[engine][food]") {
    snake::GameEngine game;

    REQUIRE(game.hasFood());

    const auto food = game.food();

    REQUIRE(food.x >= 0);
    REQUIRE(food.x < snake::config::GridColumns);
    REQUIRE(food.y >= 0);
    REQUIRE(food.y < snake::config::GridRows);

    for (const auto& segment : game.snake()) {
        REQUIRE(food != segment);
    }
}

TEST_CASE("Eating food grows the snake by exactly one segment",
          "[engine][food][growth]") {
    std::deque<snake::GridPosition> body = {
        {5, 5}, {4, 5}, {3, 5}
    };

    const snake::GridPosition initialFood{6, 5};

    snake::GameEngine game(
        body, snake::Direction::Right, initialFood);

    game.tick();

    REQUIRE(game.snake().size() == body.size() + 1);
    REQUIRE(game.snake().front() == initialFood);
    REQUIRE(game.hasFood());

    for (const auto& segment : game.snake()) {
        REQUIRE(game.food() != segment);
    }
}



TEST_CASE("Ten consecutive food consumptions grow the snake by ten",
          "[engine][food][growth]") {
    std::deque<snake::GridPosition> body = {
        {5, 5}, {4, 5}, {3, 5}
    };

    int nextFoodX = 7;

    auto selector = [&nextFoodX](
        const std::vector<snake::GridPosition>& freeCells
    ) -> snake::GridPosition {
        const snake::GridPosition target{nextFoodX, 5};

        const auto found = std::find(
            freeCells.begin(), freeCells.end(), target
        );

        if (found == freeCells.end()) {
            throw std::runtime_error(
                "Expected next food cell to be free"
            );
        }

        ++nextFoodX;
        return target;
    };

    snake::GameEngine game(
        body,
        snake::Direction::Right,
        {6, 5},
        selector
    );

    for (int foodCount = 1; foodCount <= 10; ++foodCount) {
        const auto food = game.food();

        REQUIRE(game.hasFood());
        REQUIRE(game.snake().size() ==
                body.size() + static_cast<std::size_t>(foodCount - 1));

        game.tick();

        REQUIRE_FALSE(game.isGameOver());
        REQUIRE(game.snake().size() ==
                body.size() + static_cast<std::size_t>(foodCount));
        REQUIRE(game.snake().front() == food);

        for (const auto& segment : game.snake()) {
            REQUIRE(game.food() != segment);
        }
    }

    REQUIRE(game.snake().size() == 13);
}



TEST_CASE("Eating food respawns it on a free cell",
          "[engine][food]") {
    std::deque<snake::GridPosition> body = {
        {5, 5}, {4, 5}, {3, 5}
    };

    snake::GameEngine game(
        body, snake::Direction::Right, {6, 5});

    game.tick();

    REQUIRE(game.hasFood());

    for (const auto& segment : game.snake()) {
        REQUIRE(game.food() != segment);
    }
}

TEST_CASE("Food respawns on a free in-grid cell across 1000 trials",
          "[engine][food][stress]") {
    for (int trial = 0; trial < 1000; ++trial) {
        std::deque<snake::GridPosition> body = {
            {5, 5}, {4, 5}, {3, 5}
        };

        snake::GameEngine game(
            body, snake::Direction::Right, {6, 5});

        game.tick();

        REQUIRE(game.hasFood());
        REQUIRE_FALSE(game.isGameOver());

        const auto food = game.food();

        REQUIRE(food.x >= 0);
        REQUIRE(food.x < snake::config::GridColumns);
        REQUIRE(food.y >= 0);
        REQUIRE(food.y < snake::config::GridRows);

        for (const auto& segment : game.snake()) {
            REQUIRE(food != segment);
        }
    }
}

TEST_CASE("Food cannot be placed on an occupied cell",
          "[engine][food]") {
    std::deque<snake::GridPosition> body = {
        {5, 5}, {4, 5}, {3, 5}
    };

    REQUIRE_THROWS_AS(
        snake::GameEngine(
            body, snake::Direction::Right, {5, 5}),
        std::invalid_argument);
}

TEST_CASE("One remaining free cell receives the food",
          "[engine][food]") {
    const snake::GridPosition freeCell{5, 5};
    std::deque<snake::GridPosition> body;

    for (int y = 0; y < snake::config::GridRows; ++y) {
        for (int x = 0; x < snake::config::GridColumns; ++x) {
            snake::GridPosition cell{x, y};

            if (cell != freeCell) {
                body.push_back(cell);
            }
        }
    }

    snake::GameEngine game(body, snake::Direction::Right);

    REQUIRE(game.hasFood());
    REQUIRE(game.food() == freeCell);
    REQUIRE_FALSE(game.isGameOver());
}

TEST_CASE("An already full board ends with BOARD_FULL",
          "[engine][food][collision]") {
    std::deque<snake::GridPosition> body;

    for (int y = 0; y < snake::config::GridRows; ++y) {
        for (int x = 0; x < snake::config::GridColumns; ++x) {
            body.push_back({x, y});
        }
    }

    snake::GameEngine game(body, snake::Direction::Right);

    REQUIRE(game.isGameOver());
    REQUIRE(game.gameOverReason() == snake::GameOverReason::BOARD_FULL);
    REQUIRE_FALSE(game.hasFood());
}

TEST_CASE("Eating the last food fills the board",
          "[engine][food][growth][collision]") {
    const snake::GridPosition lastCell{1, 0};
    std::deque<snake::GridPosition> body;

    for (int y = 0; y < snake::config::GridRows; ++y) {
        for (int x = 0; x < snake::config::GridColumns; ++x) {
            snake::GridPosition cell{x, y};

            if (cell != lastCell && cell != snake::GridPosition{0, 0}) {
                body.push_back(cell);
            }
        }
    }

    // Put the head at (0, 0), facing right, with (1, 0) as the only
    // unoccupied cell. The rest of the body occupies all other cells.
    body.push_front({0, 0});

    snake::GameEngine game(
        body, snake::Direction::Right, lastCell);

    game.tick();

    REQUIRE(game.isGameOver());
    REQUIRE(game.gameOverReason() == snake::GameOverReason::BOARD_FULL);
    REQUIRE_FALSE(game.hasFood());
    REQUIRE(game.snake().size() == body.size() + 1);
}

TEST_CASE("Empty initial snake is rejected",
          "[engine][validation]") {
    const std::deque<snake::GridPosition> emptyBody;

    REQUIRE_THROWS_AS(
        snake::GameEngine(emptyBody, snake::Direction::Right),
        std::invalid_argument
    );

    REQUIRE_THROWS_AS(
        snake::GameEngine(
            emptyBody,
            snake::Direction::Right,
            snake::GridPosition{5, 5}
        ),
        std::invalid_argument
    );
}

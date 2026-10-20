
#include "snakextreme/GameConfig.hpp"
#include "snakextreme/GameEngine.hpp"

#include <catch2/catch_test_macros.hpp>
#include <deque>

TEST_CASE("Initial snake state", "[engine]") {
    snake::GameEngine game;
    const auto& body = game.snake();

    REQUIRE(body.size() == snake::config::InitialSnakeLength);
    REQUIRE(body.front() == snake::GridPosition{15, 10});
    REQUIRE(body[1] == snake::GridPosition{14, 10});
    REQUIRE(body[2] == snake::GridPosition{13, 10});
    REQUIRE(game.direction() == snake::Direction::Right);
    REQUIRE(game.gameOverReason() == snake::GameOverReason::None);
}

TEST_CASE("Snake moves one cell per tick", "[engine][movement]") {
    snake::GameEngine game;
    game.tick();

    REQUIRE(game.snake().front() == snake::GridPosition{16, 10});
    REQUIRE(game.snake().size() == 3);
    REQUIRE(game.snake().back() == snake::GridPosition{14, 10});
}

TEST_CASE("Direction changes and direct reversal is rejected",
          "[engine][movement]") {
    snake::GameEngine game;

    game.changeDirection(snake::Direction::Up);
    game.tick();

    REQUIRE(game.direction() == snake::Direction::Up);
    REQUIRE(game.snake().front() == snake::GridPosition{15, 9});

    game.changeDirection(snake::Direction::Down);
    game.tick();

    REQUIRE(game.direction() == snake::Direction::Up);
    REQUIRE(game.snake().front() == snake::GridPosition{15, 8});
}

TEST_CASE("Two queued turns are applied in order", "[engine][movement]") {
    snake::GameEngine game;

    game.changeDirection(snake::Direction::Up);
    game.changeDirection(snake::Direction::Left);

    game.tick();
    REQUIRE(game.direction() == snake::Direction::Up);
    REQUIRE(game.snake().front() == snake::GridPosition{15, 9});

    game.tick();
    REQUIRE(game.direction() == snake::Direction::Left);
    REQUIRE(game.snake().front() == snake::GridPosition{14, 9});
}

TEST_CASE("Direction queue limit is enforced", "[engine][movement]") {
    snake::GameEngine game;

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

TEST_CASE("Reverse of a queued direction is rejected", "[engine][movement]") {
    snake::GameEngine game;

    game.changeDirection(snake::Direction::Up);
    game.changeDirection(snake::Direction::Down);
    game.tick();

    REQUIRE(game.direction() == snake::Direction::Up);
}

TEST_CASE("Wall collision records the correct game-over reason",
          "[engine][collision]") {
    snake::GameEngine game;

    for (int i = 0; i < snake::config::GridColumns; ++i) {
        game.tick();
    }

    REQUIRE(game.isGameOver());
    REQUIRE(game.gameOverReason() == snake::GameOverReason::WALL);
}

TEST_CASE("Snake does not move after wall collision", "[engine][collision]") {
    snake::GameEngine game;

    for (int i = 0; i < snake::config::GridColumns; ++i) {
        game.tick();
    }

    const auto bodyAfterCollision = game.snake();
    game.tick();

    REQUIRE(game.snake() == bodyAfterCollision);
}

TEST_CASE("Further ticks do nothing after game over", "[engine][collision]") {
    snake::GameEngine game;

    for (int i = 0; i < snake::config::GridColumns; ++i) {
        game.tick();
    }

    const auto bodyAfterCollision = game.snake();

    game.changeDirection(snake::Direction::Up);
    game.tick();
    game.tick();

    REQUIRE(game.snake() == bodyAfterCollision);
}

TEST_CASE("Moving into the vacating tail cell is safe",
          "[engine][collision]") {
    std::deque<snake::GridPosition> body = {
        {2, 2},
        {2, 1},
        {1, 1},
        {1, 2}
    };

    snake::GameEngine game(body, snake::Direction::Left);
    game.tick();

    REQUIRE_FALSE(game.isGameOver());
    REQUIRE(game.gameOverReason() == snake::GameOverReason::None);
    REQUIRE(game.snake().front() == snake::GridPosition{1, 2});
    REQUIRE(game.snake().back() == snake::GridPosition{1, 1});
}

TEST_CASE("Self-collision records the correct game-over reason",
          "[engine][collision]") {
    std::deque<snake::GridPosition> body = {
        {2, 2},
        {1, 2},
        {1, 1},
        {2, 1},
        {3, 1},
        {3, 2},
        {4, 2}
    };

    snake::GameEngine game(body, snake::Direction::Right);
    game.tick();

    REQUIRE(game.isGameOver());
    REQUIRE(game.gameOverReason() == snake::GameOverReason::SELF);
}


#include "snakextreme/GameConfig.hpp"
#include "snakextreme/GameEngine.hpp"

#include <cstdlib>
#include <iostream>
#include <deque>

namespace {

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void testInitialSnake() {
    snake::GameEngine game;
    const auto& body = game.snake();

    check(body.size() == snake::config::InitialSnakeLength,
          "snake starts with three segments");
    check(body.front() == snake::GridPosition{15, 10},
          "snake head starts at grid centre");
    check(body[1] == snake::GridPosition{14, 10},
          "second segment starts behind the head");
    check(body[2] == snake::GridPosition{13, 10},
          "tail starts behind the second segment");
    check(game.direction() == snake::Direction::Right,
          "snake initially faces right");
    check(game.gameOverReason() == snake::GameOverReason::None,
          "game-over reason initially is None");
}

void testMovement() {
    snake::GameEngine game;
    game.tick();

    check(game.snake().front() == snake::GridPosition{16, 10},
          "tick moves the head one cell right");
    check(game.snake().size() == 3,
          "snake length stays constant without food");
    check(game.snake().back() == snake::GridPosition{14, 10},
          "tail moves forward after a safe move");
}

void testDirectionChanges() {
    snake::GameEngine game;
    game.changeDirection(snake::Direction::Up);
    game.tick();

    check(game.direction() == snake::Direction::Up,
          "snake accepts a perpendicular direction");
    check(game.snake().front() == snake::GridPosition{15, 9},
          "snake moves upward after turning");

    game.changeDirection(snake::Direction::Down);
    game.tick();

    check(game.direction() == snake::Direction::Up,
          "snake rejects a direct reversal after a tick");
    check(game.snake().front() == snake::GridPosition{15, 8},
          "snake continues upward after reversal is rejected");
}

void testTwoQueuedTurns() {
    snake::GameEngine game;

    game.changeDirection(snake::Direction::Up);
    game.changeDirection(snake::Direction::Left);

    game.tick();
    check(game.direction() == snake::Direction::Up,
          "first queued turn is applied first");
    check(game.snake().front() == snake::GridPosition{15, 9},
          "first queued turn moves the snake up");

    game.tick();
    check(game.direction() == snake::Direction::Left,
          "second queued turn is applied on the next tick");
    check(game.snake().front() == snake::GridPosition{14, 9},
          "second queued turn moves the snake left");
}

void testQueueLimit() {
    snake::GameEngine game;

    game.changeDirection(snake::Direction::Up);
    game.changeDirection(snake::Direction::Left);
    game.changeDirection(snake::Direction::Down);

    game.tick();
    check(game.direction() == snake::Direction::Up,
          "first queued turn remains first");

    game.tick();
    check(game.direction() == snake::Direction::Left,
          "second queued turn remains second");

    game.tick();
    check(game.direction() == snake::Direction::Left,
          "third turn is ignored when the queue is full");
}

void testQueuedReverseIsRejected() {
    snake::GameEngine game;

    game.changeDirection(snake::Direction::Up);
    game.changeDirection(snake::Direction::Down);

    game.tick();

    check(game.direction() == snake::Direction::Up,
          "reverse of the queued direction is rejected");
}

void testWallCollision() {
    snake::GameEngine game;

    // Move right until the next move would leave the grid.
    for (int i = 0; i < snake::config::GridColumns; ++i) {
        game.tick();
    }

    check(game.isGameOver(),
          "wall collision sets the game-over flag");
    check(game.gameOverReason() == snake::GameOverReason::WALL,
          "wall collision records WALL as the game-over reason");
}

void testSnakeDoesNotMoveAfterWallCollision() {
    snake::GameEngine game;

    for (int i = 0; i < snake::config::GridColumns; ++i) {
        game.tick();
    }

    const auto bodyAfterCollision = game.snake();

    game.tick();

    check(game.snake() == bodyAfterCollision,
          "snake remains in place after wall collision");
}

void testTickDoesNothingAfterGameOver() {
    snake::GameEngine game;

    for (int i = 0; i < snake::config::GridColumns; ++i) {
        game.tick();
    }

    const auto bodyAfterCollision = game.snake();

    game.changeDirection(snake::Direction::Up);
    game.tick();
    game.tick();

    check(game.snake() == bodyAfterCollision,
          "further ticks do not move the snake after game over");
}


void testSafeTailCellMovement() {
    // The head is at (2,2), moving left into the current tail cell (1,2).
    // The tail vacates that cell during this move, so this is safe.
    std::deque<snake::GridPosition> body = {
        {2, 2},
        {2, 1},
        {1, 1},
        {1, 2}
    };

    snake::GameEngine game(body, snake::Direction::Left);
    game.tick();

    check(!game.isGameOver(),
          "moving into the vacating tail cell is not a collision");
    check(game.gameOverReason() == snake::GameOverReason::None,
          "safe tail-cell movement leaves the game-over reason as None");
    check(game.snake().front() == snake::GridPosition{1, 2},
          "head moves into the vacating tail cell");
    check(game.snake().back() == snake::GridPosition{1, 1},
          "tail moves forward after a safe move");
}


void testSelfCollision() {
    // The head is at (2,2), moving right.
    // The next cell (3,2) is occupied by a non-tail segment.
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

    check(game.isGameOver(),
          "self-collision sets the game-over flag");
    check(game.gameOverReason() == snake::GameOverReason::SELF,
          "self-collision records SELF as the game-over reason");
}

} // namespace

int main() {
    testInitialSnake();
    testMovement();
    testDirectionChanges();
    testTwoQueuedTurns();
    testQueueLimit();
    testQueuedReverseIsRejected();

    testWallCollision();
    testSnakeDoesNotMoveAfterWallCollision();
    testTickDoesNothingAfterGameOver();
    testSafeTailCellMovement();
    testSelfCollision();

    std::cout << "All game engine tests passed.\n";
    return EXIT_SUCCESS;
}

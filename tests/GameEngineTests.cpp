
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

bool isInsideGrid(snake::GridPosition position) {
    return position.x >= 0 &&
           position.x < snake::config::GridColumns &&
           position.y >= 0 &&
           position.y < snake::config::GridRows;
}

bool isOnSnake(const snake::GameEngine& game,
               snake::GridPosition position) {
    for (const auto& segment : game.snake()) {
        if (segment == position) {
            return true;
        }
    }
    return false;
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
}

void testRandomFoodSpawnsOnFreeCell() {
    snake::GameEngine game;

    check(game.hasFood(),
          "food exists when the game starts");

    const auto food = game.food();

    check(isInsideGrid(food),
          "food spawns inside the grid");
    check(!isOnSnake(game, food),
          "food does not spawn on the snake");
}

void testEatingFoodGrowsSnakeAndRespawnsFood() {
    std::deque<snake::GridPosition> body = {
        {5, 5},
        {4, 5},
        {3, 5}
    };

    // The snake faces right, and food is directly in front of its head.
    const snake::GridPosition initialFood{6, 5};

    snake::GameEngine game(
        body, snake::Direction::Right, initialFood);

    check(game.hasFood(),
          "test food is placed successfully");
    check(game.food() == initialFood,
          "test food starts at the requested position");

    game.tick();

    check(game.snake().size() == body.size() + 1,
          "eating food increases snake length by exactly one");
    check(game.snake().front() == initialFood,
          "snake head moves onto the eaten food cell");
    check(game.hasFood(),
          "replacement food exists immediately after eating");
    check(isInsideGrid(game.food()),
          "replacement food is inside the grid");
    check(!isOnSnake(game, game.food()),
          "replacement food does not overlap the snake");
    check(game.food() != initialFood,
          "replacement food is different from the eaten cell");
}

void testMovement() {
    snake::GameEngine game;
    game.tick();

    check(game.snake().front() == snake::GridPosition{16, 10},
          "tick moves the head one cell right");
    check(game.snake().size() == 3,
          "snake length stays constant without food");
    check(game.snake().back() == snake::GridPosition{14, 10},
          "tail moves forward when the snake moves");
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
    check(game.direction() == snake::Direction::Up,
          "snake rejects a direct reversal");
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

    for (int i = 0; i < snake::config::GridColumns; ++i) {
        game.tick();
    }

    check(game.isGameOver(),
          "wall collision sets the game-over flag");
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

void testSelfCollision() {
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
}

} // namespace

int main() {
    testInitialSnake();
    testRandomFoodSpawnsOnFreeCell();
    testEatingFoodGrowsSnakeAndRespawnsFood();

    testMovement();
    testDirectionChanges();
    testTwoQueuedTurns();
    testQueueLimit();
    testQueuedReverseIsRejected();

    testWallCollision();
    testSnakeDoesNotMoveAfterWallCollision();
    testTickDoesNothingAfterGameOver();
    testSelfCollision();

    std::cout << "All game engine tests passed.\n";
    return EXIT_SUCCESS;
}

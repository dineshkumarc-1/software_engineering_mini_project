
#include "snakextreme/GameConfig.hpp"
#include "snakextreme/GameEngine.hpp"

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>

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

// Create a snake occupying every grid cell except freeCell.
std::deque<snake::GridPosition> snakeOccupyingAllBut(
    snake::GridPosition freeCell) {
    std::deque<snake::GridPosition> body;

    for (int y = 0; y < snake::config::GridRows; ++y) {
        for (int x = 0; x < snake::config::GridColumns; ++x) {
            snake::GridPosition cell{x, y};

            if (cell != freeCell) {
                body.push_back(cell);
            }
        }
    }

    return body;
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
    std::deque<snake::GridPosition> body = {
        {15, 10},
        {14, 10},
        {13, 10}
    };

    // Explicit food placement keeps this movement test deterministic.
    snake::GameEngine game(
        body, snake::Direction::Right, snake::GridPosition{20, 15});

    game.tick();

    check(game.snake().front() == snake::GridPosition{16, 10},
          "tick moves the head one cell right");
    check(game.snake().size() == 3,
          "snake length stays constant without food");
    check(game.snake().back() == snake::GridPosition{14, 10},
          "tail moves forward when the snake moves");
}

void testSafeTailCellMovement() {
    std::deque<snake::GridPosition> body = {
        {2, 2},
        {2, 3},
        {1, 3},
        {1, 2}
    };

    snake::GameEngine game(
        body,
        snake::Direction::Left,
        snake::GridPosition{5, 5});

    game.tick();

    check(!game.isGameOver(),
          "moving into the vacating tail cell is safe");
    check(game.snake().front() == snake::GridPosition{1, 2},
          "head moves into the tail's previous cell");
    check(game.snake().size() == body.size(),
          "safe tail-cell movement preserves snake length");
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
          "snake rejects a direct reversal before the tick");

    game.tick();

    check(game.direction() == snake::Direction::Up,
          "snake direction remains unchanged after rejected reversal");
    check(game.snake().front() == snake::GridPosition{15, 8},
          "snake continues upward after rejected reversal");
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
    check(game.snake().front() == snake::GridPosition{15, 9},
          "snake moves up rather than reversing after the tick");
}

void testOneRemainingFreeCell() {
    const snake::GridPosition lastCell{5, 5};
    auto body = snakeOccupyingAllBut(lastCell);

    snake::GameEngine game(body, snake::Direction::Right);

    check(game.hasFood(),
          "food spawns when one free cell remains");
    check(game.food() == lastCell,
          "food occupies the only remaining free cell");
    check(!game.isGameOver(),
          "board is not full while one free cell remains");
}

void testEatingLastFoodFillsBoard() {
    const snake::GridPosition lastCell{1, 0};
    auto body = snakeOccupyingAllBut(lastCell);

    // Reorder the body so its head starts at (0, 0), facing right.
    const auto head = std::find(
        body.begin(), body.end(), snake::GridPosition{0, 0});

    check(head != body.end(),
          "test setup contains the initial head cell");

    body.erase(head);
    body.push_front({0, 0});

    snake::GameEngine game(
        body,
        snake::Direction::Right,
        lastCell);

    check(game.hasFood(),
          "last food is available before the final move");

    game.tick();

    check(game.isGameOver(),
          "eating the last food ends the game");
    check(!game.hasFood(),
          "no food remains when the board is full");
    check(game.gameOverReason() == snake::GameOverReason::BOARD_FULL,
          "eating the last food reports BOARD_FULL");
    check(game.snake().size() == body.size() + 1,
          "eating the last food still grows the snake");
}

void testAlreadyFullBoardReportsBoardFull() {
    std::deque<snake::GridPosition> body;

    for (int y = 0; y < snake::config::GridRows; ++y) {
        for (int x = 0; x < snake::config::GridColumns; ++x) {
            body.push_back({x, y});
        }
    }

    snake::GameEngine game(body, snake::Direction::Right);

    check(game.isGameOver(),
          "an already-full board ends the game");
    check(!game.hasFood(),
          "an already-full board has no food");
    check(game.gameOverReason() == snake::GameOverReason::BOARD_FULL,
          "an already-full board reports BOARD_FULL");
}

void testWallCollision() {
    snake::GameEngine game;

    for (int i = 0; i < snake::config::GridColumns; ++i) {
        game.tick();
    }

    check(game.isGameOver(),
          "wall collision sets the game-over flag");
    check(game.gameOverReason() == snake::GameOverReason::WALL,
          "wall collision reports WALL game-over reason");
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
    check(game.gameOverReason() == snake::GameOverReason::SELF,
          "self-collision reports SELF game-over reason");
}

} // namespace

int main() {
    testInitialSnake();
    testRandomFoodSpawnsOnFreeCell();
    testEatingFoodGrowsSnakeAndRespawnsFood();

    testMovement();
    testSafeTailCellMovement();
    testDirectionChanges();
    testTwoQueuedTurns();
    testQueueLimit();
    testQueuedReverseIsRejected();

    testOneRemainingFreeCell();
    testEatingLastFoodFillsBoard();
    testAlreadyFullBoardReportsBoardFull();

    testWallCollision();
    testSnakeDoesNotMoveAfterWallCollision();
    testTickDoesNothingAfterGameOver();
    testSelfCollision();

    std::cout << "All game engine tests passed.\n";
    return EXIT_SUCCESS;
}

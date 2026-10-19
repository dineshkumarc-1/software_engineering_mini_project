
#include "snakextreme/GameConfig.hpp"
#include "snakextreme/GameEngine.hpp"

#include <cstdlib>
#include <iostream>

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

} // namespace

int main() {
    testInitialSnake();
    testMovement();
    testDirectionChanges();
    testTwoQueuedTurns();
    testQueueLimit();
    testQueuedReverseIsRejected();

    std::cout << "All game engine tests passed.\n";
    return EXIT_SUCCESS;
}

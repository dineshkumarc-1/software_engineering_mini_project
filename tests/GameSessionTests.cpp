
#include "snakextreme/GameSession.hpp"

#include <cstdlib>
#include <iostream>

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

} // namespace

int main() {
    using snake::GameMode;
    using snake::GameSession;

    GameSession session;

    expect(session.currentScore() == 0, "initial score is zero");
    expect(session.highScore() == 0, "initial high score is zero");

    session.foodEaten(GameMode::Classic);
    expect(session.currentScore() == 10, "classic adds 10 points");

    session.foodEaten(GameMode::Timed);
    expect(session.currentScore() == 20, "timed adds 10 points");

    session.foodEaten(GameMode::Obstacle);
    expect(session.currentScore() == 35, "obstacle adds 15 points");
    expect(session.highScore() == 35, "high score updates");

    session.start();
    expect(session.currentScore() == 0, "start resets current score");
    expect(session.highScore() == 35, "start preserves high score");

    std::cout << "All GameSession checks passed\n";
    return 0;
}

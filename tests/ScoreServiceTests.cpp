
#include "snakextreme/ScoreService.hpp"

#include <cstdlib>
#include <iostream>

namespace {

int checks = 0;

void expect(bool condition, const char* message) {
    ++checks;

    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

} // namespace

int main() {
    using snake::GameMode;
    using snake::ScoreService;

    ScoreService score;

    // 1. Initial scores
    expect(score.currentScore() == 0, "initial score is zero");
    expect(score.highScore() == 0, "initial high score is zero");

    // 2. Classic mode: 10 points
    score.foodEaten(GameMode::Classic);
    expect(score.currentScore() == 10, "classic gives 10 points");
    expect(score.highScore() == 10, "high score updates to 10");

    // 3. Timed mode: 10 points
    score.foodEaten(GameMode::Timed);
    expect(score.currentScore() == 20, "timed gives 10 points");

    // 4. Obstacle mode: 15 points
    score.foodEaten(GameMode::Obstacle);
    expect(score.currentScore() == 35, "obstacle gives 15 points");
    expect(score.highScore() == 35, "high score updates to 35");

    // 5. Reset current score, but preserve high score
    score.reset();
    expect(score.currentScore() == 0, "reset clears current score");
    expect(score.highScore() == 35, "reset preserves high score");

    // 6. A lower score must not replace the high score
    score.foodEaten(GameMode::Classic);
    expect(score.currentScore() == 10, "new game starts from zero");
    expect(score.highScore() == 35, "lower score does not replace high score");

    std::cout << checks << " score checks passed\n";
    return 0;
}

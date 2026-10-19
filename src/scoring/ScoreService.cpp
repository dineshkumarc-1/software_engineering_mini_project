
#include "snakextreme/ScoreService.hpp"

namespace snake {

void ScoreService::reset() noexcept {
    currentScore_ = 0;
}

void ScoreService::foodEaten(GameMode mode) noexcept {
    switch (mode) {
    case GameMode::Classic:
    case GameMode::Timed:
        currentScore_ += 10;
        break;

    case GameMode::Obstacle:
        currentScore_ += 15;
        break;
    }

    if (currentScore_ > highScore_) {
        highScore_ = currentScore_;
    }
}

int ScoreService::currentScore() const noexcept {
    return currentScore_;
}

int ScoreService::highScore() const noexcept {
    return highScore_;
}

} // namespace snake

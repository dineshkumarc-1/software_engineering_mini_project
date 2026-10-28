
#include "snakextreme/ScoreService.hpp"

namespace snake {

namespace {

// FR-016: points per food before any power-up multiplier.
int pointsPerFood(GameMode mode) noexcept {
    switch (mode) {
    case GameMode::Classic:
    case GameMode::Timed:
        return 10;

    case GameMode::Obstacle:
        return 15;
    }
    return 10;
}

} // namespace

void ScoreService::reset() noexcept {
    currentScore_ = 0;
    multiplier_ = 1;
}

void ScoreService::foodEaten(GameMode mode) noexcept {
    currentScore_ += pointsPerFood(mode) * multiplier_;

    if (currentScore_ > highScore_) {
        highScore_ = currentScore_;
    }
}

void ScoreService::setMultiplier(int multiplier) noexcept {
    multiplier_ = multiplier > 1 ? multiplier : 1;
}

int ScoreService::multiplier() const noexcept {
    return multiplier_;
}

int ScoreService::currentScore() const noexcept {
    return currentScore_;
}

int ScoreService::highScore() const noexcept {
    return highScore_;
}

} // namespace snake

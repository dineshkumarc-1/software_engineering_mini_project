#pragma once

#include "snakextreme/GameEngine.hpp"
#include "snakextreme/ScoreService.hpp"

namespace snake {

class GameSession {
public:
    void start() noexcept {
        scoreService_.reset();
    }

    void foodEaten(GameMode mode) noexcept {
        scoreService_.foodEaten(mode);
    }

    // Advance the engine and apply score changes for any food consumed
    // during this tick. Kept here so the integration can be tested headlessly.
    void tick(GameEngine& gameEngine, GameMode mode) {
        const auto previousCount = gameEngine.foodEatenCount();
        gameEngine.tick();

        if (gameEngine.foodEatenCount() > previousCount) {
            foodEaten(mode);
        }
    }

    [[nodiscard]] int currentScore() const noexcept {
        return scoreService_.currentScore();
    }

    [[nodiscard]] int highScore() const noexcept {
        return scoreService_.highScore();
    }

private:
    ScoreService scoreService_;
};

} // namespace snake

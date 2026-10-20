
#pragma once

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


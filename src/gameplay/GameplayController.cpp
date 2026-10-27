#include "snakextreme/GameplayController.hpp"

#include <exception>
#include <utility>

namespace snake {

bool GameplayController::start(SessionConfig config) {
    return begin(config, GameEngine{}, ModeManager{config});
}

bool GameplayController::start(SessionConfig config,
                               GameEngine initialEngine,
                               std::uint32_t seed) {
    return begin(config, std::move(initialEngine), ModeManager{config, seed});
}

bool GameplayController::begin(SessionConfig config,
                               GameEngine engine,
                               ModeManager modes) {
    // Set the new game up on the side and only swap it in once it is
    // complete, so a failure leaves the previous game as it was.
    try {
        std::optional<GridPosition> food;
        if (engine.hasFood()) {
            food = engine.food();
        }
        modes.startSession(engine.snake(), food);
        engine.setObstacles(modes.obstacles());
    } catch (const std::exception& error) {
        lastError_ = error.what();
        return false;
    }

    engine_ = std::move(engine);
    modes_ = std::move(modes);
    config_ = config;
    scoring_.start();
    movementAccumulator_ = std::chrono::microseconds{0};
    hasGame_ = true;
    lastError_.clear();
    return true;
}

void GameplayController::update(std::chrono::microseconds step, Screen screen) {
    if (!hasGame_ || screen != Screen::Gameplay || step.count() <= 0 ||
        engine_.isGameOver()) {
        return;
    }

    // The countdown follows real play time, not the number of moves.
    modes_.update(step);
    if (modes_.isTimeUp()) {
        engine_.endGame(GameOverReason::TIME_UP);
        return;
    }

    movementAccumulator_ += step;
    const std::chrono::microseconds interval = modes_.tickInterval();

    while (movementAccumulator_ >= interval && !engine_.isGameOver()) {
        scoring_.tick(engine_, config_.mode);
        movementAccumulator_ -= interval;
    }
}

void GameplayController::changeDirection(Direction direction) {
    (void)requestDirection(direction);
}

bool GameplayController::requestDirection(Direction direction) {
    return hasGame_ && engine_.tryQueueDirection(direction);
}

bool GameplayController::hasGame() const noexcept {
    return hasGame_;
}

bool GameplayController::isGameOver() const noexcept {
    return hasGame_ && engine_.isGameOver();
}

GameOverReason GameplayController::gameOverReason() const noexcept {
    return hasGame_ ? engine_.gameOverReason() : GameOverReason::None;
}

const GameEngine& GameplayController::engine() const noexcept {
    return engine_;
}

const SessionConfig& GameplayController::config() const noexcept {
    return config_;
}

std::chrono::milliseconds GameplayController::tickInterval() const noexcept {
    return modes_.tickInterval();
}

std::optional<std::chrono::milliseconds> GameplayController::remainingTime() const noexcept {
    if (!hasGame_) {
        return std::nullopt;
    }
    return modes_.remainingTime();
}

int GameplayController::score() const noexcept {
    return scoring_.currentScore();
}

int GameplayController::highScore() const noexcept {
    return scoring_.highScore();
}

const std::string& GameplayController::lastError() const noexcept {
    return lastError_;
}

std::string formatCountdown(std::chrono::milliseconds remaining) {
    if (remaining.count() < 0) {
        remaining = std::chrono::milliseconds{0};
    }
    const long long seconds = (remaining.count() + 999) / 1000;
    const long long minutes = seconds / 60;
    const long long rest = seconds % 60;
    return std::to_string(minutes) + (rest < 10 ? ":0" : ":") + std::to_string(rest);
}

std::string_view describeGameOver(GameOverReason reason) noexcept {
    switch (reason) {
    case GameOverReason::WALL:
        return "You hit the wall";
    case GameOverReason::SELF:
        return "You ran into yourself";
    case GameOverReason::OBSTACLE:
        return "You hit an obstacle";
    case GameOverReason::TIME_UP:
        return "Time's up";
    case GameOverReason::BOARD_FULL:
        return "Board full - you win";
    case GameOverReason::None:
        break;
    }
    return "Game over";
}

} // namespace snake

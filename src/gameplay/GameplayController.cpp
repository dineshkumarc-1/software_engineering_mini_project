#include "snakextreme/GameplayController.hpp"

#include "snakextreme/GameConfig.hpp"

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>

namespace snake {

namespace {

// Applies power-up effects to the running game (C8 -> C6, C9): the movement
// interval scale (ADR-002: difficulty interval x boost factor), the food
// multiplier and the snake's length.
class EffectTarget final : public IPowerUpTarget {
public:
    EffectTarget(GameEngine& engine, GameSession& scoring, TickScale& scale) noexcept
        : engine_(&engine), scoring_(&scoring), scale_(&scale) {}

    void setTickScale(TickScale scale) override {
        *scale_ = scale;
    }

    void setScoreMultiplier(int multiplier) override {
        scoring_->setMultiplier(multiplier);
    }

    void shrink(std::size_t segments, std::size_t minimumLength) override {
        engine_->shrink(segments, minimumLength);
    }

private:
    GameEngine* engine_;
    GameSession* scoring_;
    TickScale* scale_;
};

// The power-up generator gets its own stream, not the ModeManager's.
std::uint32_t powerUpSeed(std::uint32_t seed) noexcept {
    return seed ^ 0x9E3779B9u;
}

} // namespace

bool GameplayController::start(SessionConfig config) {
    return begin(config, GameEngine{}, ModeManager{config}, PowerUpManager{});
}

bool GameplayController::start(SessionConfig config,
                               GameEngine initialEngine,
                               std::uint32_t seed,
                               PowerUpSchedule powerUps) {
    return begin(config, std::move(initialEngine), ModeManager{config, seed},
                 PowerUpManager{powerUpSeed(seed), powerUps});
}

bool GameplayController::begin(SessionConfig config,
                               GameEngine engine,
                               ModeManager modes,
                               PowerUpManager powerUps) {
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
    powerUps_ = std::move(powerUps);
    config_ = config;
    scoring_.start(); // score 0 and no multiplier; the high score stays
    tickScale_ = TickScale{};
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
    EffectTarget target{engine_, scoring_, tickScale_};

    // Per move (SAD 4.1): move, then check the power-up under the head. The
    // interval is read before every move because a Speed boost changes it.
    for (std::chrono::microseconds interval = currentInterval();
         movementAccumulator_ >= interval && !engine_.isGameOver();
         interval = currentInterval()) {
        scoring_.tick(engine_, config_.mode);
        movementAccumulator_ -= interval;
        if (!engine_.isGameOver() &&
            powerUps_.onHeadMoved(engine_.snake().front(), target)) {
            syncFoodExclusions();
        }
    }

    // Then the power-up timers: effect, lifetime and spawn schedule.
    if (!engine_.isGameOver()) {
        powerUps_.update(step, engine_, target);
        syncFoodExclusions();
    }
}

void GameplayController::changeDirection(Direction direction) {
    if (hasGame_) {
        engine_.changeDirection(direction);
    }
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
    return std::chrono::duration_cast<std::chrono::milliseconds>(currentInterval());
}

std::chrono::microseconds GameplayController::currentInterval() const noexcept {
    return scaled(modes_.tickInterval(), tickScale_);
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

std::optional<PowerUp> GameplayController::powerUpOnGrid() const noexcept {
    if (!hasGame_) {
        return std::nullopt;
    }
    return powerUps_.onGrid();
}

std::optional<ActiveEffect> GameplayController::activeEffect() const noexcept {
    if (!hasGame_) {
        return std::nullopt;
    }
    return powerUps_.active();
}

void GameplayController::placePowerUp(PowerUpType type, GridPosition cell) {
    if (!hasGame_) {
        throw std::logic_error("No game to place a power-up in");
    }

    const bool inside = cell.x >= 0 && cell.x < config::GridColumns && cell.y >= 0 &&
                        cell.y < config::GridRows;
    const auto& body = engine_.snake();
    const auto& obstacles = engine_.obstacles();
    const bool taken = std::find(body.begin(), body.end(), cell) != body.end() ||
                       std::find(obstacles.begin(), obstacles.end(), cell) != obstacles.end() ||
                       (engine_.hasFood() && engine_.food() == cell);
    if (!inside || taken) {
        throw std::invalid_argument("A power-up needs a free cell inside the grid");
    }

    powerUps_.place(type, cell);
    syncFoodExclusions();
}

void GameplayController::syncFoodExclusions() {
    // New food stays off the power-up's cell (FR-012: the power-up keeps an
    // unoccupied cell).
    const std::optional<PowerUp> item = powerUps_.onGrid();
    const std::vector<GridPosition>& excluded = engine_.foodExclusions();
    const bool inSync = item ? excluded.size() == 1 && excluded.front() == item->cell
                             : excluded.empty();
    if (!inSync) {
        engine_.setFoodExclusions(item ? std::vector<GridPosition>{item->cell}
                                       : std::vector<GridPosition>{});
    }
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

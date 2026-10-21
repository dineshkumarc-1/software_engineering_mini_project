#include "snakextreme/ModeManager.hpp"

#include <algorithm>

namespace snake {

ModeManager::ModeManager(SessionConfig config)
    : ModeManager(config, std::random_device{}()) {}

ModeManager::ModeManager(SessionConfig config, std::uint32_t seed)
    : config_(config),
      strategy_(makeModeStrategy(config.mode)),
      random_(seed) {}

void ModeManager::startSession(const std::deque<GridPosition>& snake,
                               std::optional<GridPosition> food) {
    elapsed_ = std::chrono::microseconds{0};
    obstacles_ = strategy_->placeObstacles(random_, snake, food);
}

void ModeManager::update(std::chrono::microseconds elapsed) noexcept {
    const auto limit = strategy_->timeLimit();
    if (!limit || elapsed.count() <= 0) {
        return;
    }
    // Clamp at the limit so a long stall cannot overflow or go negative.
    const std::chrono::microseconds limitUs = *limit;
    elapsed_ = std::min(limitUs, elapsed_ + std::min(elapsed, limitUs));
}

const SessionConfig& ModeManager::config() const noexcept {
    return config_;
}

GameMode ModeManager::mode() const noexcept {
    return config_.mode;
}

Difficulty ModeManager::difficulty() const noexcept {
    return config_.difficulty;
}

const IModeStrategy& ModeManager::strategy() const noexcept {
    return *strategy_;
}

std::chrono::milliseconds ModeManager::tickInterval() const noexcept {
    return snake::tickInterval(config_.difficulty);
}

int ModeManager::foodPoints() const noexcept {
    return strategy_->foodPoints();
}

bool ModeManager::hasTimeLimit() const noexcept {
    return strategy_->timeLimit().has_value();
}

std::optional<std::chrono::milliseconds> ModeManager::remainingTime() const noexcept {
    const auto limit = strategy_->timeLimit();
    if (!limit) {
        return std::nullopt;
    }
    const std::chrono::microseconds left =
        std::chrono::microseconds{*limit} - elapsed_;
    return std::chrono::ceil<std::chrono::milliseconds>(left);
}

bool ModeManager::isTimeUp() const noexcept {
    const auto limit = strategy_->timeLimit();
    return limit && elapsed_ >= *limit;
}

const Obstacles& ModeManager::obstacles() const noexcept {
    return obstacles_;
}

bool ModeManager::isObstacle(GridPosition cell) const noexcept {
    return std::find(obstacles_.begin(), obstacles_.end(), cell) != obstacles_.end();
}

bool ModeManager::isFatal(GridPosition cell) const noexcept {
    return strategy_->isFatal(cell, obstacles_);
}

} // namespace snake

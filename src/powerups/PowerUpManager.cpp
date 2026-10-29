#include "snakextreme/PowerUpManager.hpp"

#include "snakextreme/GameConfig.hpp"

#include <stdexcept>
#include <vector>

namespace snake {

namespace {

void validate(const PowerUpSchedule& schedule) {
    if (schedule.minGap.count() <= 0 || schedule.maxGap < schedule.minGap) {
        throw std::invalid_argument("Power-up gap must be positive with min <= max");
    }
    if (schedule.lifetime.count() <= 0) {
        throw std::invalid_argument("Power-up lifetime must be positive");
    }
}

bool insideGrid(GridPosition cell) noexcept {
    return cell.x >= 0 && cell.x < config::GridColumns && cell.y >= 0 &&
           cell.y < config::GridRows;
}

std::size_t cellIndex(GridPosition cell) noexcept {
    return static_cast<std::size_t>(cell.y) * static_cast<std::size_t>(config::GridColumns) +
           static_cast<std::size_t>(cell.x);
}

// Cells a power-up may appear on: not snake, food or obstacle.
std::vector<GridPosition> freeCells(const GameEngine& board) {
    std::vector<bool> taken(static_cast<std::size_t>(config::GridColumns) *
                                static_cast<std::size_t>(config::GridRows),
                            false);
    const auto mark = [&taken](GridPosition cell) {
        if (insideGrid(cell)) {
            taken[cellIndex(cell)] = true;
        }
    };
    for (const GridPosition& segment : board.snake()) {
        mark(segment);
    }
    for (const GridPosition& obstacle : board.obstacles()) {
        mark(obstacle);
    }
    if (board.hasFood()) {
        mark(board.food());
    }

    std::vector<GridPosition> cells;
    for (int y = 0; y < config::GridRows; ++y) {
        for (int x = 0; x < config::GridColumns; ++x) {
            const GridPosition cell{x, y};
            if (!taken[cellIndex(cell)]) {
                cells.push_back(cell);
            }
        }
    }
    return cells;
}

} // namespace

PowerUpManager::PowerUpManager(PowerUpSchedule schedule)
    : PowerUpManager(std::random_device{}(), schedule) {}

PowerUpManager::PowerUpManager(std::uint32_t seed, PowerUpSchedule schedule)
    : schedule_(schedule), random_(seed) {
    validate(schedule_);
    // The first power-up is due 8-15 s after the game starts.
    untilSpawn_ = nextGap();
}

void PowerUpManager::update(std::chrono::microseconds elapsed,
                            const GameEngine& board,
                            IPowerUpTarget& target) {
    if (elapsed.count() <= 0) {
        return;
    }

    // The active effect runs out (FR-013, FR-014).
    if (effect_ != nullptr) {
        effectLeft_ -= elapsed;
        if (effectLeft_.count() <= 0) {
            const IPowerUpEffect* ending = effect_;
            effect_ = nullptr;
            effectLeft_ = std::chrono::microseconds{0};
            ending->revert(target);
        }
    }

    // An uncollected power-up disappears after its lifetime.
    if (item_) {
        item_->remaining -= elapsed;
        if (item_->remaining.count() <= 0) {
            item_.reset();
        }
    }

    if (!schedule_.randomSpawns) {
        return;
    }

    // FR-012: the next power-up is due 8-15 s after the previous one was due.
    // Counting from the due time, not from the update that noticed it, keeps
    // the schedule exact whatever the update step.
    untilSpawn_ -= elapsed;
    while (untilSpawn_.count() <= 0) {
        spawn(board, -untilSpawn_);
        untilSpawn_ += nextGap();
    }
}

std::optional<PowerUpType> PowerUpManager::onHeadMoved(GridPosition head,
                                                       IPowerUpTarget& target) {
    if (!item_ || item_->cell != head) {
        return std::nullopt;
    }

    const PowerUpType type = item_->type;
    item_.reset();

    // SRS 6.5: only one effect at a time; the current one ends first.
    if (effect_ != nullptr) {
        const IPowerUpEffect* previous = effect_;
        effect_ = nullptr;
        effectLeft_ = std::chrono::microseconds{0};
        previous->revert(target);
    }

    const IPowerUpEffect& effect = effectFor(type);
    effect.apply(target);
    if (effect.duration().count() > 0) {
        effect_ = &effect;
        effectLeft_ = effect.duration();
    }
    return type;
}

std::optional<ActiveEffect> PowerUpManager::active() const noexcept {
    if (effect_ == nullptr) {
        return std::nullopt;
    }
    return ActiveEffect{effect_->type(), effectLeft_};
}

std::optional<PowerUp> PowerUpManager::onGrid() const noexcept {
    return item_;
}

std::size_t PowerUpManager::spawnCount() const noexcept {
    return spawnCount_;
}

const PowerUpSchedule& PowerUpManager::schedule() const noexcept {
    return schedule_;
}

void PowerUpManager::place(PowerUpType type, GridPosition cell) {
    item_ = PowerUp{type, cell, schedule_.lifetime};
}

std::chrono::microseconds PowerUpManager::nextGap() {
    std::uniform_int_distribution<std::chrono::milliseconds::rep> gapMs(
        schedule_.minGap.count(), schedule_.maxGap.count());
    return std::chrono::milliseconds{gapMs(random_)};
}

void PowerUpManager::spawn(const GameEngine& board, std::chrono::microseconds late) {
    // At most one power-up: one still on the grid is moved to the new cell
    // (SRS Addendum v1.1: an uncollected item is relocated).
    item_.reset();

    const std::chrono::microseconds lifetime = schedule_.lifetime;
    if (late >= lifetime) {
        // It would already have disappeared again (only with huge steps).
        return;
    }

    const std::vector<GridPosition> cells = freeCells(board);
    if (cells.empty()) {
        return;
    }

    std::uniform_int_distribution<std::size_t> pickType(0, AllPowerUpTypes.size() - 1);
    std::uniform_int_distribution<std::size_t> pickCell(0, cells.size() - 1);
    const PowerUpType type = AllPowerUpTypes[pickType(random_)];
    const GridPosition cell = cells[pickCell(random_)];

    // Its lifetime counts from when it was due.
    item_ = PowerUp{type, cell, lifetime - late};
    ++spawnCount_;
}

} // namespace snake

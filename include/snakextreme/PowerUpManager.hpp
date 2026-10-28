#pragma once

// C8 PowerUpManager (SAD 3.4, 4.3.4; issue #14): the spawn schedule, pick-up
// and the single active effect (FR-012..015, SRS 6.5). SFML-free.
//
// The SAD's IPowerUpManager has update(Duration) and onHeadMoved(Cell). Here
// both also receive what they act on (the board to place power-ups on and
// the target that effects change), so the manager keeps no references and
// the game that owns it can be moved and replaced freely.

#include "snakextreme/GameEngine.hpp"
#include "snakextreme/PowerUp.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>

namespace snake {

class PowerUpManager {
public:
    // Random seed taken from std::random_device.
    explicit PowerUpManager(PowerUpSchedule schedule = {});
    // Fixed seed for deterministic tests (ADR-005). Assert invariants, not
    // exact cells: random distributions differ between standard libraries.
    explicit PowerUpManager(std::uint32_t seed, PowerUpSchedule schedule = {});

    // Advances the power-up clocks by `elapsed` of play. Call only while the
    // game runs, so a pause freezes them (FR-024).
    //  - The active effect counts down and is reverted on `target` when its
    //    time is up (FR-013: 5 s, FR-014: 10 s).
    //  - A power-up that is not collected disappears after its lifetime (5 s).
    //  - When the next spawn is due, a power-up of a random type appears on a
    //    random free cell of `board` (not snake, food or obstacle) and the one
    //    after it is scheduled 8-15 s later (FR-012). A power-up still on the
    //    grid at that moment is moved to the new cell, so there is never more
    //    than one. If no cell is free, that spawn is skipped.
    void update(std::chrono::microseconds elapsed, const GameEngine& board,
                IPowerUpTarget& target);

    // Call after every move with the new head. If the head is on the
    // power-up, it is collected: the active effect (if any) is reverted
    // first, then the new one is applied (SRS 6.5: only one effect at a time;
    // a new power-up replaces the current effect immediately). Shrink acts at
    // once and leaves no effect running. Returns the collected type.
    std::optional<PowerUpType> onHeadMoved(GridPosition head, IPowerUpTarget& target);

    std::optional<ActiveEffect> active() const noexcept;
    std::optional<PowerUp> onGrid() const noexcept;

    // Power-ups that have appeared on their own so far (not counting place()).
    std::size_t spawnCount() const noexcept;
    const PowerUpSchedule& schedule() const noexcept;

    // Puts a power-up on `cell` now with a full lifetime, in place of any
    // power-up already on the grid. The spawn schedule is not changed. For
    // tests and demos: the caller makes sure the cell is free.
    void place(PowerUpType type, GridPosition cell);

private:
    std::chrono::microseconds nextGap();
    void spawn(const GameEngine& board, std::chrono::microseconds late);

    PowerUpSchedule schedule_;
    std::mt19937 random_;
    std::chrono::microseconds untilSpawn_{0};
    std::optional<PowerUp> item_;
    const IPowerUpEffect* effect_{nullptr};
    std::chrono::microseconds effectLeft_{0};
    std::size_t spawnCount_{0};
};

} // namespace snake

#pragma once

// One game as the desktop app runs it: the engine (C6), the selected mode and
// difficulty (C7 ModeManager), the power-ups (C8 PowerUpManager) and the
// score (GameSession). AppManager owns one and forwards keys and fixed-step
// updates to it. It is SFML-free, so the integration tests drive exactly the
// code the game runs.

#include "snakextreme/AppState.hpp"
#include "snakextreme/GameEngine.hpp"
#include "snakextreme/GameSession.hpp"
#include "snakextreme/ModeManager.hpp"
#include "snakextreme/PowerUp.hpp"
#include "snakextreme/PowerUpManager.hpp"

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace snake {

class GameplayController {
public:
    // Starts a game with `config`: a fresh engine, a new ModeManager session
    // whose obstacles are applied to the engine, the current score reset (the
    // session high score is kept) and the movement accumulator cleared.
    // Returns false if the session cannot start; the previous game is then
    // left untouched and lastError() says why.
    bool start(SessionConfig config);

    // The same, from a given starting engine and with a fixed seed (tests).
    // `powerUps` sets when power-ups appear; the default is the SRS schedule.
    bool start(SessionConfig config, GameEngine initialEngine, std::uint32_t seed,
               PowerUpSchedule powerUps = {});

    // One fixed-step update of `step`. Only the Gameplay screen advances the
    // game, so pausing freezes movement, the Timed countdown and the
    // power-up timers. The countdown advances once per update; the snake
    // moves once per difficulty interval (Easy 200 ms, Medium 120 ms, Hard
    // 70 ms; x0.6 during a Speed boost) and after every move collects a
    // power-up under its head; then the power-up timers advance by `step`.
    void update(std::chrono::microseconds step, Screen screen);

    // Queues a turn (WASD / arrow keys). The engine keeps its reversal rule
    // and two-turn queue.
    void changeDirection(Direction direction);

    bool hasGame() const noexcept;
    bool isGameOver() const noexcept;
    GameOverReason gameOverReason() const noexcept;
    const GameEngine& engine() const noexcept;
    const SessionConfig& config() const noexcept;
    // Movement interval now: the difficulty's interval, x0.6 during a Speed
    // boost (FR-013), e.g. Medium 120 ms -> 72 ms.
    std::chrono::milliseconds tickInterval() const noexcept;
    // Time left in Timed mode; empty in the other modes or before a game.
    std::optional<std::chrono::milliseconds> remainingTime() const noexcept;
    int score() const noexcept;
    int highScore() const noexcept;
    const std::string& lastError() const noexcept;

    // Power-ups (FR-012..015): the one waiting on the grid and the timed
    // effect on the snake, if any.
    std::optional<PowerUp> powerUpOnGrid() const noexcept;
    std::optional<ActiveEffect> activeEffect() const noexcept;
    // Puts a power-up on `cell` now, in place of any other (tests, demos).
    // Throws std::logic_error without a game and std::invalid_argument when
    // the cell is outside the grid or holds the snake, food or an obstacle.
    void placePowerUp(PowerUpType type, GridPosition cell);

private:
    bool begin(SessionConfig config, GameEngine engine, ModeManager modes,
               PowerUpManager powerUps);
    std::chrono::microseconds currentInterval() const noexcept;
    void syncFoodExclusions();

    GameEngine engine_;
    ModeManager modes_;
    PowerUpManager powerUps_;
    TickScale tickScale_{};
    GameSession scoring_;
    SessionConfig config_;
    std::chrono::microseconds movementAccumulator_{0};
    bool hasGame_{false};
    std::string lastError_;
};

// Countdown as m:ss, rounded up to whole seconds, so it shows 1:00 at the
// start and 0:00 only once the time is up.
std::string formatCountdown(std::chrono::milliseconds remaining);

// Short reason shown when a game stops, e.g. "You hit an obstacle".
std::string_view describeGameOver(GameOverReason reason) noexcept;

} // namespace snake

// Unit tests for the C8 power-ups (issue #14, FR-012..015, SRS 6.5): the
// effects, the PowerUpManager schedule, pick-up and one-effect rule, and the
// engine and score hooks they use. STP TC-PWR-01..05 at unit level, with a
// fake clock (explicit update steps) and seeded random numbers (ADR-005).
// Random cells and types differ between standard libraries, so the tests
// check invariants, never exact cells.

#include "snakextreme/GameConfig.hpp"
#include "snakextreme/GameEngine.hpp"
#include "snakextreme/GameMode.hpp"
#include "snakextreme/PowerUp.hpp"
#include "snakextreme/PowerUpManager.hpp"
#include "snakextreme/ScoreService.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std::chrono_literals;

using snake::ActiveEffect;
using snake::Direction;
using snake::GameEngine;
using snake::GameMode;
using snake::GameOverReason;
using snake::GridPosition;
using snake::IPowerUpTarget;
using snake::PowerUp;
using snake::PowerUpManager;
using snake::PowerUpSchedule;
using snake::PowerUpType;
using snake::ScoreService;
using snake::TickScale;
using Micros = std::chrono::microseconds;

namespace {

// Records what the effects ask for.
class FakeTarget final : public IPowerUpTarget {
public:
    void setTickScale(TickScale value) override {
        scale = value;
        ++scaleChanges;
    }

    void setScoreMultiplier(int value) override {
        multiplier = value;
        ++multiplierChanges;
    }

    void shrink(std::size_t segments, std::size_t minimumLength) override {
        shrinks.emplace_back(segments, minimumLength);
    }

    TickScale scale{};
    int multiplier{1};
    int scaleChanges{0};
    int multiplierChanges{0};
    std::vector<std::pair<std::size_t, std::size_t>> shrinks;
};

// Passes the effects on to a real engine and score service.
class GameTarget final : public IPowerUpTarget {
public:
    GameTarget(GameEngine& engine, ScoreService& score) : engine_(&engine), score_(&score) {}

    void setTickScale(TickScale value) override {
        scale = value;
    }

    void setScoreMultiplier(int value) override {
        score_->setMultiplier(value);
    }

    void shrink(std::size_t segments, std::size_t minimumLength) override {
        engine_->shrink(segments, minimumLength);
    }

    TickScale scale{};

private:
    GameEngine* engine_;
    ScoreService* score_;
};

PowerUpSchedule placedOnly() {
    PowerUpSchedule schedule;
    schedule.randomSpawns = false;
    return schedule;
}

bool contains(const std::vector<GridPosition>& cells, GridPosition cell) {
    return std::find(cells.begin(), cells.end(), cell) != cells.end();
}

bool onSnake(const GameEngine& board, GridPosition cell) {
    const auto& body = board.snake();
    return std::find(body.begin(), body.end(), cell) != body.end();
}

bool insideGrid(GridPosition cell) {
    return cell.x >= 0 && cell.x < snake::config::GridColumns && cell.y >= 0 &&
           cell.y < snake::config::GridRows;
}

// A snake of `length` on row 10 with its head at (10, 10), heading right.
std::deque<GridPosition> rowSnake(std::size_t length) {
    std::deque<GridPosition> body;
    for (std::size_t i = 0; i < length; ++i) {
        body.push_back({10 - static_cast<int>(i), 10});
    }
    return body;
}

// A snake covering every cell except `freeCells`, head at (29, 19).
std::deque<GridPosition> snakeCoveringAllBut(const std::vector<GridPosition>& freeCells) {
    std::deque<GridPosition> body{{29, 19}};
    for (int y = 0; y < snake::config::GridRows; ++y) {
        for (int x = 0; x < snake::config::GridColumns; ++x) {
            const GridPosition cell{x, y};
            if (cell != GridPosition{29, 19} && !contains(freeCells, cell)) {
                body.push_back(cell);
            }
        }
    }
    return body;
}

// Advances `manager` by `total` in updates of `step`.
void advance(PowerUpManager& manager, Micros total, Micros step, const GameEngine& board,
             IPowerUpTarget& target) {
    for (Micros done{0}; done < total; done += step) {
        manager.update(step, board, target);
    }
}

// When the power-up on the grid was due, `now` being the current time.
Micros dueTime(const PowerUp& item, Micros now, const PowerUpSchedule& schedule) {
    return now - (Micros{schedule.lifetime} - item.remaining);
}

} // namespace

// ---------------------------------------------------------------------------
// Effects (Strategy, ADR-003)
// ---------------------------------------------------------------------------

TEST_CASE("Speed scales the movement interval by 0.6 for 5 s", "[powerups][effects][TC-PWR-02]") {
    const snake::IPowerUpEffect& speed = snake::effectFor(PowerUpType::Speed);
    REQUIRE(speed.type() == PowerUpType::Speed);
    REQUIRE(speed.duration() == 5s);

    FakeTarget target;
    speed.apply(target);
    REQUIRE(target.scale == TickScale{3, 5});
    // Easy, Medium and Hard become exactly 120, 72 and 42 ms.
    REQUIRE(snake::scaled(200ms, target.scale) == 120ms);
    REQUIRE(snake::scaled(120ms, target.scale) == 72ms);
    REQUIRE(snake::scaled(70ms, target.scale) == 42ms);

    speed.revert(target);
    REQUIRE(target.scale == TickScale{});
    REQUIRE(snake::scaled(120ms, target.scale) == 120ms);
}

TEST_CASE("Multiplier doubles food points for 10 s", "[powerups][effects][TC-PWR-03]") {
    const snake::IPowerUpEffect& multiplier = snake::effectFor(PowerUpType::Multiplier);
    REQUIRE(multiplier.type() == PowerUpType::Multiplier);
    REQUIRE(multiplier.duration() == 10s);

    FakeTarget target;
    multiplier.apply(target);
    REQUIRE(target.multiplier == 2);
    multiplier.revert(target);
    REQUIRE(target.multiplier == 1);
}

TEST_CASE("Shrink removes one segment at once with a minimum of 3", "[powerups][effects][TC-PWR-04]") {
    const snake::IPowerUpEffect& shrink = snake::effectFor(PowerUpType::Shrink);
    REQUIRE(shrink.type() == PowerUpType::Shrink);
    REQUIRE(shrink.duration() == 0s);

    FakeTarget target;
    shrink.apply(target);
    REQUIRE(target.shrinks == std::vector<std::pair<std::size_t, std::size_t>>{{1, 3}});

    // Nothing to give back.
    shrink.revert(target);
    REQUIRE(target.shrinks.size() == 1);
    REQUIRE(target.scaleChanges == 0);
    REQUIRE(target.multiplierChanges == 0);
}

TEST_CASE("Every power-up type has a name and its own effect", "[powerups][effects]") {
    REQUIRE(snake::AllPowerUpTypes.size() == 3);
    REQUIRE(snake::toString(PowerUpType::Speed) == "Speed");
    REQUIRE(snake::toString(PowerUpType::Multiplier) == "Multiplier");
    REQUIRE(snake::toString(PowerUpType::Shrink) == "Shrink");
    for (const PowerUpType type : snake::AllPowerUpTypes) {
        REQUIRE(snake::effectFor(type).type() == type);
    }
}

TEST_CASE("Effect time is shown in tenths of a second, rounded up", "[powerups][hud]") {
    REQUIRE(snake::formatEffectTime(5s) == "5.0s");
    REQUIRE(snake::formatEffectTime(Micros{4983334}) == "5.0s");
    REQUIRE(snake::formatEffectTime(4900ms) == "4.9s");
    REQUIRE(snake::formatEffectTime(Micros{4899999}) == "4.9s");
    REQUIRE(snake::formatEffectTime(10s) == "10.0s");
    REQUIRE(snake::formatEffectTime(Micros{1}) == "0.1s");
    REQUIRE(snake::formatEffectTime(0s) == "0.0s");
    REQUIRE(snake::formatEffectTime(Micros{-5}) == "0.0s");

    REQUIRE(snake::describeEffect(ActiveEffect{PowerUpType::Speed, 4900ms}) ==
            "Speed boost 4.9s");
    REQUIRE(snake::describeEffect(ActiveEffect{PowerUpType::Multiplier, 10s}) ==
            "Score x2 10.0s");
}

// ---------------------------------------------------------------------------
// Score and engine hooks (C9, C6)
// ---------------------------------------------------------------------------

TEST_CASE("The score multiplier applies to every mode and resets with a new game",
          "[powerups][scoring][TC-PWR-03]") {
    ScoreService score;
    REQUIRE(score.multiplier() == 1);

    score.setMultiplier(2);
    score.foodEaten(GameMode::Classic);
    REQUIRE(score.currentScore() == 20);
    score.foodEaten(GameMode::Obstacle);
    REQUIRE(score.currentScore() == 50);
    score.foodEaten(GameMode::Timed);
    REQUIRE(score.currentScore() == 70);

    score.setMultiplier(1);
    score.foodEaten(GameMode::Classic);
    REQUIRE(score.currentScore() == 80);
    REQUIRE(score.highScore() == 80);

    // Below 1 counts as 1: a score never goes down (TC-SCORE-01).
    score.setMultiplier(0);
    REQUIRE(score.multiplier() == 1);
    score.setMultiplier(-3);
    REQUIRE(score.multiplier() == 1);

    score.setMultiplier(2);
    score.reset();
    REQUIRE(score.currentScore() == 0);
    REQUIRE(score.multiplier() == 1);
    REQUIRE(score.highScore() == 80);
    score.foodEaten(GameMode::Classic);
    REQUIRE(score.currentScore() == 10);
}

TEST_CASE("The engine shrinks the tail but keeps the minimum length", "[powerups][engine][TC-PWR-04]") {
    GameEngine engine(rowSnake(6), Direction::Right, {20, 5});

    REQUIRE(engine.shrink(1, 3) == 1);
    REQUIRE(engine.snake().size() == 5);
    REQUIRE(engine.snake().back() == GridPosition{6, 10});
    REQUIRE(engine.snake().front() == GridPosition{10, 10});

    REQUIRE(engine.shrink(5, 3) == 2);
    REQUIRE(engine.snake().size() == 3);
    REQUIRE(engine.shrink(1, 3) == 0);
    REQUIRE(engine.snake().size() == 3);

    // The head always stays, even with a minimum of 0.
    REQUIRE(engine.shrink(10, 0) == 2);
    REQUIRE(engine.snake().size() == 1);
    REQUIRE(engine.snake().front() == GridPosition{10, 10});

    // The snake still moves normally afterwards.
    engine.tick();
    REQUIRE(engine.snake().front() == GridPosition{11, 10});
    REQUIRE(engine.snake().size() == 1);
}

TEST_CASE("A finished game cannot shrink", "[powerups][engine]") {
    GameEngine engine(rowSnake(6), Direction::Right, {20, 5});
    engine.endGame(GameOverReason::TIME_UP);
    REQUIRE(engine.shrink(1, 3) == 0);
    REQUIRE(engine.snake().size() == 6);
}

TEST_CASE("New food avoids excluded cells while another cell is free", "[powerups][engine][FR-012]") {
    std::vector<std::vector<GridPosition>> offered;
    GameEngine engine({{2, 10}, {1, 10}, {0, 10}}, Direction::Right, {3, 10},
                      [&offered](const std::vector<GridPosition>& cells) {
                          offered.push_back(cells);
                          return cells.front();
                      });
    REQUIRE(engine.foodExclusions().empty());

    engine.setFoodExclusions({{0, 0}});
    REQUIRE(engine.foodExclusions() == std::vector<GridPosition>{{0, 0}});

    engine.tick(); // eats the food at (3, 10)
    REQUIRE(offered.size() == 1);
    REQUIRE_FALSE(contains(offered[0], {0, 0}));
    // 600 cells - 4 snake segments - the excluded cell.
    REQUIRE(offered[0].size() == 595);
    REQUIRE(engine.food() == GridPosition{1, 0});

    // Cleared again: every free cell is offered.
    engine.setFoodExclusions({});
    REQUIRE(engine.foodExclusions().empty());
}

TEST_CASE("An excluded cell still gets the food when it is the only free cell",
          "[powerups][engine][FR-012]") {
    // Free: the food at (28, 19), right of the head's path, and (0, 0).
    GameEngine engine(snakeCoveringAllBut({{28, 19}, {0, 0}}), Direction::Left, {28, 19});
    engine.setFoodExclusions({{0, 0}});

    engine.tick(); // the head eats at (28, 19)
    REQUIRE_FALSE(engine.isGameOver());
    REQUIRE(engine.hasFood());
    REQUIRE(engine.food() == GridPosition{0, 0});
}

// ---------------------------------------------------------------------------
// PowerUpManager: schedule (FR-012)
// ---------------------------------------------------------------------------

TEST_CASE("The first power-up appears 8 to 15 s after the start", "[powerups][schedule][TC-PWR-01]") {
    const GameEngine board(rowSnake(3), Direction::Right, {20, 5});

    for (std::uint32_t seed = 1; seed <= 25; ++seed) {
        CAPTURE(seed);
        PowerUpManager manager(seed);
        FakeTarget target;
        REQUIRE_FALSE(manager.onGrid());

        Micros now{0};
        while (!manager.onGrid() && now < 20s) {
            manager.update(1ms, board, target);
            now += 1ms;
        }
        REQUIRE(manager.onGrid());
        REQUIRE(manager.spawnCount() == 1);
        const Micros due = dueTime(*manager.onGrid(), now, manager.schedule());
        REQUIRE(due >= 8s);
        REQUIRE(due <= 15s);
    }
}

TEST_CASE("Over 10 simulated minutes power-ups appear 8 to 15 s apart on free cells, one at a time",
          "[powerups][schedule][TC-PWR-01]") {
    const std::uint32_t seed = GENERATE(1u, 2u, 3u);
    // 1 ms steps and the app's 60 Hz step.
    const Micros step = GENERATE(Micros{1000}, Micros{16666});
    CAPTURE(seed, step.count());

    GameEngine board(rowSnake(5), Direction::Right, {20, 5});
    board.setObstacles({{3, 3}, {25, 4}, {7, 15}, {18, 12}, {22, 17}, {12, 2}, {28, 9}, {1, 18}});
    PowerUpManager manager(seed);
    FakeTarget target;
    const PowerUpSchedule& schedule = manager.schedule();

    std::vector<Micros> spawnTimes;
    std::size_t badCells = 0;
    std::size_t earlyExpiries = 0;
    std::size_t lateExpiries = 0;
    std::size_t seen = 0;

    for (Micros now{0}; now < 10min;) {
        manager.update(step, board, target);
        now += step;

        const std::optional<PowerUp> item = manager.onGrid();
        if (manager.spawnCount() != seen) {
            // A new power-up: when it was due, and where.
            REQUIRE(manager.spawnCount() == seen + 1);
            seen = manager.spawnCount();
            REQUIRE(item);
            spawnTimes.push_back(dueTime(*item, now, schedule));
            const bool free = insideGrid(item->cell) && !onSnake(board, item->cell) &&
                              !contains(board.obstacles(), item->cell) &&
                              item->cell != board.food();
            if (!free) {
                ++badCells;
            }
        } else if (!spawnTimes.empty()) {
            // Uncollected: on the grid for exactly its 5 s lifetime.
            const Micros age = now - spawnTimes.back();
            if (item && age >= 5s) {
                ++lateExpiries;
            }
            if (!item && age < 5s) {
                ++earlyExpiries;
            }
        }
    }

    REQUIRE(badCells == 0);
    REQUIRE(earlyExpiries == 0);
    REQUIRE(lateExpiries == 0);
    // 600 s with gaps of at most 15 s: at least 40 power-ups.
    REQUIRE(spawnTimes.size() >= 40);
    REQUIRE(spawnTimes.front() >= 8s);
    REQUIRE(spawnTimes.front() <= 15s);
    for (std::size_t i = 1; i < spawnTimes.size(); ++i) {
        CAPTURE(i);
        const Micros gap = spawnTimes[i] - spawnTimes[i - 1];
        REQUIRE(gap >= 8s);
        REQUIRE(gap <= 15s);
    }
}

TEST_CASE("An uncollected power-up disappears after 5 s", "[powerups][schedule][FR-012]") {
    const GameEngine board(rowSnake(3), Direction::Right, {20, 5});
    PowerUpManager manager(7u, placedOnly());
    FakeTarget target;

    manager.place(PowerUpType::Multiplier, {4, 4});
    REQUIRE(manager.onGrid()->remaining == 5s);

    advance(manager, 4999ms, 1ms, board, target);
    REQUIRE(manager.onGrid());
    REQUIRE(manager.onGrid()->remaining == 1ms);

    manager.update(1ms, board, target);
    REQUIRE_FALSE(manager.onGrid());
    // Nothing was collected, so nothing changed on the snake.
    REQUIRE_FALSE(manager.active());
    REQUIRE(target.multiplierChanges == 0);
}

TEST_CASE("A power-up still on the grid when the next one is due is moved there",
          "[powerups][schedule][FR-012]") {
    // A lifetime longer than the gap, so the first is still there at 16 s.
    PowerUpSchedule schedule;
    schedule.minGap = 8s;
    schedule.maxGap = 8s;
    schedule.lifetime = 20s;
    const GameEngine board(rowSnake(3), Direction::Right, {20, 5});
    PowerUpManager manager(3u, schedule);
    FakeTarget target;

    advance(manager, 8s, 1ms, board, target);
    REQUIRE(manager.spawnCount() == 1);
    REQUIRE(manager.onGrid()->remaining == 20s);

    advance(manager, 8s, 1ms, board, target);
    REQUIRE(manager.spawnCount() == 2);
    // Still exactly one power-up, with a fresh lifetime.
    REQUIRE(manager.onGrid());
    REQUIRE(manager.onGrid()->remaining == 20s);
}

TEST_CASE("Power-ups use only free cells", "[powerups][schedule][TC-PWR-01]") {
    // Everything is taken except the food cell and (5, 5).
    const GameEngine board(snakeCoveringAllBut({{28, 19}, {5, 5}}), Direction::Left, {28, 19});
    PowerUpManager manager(11u);
    FakeTarget target;

    std::size_t seen = 0;
    for (Micros now{0}; now < 2min; now += 16666us) {
        manager.update(16666us, board, target);
        if (manager.spawnCount() != seen) {
            seen = manager.spawnCount();
            REQUIRE(manager.onGrid()->cell == GridPosition{5, 5});
        }
    }
    REQUIRE(seen >= 8);
}

TEST_CASE("Without a free cell no power-up appears and the schedule goes on",
          "[powerups][schedule]") {
    const GameEngine board(snakeCoveringAllBut({{28, 19}}), Direction::Left, {28, 19});
    PowerUpManager manager(5u);
    FakeTarget target;

    advance(manager, 2min, 16666us, board, target);
    REQUIRE_FALSE(manager.onGrid());
    REQUIRE(manager.spawnCount() == 0);
}

TEST_CASE("With random spawns off only placed power-ups appear", "[powerups][schedule]") {
    const GameEngine board(rowSnake(3), Direction::Right, {20, 5});
    PowerUpManager manager(1u, placedOnly());
    FakeTarget target;

    advance(manager, 10min, 16666us, board, target);
    REQUIRE_FALSE(manager.onGrid());
    REQUIRE(manager.spawnCount() == 0);

    manager.place(PowerUpType::Speed, {1, 1});
    REQUIRE(manager.onGrid()->type == PowerUpType::Speed);
    REQUIRE(manager.onGrid()->cell == GridPosition{1, 1});
    REQUIRE(manager.spawnCount() == 0);
}

TEST_CASE("A zero or negative update changes nothing", "[powerups][schedule]") {
    const GameEngine board(rowSnake(3), Direction::Right, {20, 5});
    PowerUpManager manager(1u, placedOnly());
    FakeTarget target;
    manager.place(PowerUpType::Speed, {12, 10});
    REQUIRE(manager.onHeadMoved({12, 10}, target));
    manager.place(PowerUpType::Shrink, {1, 1});

    manager.update(0us, board, target);
    manager.update(-5s, board, target);
    REQUIRE(manager.active()->remaining == 5s);
    REQUIRE(manager.onGrid()->remaining == 5s);
}

TEST_CASE("Invalid schedules are rejected", "[powerups][schedule]") {
    PowerUpSchedule noGap;
    noGap.minGap = 0s;
    REQUIRE_THROWS_AS(PowerUpManager(1u, noGap), std::invalid_argument);

    PowerUpSchedule reversed;
    reversed.minGap = 10s;
    reversed.maxGap = 9s;
    REQUIRE_THROWS_AS(PowerUpManager(1u, reversed), std::invalid_argument);

    PowerUpSchedule noLifetime;
    noLifetime.lifetime = 0s;
    REQUIRE_THROWS_AS(PowerUpManager(1u, noLifetime), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// PowerUpManager: pick-up and timed effects (FR-013..015, SRS 6.5)
// ---------------------------------------------------------------------------

TEST_CASE("A power-up is collected only when the head reaches its cell", "[powerups][pickup]") {
    PowerUpManager manager(1u, placedOnly());
    FakeTarget target;
    manager.place(PowerUpType::Speed, {5, 5});

    REQUIRE_FALSE(manager.onHeadMoved({5, 6}, target));
    REQUIRE(manager.onGrid());
    REQUIRE(target.scaleChanges == 0);

    REQUIRE(manager.onHeadMoved({5, 5}, target) == PowerUpType::Speed);
    REQUIRE_FALSE(manager.onGrid());
    REQUIRE(manager.active()->type == PowerUpType::Speed);
    REQUIRE(manager.active()->remaining == 5s);
    REQUIRE(target.scale == TickScale{3, 5});

    // Nothing left to collect.
    REQUIRE_FALSE(manager.onHeadMoved({5, 5}, target));
}

TEST_CASE("TC-PWR-02: a Speed boost lasts 5.0 s, then the interval is back to normal",
          "[powerups][timing][TC-PWR-02]") {
    const GameEngine board(rowSnake(3), Direction::Right, {20, 5});
    PowerUpManager manager(1u, placedOnly());
    FakeTarget target;
    manager.place(PowerUpType::Speed, {11, 10});
    REQUIRE(manager.onHeadMoved({11, 10}, target));

    // Medium: 120 ms becomes 72 ms.
    REQUIRE(snake::scaled(120ms, target.scale) == 72ms);
    REQUIRE(snake::describeEffect(*manager.active()) == "Speed boost 5.0s");

    advance(manager, 100ms, 1ms, board, target);
    REQUIRE(snake::describeEffect(*manager.active()) == "Speed boost 4.9s");

    advance(manager, 4899ms, 1ms, board, target);
    REQUIRE(manager.active()->remaining == 1ms);
    REQUIRE(snake::scaled(120ms, target.scale) == 72ms);
    REQUIRE(snake::describeEffect(*manager.active()) == "Speed boost 0.1s");

    manager.update(1ms, board, target); // 5.000 s
    REQUIRE_FALSE(manager.active());
    REQUIRE(snake::scaled(120ms, target.scale) == 120ms);
    REQUIRE(target.scaleChanges == 2); // applied once, reverted once
}

TEST_CASE("TC-PWR-03: the x2 multiplier lasts 10 s", "[powerups][timing][TC-PWR-03]") {
    // Classic mode, 10 points per food. Food at t = 1 s, 9.9 s and 10.1 s.
    GameEngine board(rowSnake(3), Direction::Right, {20, 5});
    ScoreService score;
    GameTarget target(board, score);
    PowerUpManager manager(1u, placedOnly());

    manager.place(PowerUpType::Multiplier, {11, 10});
    REQUIRE(manager.onHeadMoved({11, 10}, target) == PowerUpType::Multiplier);

    std::vector<int> deltas;
    const auto eat = [&] {
        const int before = score.currentScore();
        score.foodEaten(GameMode::Classic);
        deltas.push_back(score.currentScore() - before);
    };

    advance(manager, 1s, 1ms, board, target);
    eat();
    advance(manager, 8900ms, 1ms, board, target);
    eat();
    advance(manager, 200ms, 1ms, board, target);
    eat();

    REQUIRE(deltas == std::vector<int>{20, 20, 10});
    REQUIRE_FALSE(manager.active());
    REQUIRE(score.multiplier() == 1);
}

TEST_CASE("TC-PWR-04: Shrink takes lengths 6, 4 and 3 to 5, 3 and 3", "[powerups][TC-PWR-04]") {
    const std::size_t length = GENERATE(6u, 4u, 3u);
    const std::size_t expected = length == 6 ? 5u : 3u;
    CAPTURE(length);

    GameEngine board(rowSnake(length), Direction::Right, {20, 5});
    ScoreService score;
    GameTarget target(board, score);
    PowerUpManager manager(1u, placedOnly());

    manager.place(PowerUpType::Shrink, {11, 10});
    board.tick(); // the head moves onto the power-up
    REQUIRE(manager.onHeadMoved(board.snake().front(), target) == PowerUpType::Shrink);

    REQUIRE(board.snake().size() == expected);
    REQUIRE(board.snake().size() >= snake::config::MinimumSnakeLength);
    // Consumed, and nothing keeps running.
    REQUIRE_FALSE(manager.onGrid());
    REQUIRE_FALSE(manager.active());
}

TEST_CASE("TC-PWR-05: a new power-up replaces the running effect at once",
          "[powerups][timing][TC-PWR-05]") {
    const GameEngine board(rowSnake(3), Direction::Right, {20, 5});
    PowerUpManager manager(1u, placedOnly());
    FakeTarget target;

    manager.place(PowerUpType::Speed, {11, 10});
    REQUIRE(manager.onHeadMoved({11, 10}, target));
    REQUIRE(target.scale == TickScale{3, 5});

    advance(manager, 2s, 1ms, board, target);
    REQUIRE(manager.active()->type == PowerUpType::Speed);
    REQUIRE(manager.active()->remaining == 3s);

    // Collecting the Multiplier ends the Speed boost at that moment.
    manager.place(PowerUpType::Multiplier, {12, 10});
    REQUIRE(manager.onHeadMoved({12, 10}, target));
    REQUIRE(target.scale == TickScale{});
    REQUIRE(target.multiplier == 2);
    REQUIRE(manager.active()->type == PowerUpType::Multiplier);
    REQUIRE(manager.active()->remaining == 10s);
    const int scaleChangesAfterReplace = target.scaleChanges;
    REQUIRE(scaleChangesAfterReplace == 2);

    // Ten more seconds: only the Multiplier ends; no late Speed revert or
    // re-apply ever happens.
    for (int ms = 0; ms < 10000; ++ms) {
        manager.update(1ms, board, target);
        REQUIRE(target.scale == TickScale{});
    }
    REQUIRE_FALSE(manager.active());
    REQUIRE(target.multiplier == 1);
    REQUIRE(target.scaleChanges == scaleChangesAfterReplace);

    // Speed again works as before.
    manager.place(PowerUpType::Speed, {13, 10});
    REQUIRE(manager.onHeadMoved({13, 10}, target));
    REQUIRE(target.scale == TickScale{3, 5});
    REQUIRE(manager.active()->type == PowerUpType::Speed);
    REQUIRE(manager.active()->remaining == 5s);
}

TEST_CASE("Shrink also ends a running effect", "[powerups][timing][SRS-6.5]") {
    GameEngine board(rowSnake(6), Direction::Right, {20, 5});
    ScoreService score;
    GameTarget target(board, score);
    PowerUpManager manager(1u, placedOnly());

    manager.place(PowerUpType::Multiplier, {11, 10});
    REQUIRE(manager.onHeadMoved({11, 10}, target));
    REQUIRE(score.multiplier() == 2);

    manager.place(PowerUpType::Shrink, {12, 10});
    REQUIRE(manager.onHeadMoved({12, 10}, target) == PowerUpType::Shrink);
    REQUIRE(score.multiplier() == 1);
    REQUIRE(board.snake().size() == 5);
    REQUIRE_FALSE(manager.active());
}

TEST_CASE("Collecting the same type again restarts its time", "[powerups][timing]") {
    const GameEngine board(rowSnake(3), Direction::Right, {20, 5});
    PowerUpManager manager(1u, placedOnly());
    FakeTarget target;

    manager.place(PowerUpType::Speed, {11, 10});
    REQUIRE(manager.onHeadMoved({11, 10}, target));
    advance(manager, 3s, 1ms, board, target);
    REQUIRE(manager.active()->remaining == 2s);

    manager.place(PowerUpType::Speed, {12, 10});
    REQUIRE(manager.onHeadMoved({12, 10}, target));
    REQUIRE(manager.active()->remaining == 5s);
    REQUIRE(target.scale == TickScale{3, 5});

    advance(manager, 4999ms, 1ms, board, target);
    REQUIRE(manager.active());
    manager.update(1ms, board, target);
    REQUIRE_FALSE(manager.active());
    REQUIRE(target.scale == TickScale{});
}

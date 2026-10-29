// Power-ups in the running game (issue #14): GameplayController, the object
// the desktop app drives, with the real engine, ModeManager and score
// session. STP TC-PWR-01..05 as system-level checks, plus pause, restart,
// game over and food placement. Long runs steer the snake around a loop so
// it never hits a wall.

#include "snakextreme/AppState.hpp"
#include "snakextreme/GameConfig.hpp"
#include "snakextreme/GameEngine.hpp"
#include "snakextreme/GameMode.hpp"
#include "snakextreme/GameplayController.hpp"
#include "snakextreme/ModeManager.hpp"
#include "snakextreme/PowerUp.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace std::chrono_literals;

using snake::Difficulty;
using snake::Direction;
using snake::GameEngine;
using snake::GameMode;
using snake::GameOverReason;
using snake::GameplayController;
using snake::GridPosition;
using snake::PowerUp;
using snake::PowerUpSchedule;
using snake::PowerUpType;
using snake::Screen;
using Micros = std::chrono::microseconds;

namespace {

// One fixed update of the app loop: sf::seconds(1.f / 60.f).asMicroseconds().
constexpr Micros AppStep{16666};

// A clockwise loop inside the board (x 2..27, y 2..17, 80 cells).
constexpr int LoopLeft = 2;
constexpr int LoopRight = 27;
constexpr int LoopTop = 2;
constexpr int LoopBottom = 17;

PowerUpSchedule placedOnly() {
    PowerUpSchedule schedule;
    schedule.randomSpawns = false;
    return schedule;
}

bool contains(const std::vector<GridPosition>& cells, GridPosition cell) {
    return std::find(cells.begin(), cells.end(), cell) != cells.end();
}

bool occupied(const GameEngine& engine, GridPosition cell) {
    const auto& body = engine.snake();
    return std::find(body.begin(), body.end(), cell) != body.end() ||
           contains(engine.obstacles(), cell) || (engine.hasFood() && engine.food() == cell);
}

// Heading for the next move on the loop.
Direction loopTurn(GridPosition head, Direction heading) {
    if (heading == Direction::Right && head.x >= LoopRight) {
        return Direction::Down;
    }
    if (heading == Direction::Down && head.y >= LoopBottom) {
        return Direction::Left;
    }
    if (heading == Direction::Left && head.x <= LoopLeft) {
        return Direction::Up;
    }
    if (heading == Direction::Up && head.y <= LoopTop) {
        return Direction::Right;
    }
    return heading;
}

GridPosition step(GridPosition cell, Direction heading) {
    switch (heading) {
    case Direction::Up:
        return {cell.x, cell.y - 1};
    case Direction::Down:
        return {cell.x, cell.y + 1};
    case Direction::Left:
        return {cell.x - 1, cell.y};
    case Direction::Right:
        return {cell.x + 1, cell.y};
    }
    return cell;
}

// Where the head is after each of the next `moves` moves around the loop:
// element k - 1 is the head after move k.
std::vector<GridPosition> loopPath(GridPosition head, Direction heading, std::size_t moves) {
    std::vector<GridPosition> path;
    for (std::size_t i = 0; i < moves; ++i) {
        heading = loopTurn(head, heading);
        head = step(head, heading);
        path.push_back(head);
    }
    return path;
}

// Queues the loop's next turn. Called before every update; every test here
// has at most one move per update.
void steer(GameplayController& game) {
    const GameEngine& engine = game.engine();
    const Direction turn = loopTurn(engine.snake().front(), engine.direction());
    if (turn != engine.direction()) {
        game.changeDirection(turn);
    }
}

// The cell the head enters on its next move around the loop.
GridPosition ahead(const GameplayController& game) {
    const GameEngine& engine = game.engine();
    const GridPosition head = engine.snake().front();
    return step(head, loopTurn(head, engine.direction()));
}

// Snake on row 10, head at (4, 10), heading right (inside the loop). Food
// starts at `food`; new food goes to the last free cell, (29, 19) unless taken.
GameEngine rowStart(GridPosition food) {
    return GameEngine({{4, 10}, {3, 10}, {2, 10}}, Direction::Right, food,
                      [](const std::vector<GridPosition>& cells) { return cells.back(); });
}

// New food on a random free cell from a seeded generator (ADR-005).
std::function<GridPosition(const std::vector<GridPosition>&)> seededFood(std::uint32_t seed) {
    return [rng = std::mt19937{seed}](const std::vector<GridPosition>& cells) mutable {
        std::uniform_int_distribution<std::size_t> pick(0, cells.size() - 1);
        return cells[pick(rng)];
    };
}

std::chrono::milliseconds baseInterval(Difficulty difficulty) {
    switch (difficulty) {
    case Difficulty::Easy:
        return 200ms;
    case Difficulty::Medium:
        return 120ms;
    case Difficulty::Hard:
        return 70ms;
    }
    return 0ms;
}

} // namespace

// ---------------------------------------------------------------------------
// STP TC-PWR-01..05 in the running game
// ---------------------------------------------------------------------------

TEST_CASE("TC-PWR-01 in play: power-ups appear 8-15 s apart on free cells, one at a time",
          "[powerups][gameplay][TC-PWR-01]") {
    const std::uint32_t seed = GENERATE(1u, 2u);
    CAPTURE(seed);

    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Easy},
                       GameEngine({{4, 10}, {3, 10}, {2, 10}}, Direction::Right, {29, 19},
                                  seededFood(seed)),
                       seed));
    const Micros lifetime = std::chrono::milliseconds{snake::config::PowerUpLifetimeMs};

    std::vector<Micros> spawns;
    std::size_t badCells = 0;
    bool hadItem = false;
    for (Micros now{0}; now < 10min;) {
        steer(game);
        game.update(AppStep, Screen::Gameplay);
        now += AppStep;

        const std::optional<PowerUp> item = game.powerUpOnGrid();
        if (item && !hadItem) {
            // A new power-up (the previous one is always gone by then):
            // record when it was due.
            spawns.push_back(now - (lifetime - item->remaining));
        }
        if (item && occupied(game.engine(), item->cell)) {
            ++badCells;
        }
        hadItem = item.has_value();
    }

    REQUIRE_FALSE(game.isGameOver());
    REQUIRE(badCells == 0);
    REQUIRE(spawns.size() >= 40);
    REQUIRE(spawns.front() >= 8s);
    REQUIRE(spawns.front() <= 15s);
    for (std::size_t i = 1; i < spawns.size(); ++i) {
        CAPTURE(i);
        REQUIRE(spawns[i] - spawns[i - 1] >= 8s);
        REQUIRE(spawns[i] - spawns[i - 1] <= 15s);
    }
}

TEST_CASE("TC-PWR-02 in play: Medium moves every 72 ms for 5.0 s, then every 120 ms",
          "[powerups][gameplay][TC-PWR-02]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Medium}, rowStart({29, 19}), 1u,
                       placedOnly()));
    game.placePowerUp(PowerUpType::Speed, {5, 10}); // on the first move
    REQUIRE(game.tickInterval() == 120ms);

    std::vector<Micros> moves;
    std::optional<Micros> collectedAt;
    std::optional<Micros> endedAt;
    std::size_t wrongIntervals = 0;
    GridPosition head = game.engine().snake().front();

    for (Micros now{0}; now < 8s;) {
        steer(game);
        game.update(1ms, Screen::Gameplay);
        now += 1ms;

        if (game.engine().snake().front() != head) {
            head = game.engine().snake().front();
            moves.push_back(now);
        }
        const auto effect = game.activeEffect();
        if (!collectedAt && effect) {
            collectedAt = now;
            REQUIRE(effect->type == PowerUpType::Speed);
            // HUD: the remaining time is shown from the start.
            REQUIRE(snake::describeEffect(*effect) == "Speed boost 5.0s");
        }
        if (collectedAt && effect && now == *collectedAt + 2500ms) {
            REQUIRE(snake::describeEffect(*effect) == "Speed boost 2.5s");
        }
        if (collectedAt && !endedAt && !effect) {
            endedAt = now;
        }
        // The tick interval read each update follows the effect.
        const auto expected = effect ? 72ms : 120ms;
        if (game.tickInterval() != expected) {
            ++wrongIntervals;
        }
    }

    REQUIRE_FALSE(game.isGameOver());
    REQUIRE(wrongIntervals == 0);
    REQUIRE(collectedAt == Micros{120ms});
    REQUIRE(endedAt);
    // 5.0 s within one tick.
    const Micros boost = *endedAt - *collectedAt;
    CAPTURE(boost.count());
    REQUIRE(boost >= 5s - 72ms);
    REQUIRE(boost <= 5s + 72ms);

    // The moves themselves: 72 ms apart during the boost, 120 ms after it.
    std::size_t boosted = 0;
    std::size_t normal = 0;
    for (std::size_t i = 1; i < moves.size(); ++i) {
        CAPTURE(i, moves[i].count());
        const Micros gap = moves[i] - moves[i - 1];
        if (moves[i - 1] >= *collectedAt && moves[i] <= *endedAt) {
            REQUIRE(gap == 72ms);
            ++boosted;
        } else if (moves[i - 1] >= *endedAt) {
            REQUIRE(gap == 120ms);
            ++normal;
        } else if (moves[i] > *endedAt) {
            // The move across the end of the boost.
            REQUIRE(gap >= 72ms);
            REQUIRE(gap <= 120ms);
        }
    }
    REQUIRE(boosted >= 68);
    REQUIRE(normal >= 20);
}

TEST_CASE("TC-PWR-03 in play: food scores +20, +20, then +10 once the x2 has ended",
          "[powerups][gameplay][TC-PWR-03]") {
    // Medium with 1 ms updates: move k happens at k x 120 ms. The Multiplier
    // is collected on move 1 (0.12 s); food is eaten on moves 9, 84 and 85,
    // which is 0.96 s, 9.96 s and 10.08 s after collecting it.
    const std::vector<GridPosition> path = loopPath({4, 10}, Direction::Right, 90);
    const std::vector<GridPosition> foodPlan{path[83], path[84], {0, 19}};
    std::size_t nextFood = 0;
    const auto plannedFood = [&foodPlan, &nextFood](const std::vector<GridPosition>& cells) {
        const GridPosition cell = foodPlan.at(nextFood++);
        REQUIRE(contains(cells, cell));
        return cell;
    };

    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Medium},
                       GameEngine({{4, 10}, {3, 10}, {2, 10}}, Direction::Right, path[8],
                                  plannedFood),
                       1u, placedOnly()));
    game.placePowerUp(PowerUpType::Multiplier, path[0]);

    std::optional<Micros> collectedAt;
    std::vector<std::pair<Micros, int>> scored; // (time after pick-up, points)
    int lastScore = 0;
    for (Micros now{0}; now < 10300ms;) {
        steer(game);
        game.update(1ms, Screen::Gameplay);
        now += 1ms;

        if (!collectedAt && game.activeEffect()) {
            collectedAt = now;
        }
        if (game.score() != lastScore) {
            REQUIRE(collectedAt);
            scored.emplace_back(now - *collectedAt, game.score() - lastScore);
            lastScore = game.score();
        }
    }

    REQUIRE_FALSE(game.isGameOver());
    REQUIRE(collectedAt == Micros{120ms});
    const std::vector<std::pair<Micros, int>> expected{
        {960ms, 20}, {9960ms, 20}, {10080ms, 10}};
    REQUIRE(scored == expected);
    REQUIRE(game.score() == 50);
    REQUIRE(game.highScore() == 50);
    REQUIRE_FALSE(game.activeEffect());
}

TEST_CASE("TC-PWR-04 in play: Shrink at lengths 6, 4 and 3 gives 5, 3 and 3",
          "[powerups][gameplay][TC-PWR-04]") {
    const std::size_t length = GENERATE(6u, 4u, 3u);
    const std::size_t expected = length == 6 ? 5u : 3u;
    CAPTURE(length);

    std::deque<GridPosition> body;
    for (std::size_t i = 0; i < length; ++i) {
        body.push_back({10 - static_cast<int>(i), 10});
    }
    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Medium},
                       GameEngine(body, Direction::Right, {29, 19}), 1u, placedOnly()));
    game.placePowerUp(PowerUpType::Shrink, {11, 10});

    game.update(120ms, Screen::Gameplay);

    REQUIRE(game.engine().snake().front() == GridPosition{11, 10});
    REQUIRE(game.engine().snake().size() == expected);
    REQUIRE_FALSE(game.powerUpOnGrid()); // consumed
    REQUIRE_FALSE(game.activeEffect());
    REQUIRE(game.tickInterval() == 120ms);
    REQUIRE(game.score() == 0);
}

TEST_CASE("TC-PWR-05 in play: Multiplier replaces Speed at once and leaves no speed change",
          "[powerups][gameplay][TC-PWR-05]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Medium}, rowStart({0, 19}), 1u,
                       placedOnly()));
    game.placePowerUp(PowerUpType::Speed, {5, 10});

    Micros now{0};
    const auto play = [&](Micros duration) {
        const Micros end = now + duration;
        while (now < end) {
            steer(game);
            game.update(1ms, Screen::Gameplay);
            now += 1ms;
        }
    };

    play(120ms);
    REQUIRE(game.activeEffect()->type == PowerUpType::Speed);
    REQUIRE(game.tickInterval() == 72ms);

    // 2 s later, a Multiplier straight ahead.
    play(2s);
    REQUIRE(game.activeEffect()->type == PowerUpType::Speed);
    game.placePowerUp(PowerUpType::Multiplier, ahead(game));
    while (game.activeEffect() && game.activeEffect()->type == PowerUpType::Speed) {
        REQUIRE(now < 2200ms);
        play(1ms);
    }
    // Speed ended the moment the Multiplier was collected.
    REQUIRE(game.activeEffect());
    REQUIRE(game.activeEffect()->type == PowerUpType::Multiplier);
    REQUIRE(game.activeEffect()->remaining >= 10s - 1ms);
    REQUIRE(game.tickInterval() == 120ms);

    // The next 10 s: the Multiplier runs out, the speed never changes again.
    std::size_t wrongIntervals = 0;
    for (int ms = 0; ms < 10000; ++ms) {
        play(1ms);
        const auto effect = game.activeEffect();
        if (effect) {
            REQUIRE(effect->type == PowerUpType::Multiplier);
        }
        if (game.tickInterval() != 120ms) {
            ++wrongIntervals;
        }
    }
    REQUIRE(wrongIntervals == 0);
    REQUIRE_FALSE(game.activeEffect());

    // Speed again works as the first time.
    game.placePowerUp(PowerUpType::Speed, ahead(game));
    play(120ms);
    REQUIRE(game.activeEffect()->type == PowerUpType::Speed);
    REQUIRE(game.tickInterval() == 72ms);
    REQUIRE_FALSE(game.isGameOver());
}

// ---------------------------------------------------------------------------
// Speed per difficulty, pause, game over, restart, placement
// ---------------------------------------------------------------------------

TEST_CASE("A Speed boost makes Easy, Medium and Hard 120, 72 and 42 ms",
          "[powerups][gameplay][TC-PWR-02]") {
    const Difficulty difficulty = GENERATE(from_range(snake::AllDifficulties));
    CAPTURE(snake::toString(difficulty));

    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, difficulty}, rowStart({29, 19}), 1u, placedOnly()));
    game.placePowerUp(PowerUpType::Speed, {5, 10});
    REQUIRE(game.tickInterval() == baseInterval(difficulty));

    game.update(baseInterval(difficulty), Screen::Gameplay);
    REQUIRE(game.activeEffect()->type == PowerUpType::Speed);
    REQUIRE(game.tickInterval() == baseInterval(difficulty) * 3 / 5);
}

TEST_CASE("Pausing freezes the effect, the power-up on the grid and the countdown",
          "[powerups][gameplay][TC-UI-03]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Timed, Difficulty::Medium}, rowStart({29, 19}), 1u,
                       placedOnly()));
    game.placePowerUp(PowerUpType::Speed, {5, 10});
    game.update(120ms, Screen::Gameplay);
    game.placePowerUp(PowerUpType::Shrink, {20, 5});

    const auto effect = game.activeEffect();
    const auto item = game.powerUpOnGrid();
    const auto countdown = game.remainingTime();
    const GridPosition head = game.engine().snake().front();
    REQUIRE(effect);
    REQUIRE(item);

    for (int i = 0; i < 600; ++i) {
        game.update(AppStep, i % 2 == 0 ? Screen::Paused : Screen::MainMenu);
    }
    REQUIRE(game.activeEffect()->remaining == effect->remaining);
    REQUIRE(game.powerUpOnGrid()->remaining == item->remaining);
    REQUIRE(game.remainingTime() == countdown);
    REQUIRE(game.engine().snake().front() == head);

    // Resuming continues from the same state.
    game.update(AppStep, Screen::Gameplay);
    REQUIRE(game.activeEffect()->remaining == effect->remaining - AppStep);
    REQUIRE(game.powerUpOnGrid()->remaining == item->remaining - AppStep);
}

TEST_CASE("A finished game keeps its power-up state", "[powerups][gameplay]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Hard}, rowStart({29, 19}), 1u,
                       placedOnly()));
    game.placePowerUp(PowerUpType::Multiplier, {5, 10});
    for (int i = 0; i < 100 && !game.isGameOver(); ++i) {
        game.update(70ms, Screen::Gameplay); // straight into the right wall
    }
    REQUIRE(game.gameOverReason() == GameOverReason::WALL);

    const auto effect = game.activeEffect();
    REQUIRE(effect);
    game.update(5s, Screen::Gameplay);
    REQUIRE(game.activeEffect()->remaining == effect->remaining);
}

TEST_CASE("A new game starts without power-ups, effects or multiplier",
          "[powerups][gameplay]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Medium}, rowStart({6, 10}), 1u,
                       placedOnly()));
    game.placePowerUp(PowerUpType::Multiplier, {5, 10});
    game.update(120ms, Screen::Gameplay); // collects the Multiplier
    game.update(120ms, Screen::Gameplay); // eats at (6, 10)
    REQUIRE(game.score() == 20);
    game.placePowerUp(PowerUpType::Speed, {20, 5});

    REQUIRE(game.start({GameMode::Classic, Difficulty::Medium}, rowStart({5, 10}), 2u,
                       placedOnly()));
    REQUIRE_FALSE(game.powerUpOnGrid());
    REQUIRE_FALSE(game.activeEffect());
    REQUIRE(game.engine().foodExclusions().empty());
    REQUIRE(game.tickInterval() == 120ms);
    REQUIRE(game.score() == 0);
    REQUIRE(game.highScore() == 20);

    game.update(120ms, Screen::Gameplay); // eats at (5, 10): no multiplier
    REQUIRE(game.score() == 10);
}

TEST_CASE("A new game after a Speed boost moves at the normal speed", "[powerups][gameplay]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Medium}, rowStart({29, 19}), 1u,
                       placedOnly()));
    game.placePowerUp(PowerUpType::Speed, {5, 10});
    game.update(120ms, Screen::Gameplay);
    REQUIRE(game.tickInterval() == 72ms);

    REQUIRE(game.start({GameMode::Classic, Difficulty::Medium}));
    REQUIRE(game.tickInterval() == 120ms);
    REQUIRE_FALSE(game.activeEffect());
}

TEST_CASE("New food never lands on the power-up", "[powerups][gameplay][FR-012]") {
    bool itemCellOffered = false;
    const GridPosition itemCell{0, 0};
    const auto firstFree = [&itemCellOffered, itemCell](const std::vector<GridPosition>& cells) {
        itemCellOffered = itemCellOffered || contains(cells, itemCell);
        return cells.front();
    };

    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Medium},
                       GameEngine({{4, 10}, {3, 10}, {2, 10}}, Direction::Right, {5, 10},
                                  firstFree),
                       1u, placedOnly()));
    game.placePowerUp(PowerUpType::Shrink, itemCell);
    REQUIRE(game.engine().foodExclusions() == std::vector<GridPosition>{itemCell});

    game.update(120ms, Screen::Gameplay); // eats at (5, 10): new food
    REQUIRE(game.score() == 10);
    REQUIRE_FALSE(itemCellOffered);
    REQUIRE(game.engine().food() == GridPosition{1, 0});
    REQUIRE(game.powerUpOnGrid()->cell == itemCell);
}

TEST_CASE("The food exclusion follows the power-up", "[powerups][gameplay]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Medium}, rowStart({29, 19}), 1u,
                       placedOnly()));
    REQUIRE(game.engine().foodExclusions().empty());

    game.placePowerUp(PowerUpType::Speed, {20, 5});
    REQUIRE(game.engine().foodExclusions() == std::vector<GridPosition>{{20, 5}});

    // Moved by placing another one.
    game.placePowerUp(PowerUpType::Shrink, {5, 10});
    REQUIRE(game.engine().foodExclusions() == std::vector<GridPosition>{{5, 10}});

    // Collected: no exclusion left.
    game.update(120ms, Screen::Gameplay);
    REQUIRE_FALSE(game.powerUpOnGrid());
    REQUIRE(game.engine().foodExclusions().empty());
}

TEST_CASE("An uncollected power-up disappears from the game after 5 s",
          "[powerups][gameplay][FR-012]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Easy}, rowStart({29, 19}), 1u,
                       placedOnly()));
    game.placePowerUp(PowerUpType::Multiplier, {1, 1}); // off the loop

    for (Micros now{0}; now < 4999ms; now += 1ms) {
        steer(game);
        game.update(1ms, Screen::Gameplay);
    }
    REQUIRE(game.powerUpOnGrid());
    steer(game);
    game.update(1ms, Screen::Gameplay);
    REQUIRE_FALSE(game.powerUpOnGrid());
    REQUIRE(game.engine().foodExclusions().empty());
    REQUIRE_FALSE(game.activeEffect());
}

TEST_CASE("A normal game gets its first power-up within 15 s", "[powerups][gameplay][TC-PWR-01]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Easy}));

    bool seen = false;
    for (Micros now{0}; now < 15s + AppStep && !seen; now += AppStep) {
        steer(game);
        game.update(AppStep, Screen::Gameplay);
        seen = game.powerUpOnGrid().has_value() || game.activeEffect().has_value();
    }
    REQUIRE_FALSE(game.isGameOver());
    REQUIRE(seen);
}

TEST_CASE("Power-ups can only be placed on free cells of a running game",
          "[powerups][gameplay]") {
    GameplayController none;
    REQUIRE_THROWS_AS(none.placePowerUp(PowerUpType::Speed, {1, 1}), std::logic_error);
    REQUIRE_FALSE(none.powerUpOnGrid());
    REQUIRE_FALSE(none.activeEffect());

    GameplayController game;
    REQUIRE(game.start({GameMode::Obstacle, Difficulty::Medium}, rowStart({29, 19}), 3u,
                       placedOnly()));
    REQUIRE_FALSE(game.engine().obstacles().empty());

    REQUIRE_THROWS_AS(game.placePowerUp(PowerUpType::Speed, {3, 10}), std::invalid_argument);
    REQUIRE_THROWS_AS(game.placePowerUp(PowerUpType::Speed, {29, 19}), std::invalid_argument);
    REQUIRE_THROWS_AS(game.placePowerUp(PowerUpType::Speed, game.engine().obstacles().front()),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(game.placePowerUp(PowerUpType::Speed, {-1, 0}), std::invalid_argument);
    REQUIRE_THROWS_AS(game.placePowerUp(PowerUpType::Speed, {30, 5}), std::invalid_argument);
    REQUIRE_FALSE(game.powerUpOnGrid());

    // A free cell works.
    GridPosition freeCell{0, 0};
    while (occupied(game.engine(), freeCell)) {
        freeCell.x += 1;
    }
    game.placePowerUp(PowerUpType::Speed, freeCell);
    REQUIRE(game.powerUpOnGrid()->cell == freeCell);
}

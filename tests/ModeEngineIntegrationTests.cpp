// Integration tests: ModeManager (C7) wired to GameEngine (C6) the way the
// application loop drives them (issue #13, TC-MODE-02..04).

#include "snakextreme/GameConfig.hpp"
#include "snakextreme/GameEngine.hpp"
#include "snakextreme/GameMode.hpp"
#include "snakextreme/ModeManager.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <deque>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace std::chrono_literals;

using snake::Difficulty;
using snake::Direction;
using snake::GameEngine;
using snake::GameMode;
using snake::GameOverReason;
using snake::GridPosition;
using snake::ModeManager;

namespace {

// Test harness mirroring the application loop (SAD ADR-002): the mode timer
// advances on every update, and the snake moves one cell per difficulty
// tick interval. A paused game simply stops calling advance().
class ModeSession {
public:
    ModeSession(snake::SessionConfig config,
                std::uint32_t seed,
                GameEngine engine = GameEngine{})
        : engine_(std::move(engine)), modes_(config, seed) {
        std::optional<GridPosition> food;
        if (engine_.hasFood()) {
            food = engine_.food();
        }
        modes_.startSession(engine_.snake(), food);
        engine_.setObstacles(modes_.obstacles());
    }

    void advance(std::chrono::milliseconds elapsed) {
        if (engine_.isGameOver()) {
            return;
        }

        modes_.update(elapsed);
        if (modes_.isTimeUp()) {
            engine_.endGame(GameOverReason::TIME_UP);
            return;
        }

        accumulator_ += elapsed;
        while (accumulator_ >= modes_.tickInterval() && !engine_.isGameOver()) {
            accumulator_ -= modes_.tickInterval();
            engine_.tick();
            ++ticks_;
        }
    }

    GameEngine& engine() { return engine_; }
    const ModeManager& modes() const { return modes_; }
    int ticks() const { return ticks_; }

private:
    GameEngine engine_;
    ModeManager modes_;
    std::chrono::milliseconds accumulator_{0};
    int ticks_{0};
};

// A food selector that always picks the last free cell (bottom-right area),
// keeping food away from the snake's path in these tests.
GridPosition lastFreeCell(const std::vector<GridPosition>& freeCells) {
    return freeCells.back();
}

bool contains(const std::vector<GridPosition>& cells, GridPosition cell) {
    return std::find(cells.begin(), cells.end(), cell) != cells.end();
}

} // namespace

// ---------------------------------------------------------------------------
// TC-MODE-02: difficulty decides the movement speed
// ---------------------------------------------------------------------------

TEST_CASE("Difficulty decides how many cells the snake moves in one second",
          "[difficulty][integration][TC-MODE-02]") {
    const GameMode mode = GENERATE(GameMode::Classic, GameMode::Timed);
    const Difficulty difficulty = GENERATE(from_range(snake::AllDifficulties));
    CAPTURE(snake::toString(mode), snake::toString(difficulty));

    ModeSession session(
        {mode, difficulty}, 1u,
        GameEngine({{2, 10}, {1, 10}, {0, 10}}, Direction::Right, {29, 19}, lastFreeCell));

    for (int step = 0; step < 100; ++step) {
        session.advance(10ms);
    }

    // 1000 ms / interval, rounded down: Easy 5, Medium 8, Hard 14.
    const int expectedTicks =
        difficulty == Difficulty::Easy ? 5 : difficulty == Difficulty::Medium ? 8 : 14;

    REQUIRE(session.ticks() == expectedTicks);
    REQUIRE(session.engine().snake().front() == GridPosition{2 + expectedTicks, 10});
    REQUIRE_FALSE(session.engine().isGameOver());
}

// ---------------------------------------------------------------------------
// TC-MODE-03: the Timed Challenge ends the game at zero
// ---------------------------------------------------------------------------

TEST_CASE("A Timed Challenge game ends with TIME_UP at exactly 60 seconds",
          "[timed][integration][TC-MODE-03]") {
    // Easy speed: one tick per 200 ms update. The snake circles a 6x6 square
    // so it stays alive for the whole minute.
    ModeSession session(
        {GameMode::Timed, Difficulty::Easy}, 1u,
        GameEngine({{10, 10}, {9, 10}, {8, 10}}, Direction::Right, {29, 19}, lastFreeCell));

    const Direction turns[] = {Direction::Down, Direction::Left, Direction::Up, Direction::Right};
    int turn = 0;

    // 299 ticks = 59.8 s.
    for (int step = 1; step <= 299; ++step) {
        session.advance(200ms);
        if (step % 6 == 0) {
            session.engine().changeDirection(turns[turn]);
            turn = (turn + 1) % 4;
        }
    }

    session.advance(100ms); // 59.9 s
    REQUIRE_FALSE(session.engine().isGameOver());
    REQUIRE(session.modes().remainingTime() == std::optional<std::chrono::milliseconds>{100ms});

    const GridPosition headBefore = session.engine().snake().front();
    const auto lengthBefore = session.engine().snake().size();

    session.advance(100ms); // 60.0 s

    REQUIRE(session.engine().isGameOver());
    REQUIRE(session.engine().gameOverReason() == GameOverReason::TIME_UP);
    REQUIRE(session.modes().remainingTime() == std::optional<std::chrono::milliseconds>{0ms});
    REQUIRE(session.ticks() == 299);

    // The final state is kept for the score and Game Over screen.
    session.advance(5s);
    REQUIRE(session.engine().snake().front() == headBefore);
    REQUIRE(session.engine().snake().size() == lengthBefore);
    REQUIRE(session.engine().gameOverReason() == GameOverReason::TIME_UP);
}

TEST_CASE("Pausing freezes the Timed countdown",
          "[timed][integration][TC-MODE-03]") {
    ModeSession session(
        {GameMode::Timed, Difficulty::Easy}, 1u,
        GameEngine({{2, 10}, {1, 10}, {0, 10}}, Direction::Right, {29, 19}, lastFreeCell));

    session.advance(2s);
    REQUIRE(session.modes().remainingTime() == std::optional<std::chrono::milliseconds>{58s});
    const GridPosition headAtPause = session.engine().snake().front();

    // While paused, the application stops calling advance(), so however long
    // the pause lasts, resuming continues from 58 s rather than wall time.
    session.advance(1s);
    REQUIRE(session.modes().remainingTime() == std::optional<std::chrono::milliseconds>{57s});
    REQUIRE(session.engine().snake().front() != headAtPause);
}

TEST_CASE("A game that is already over keeps its first game-over reason",
          "[timed][integration][TC-MODE-03][negative]") {
    GameEngine engine({{29, 5}, {28, 5}, {27, 5}}, Direction::Right, {0, 0});
    engine.tick();
    REQUIRE(engine.gameOverReason() == GameOverReason::WALL);

    engine.endGame(GameOverReason::TIME_UP);
    REQUIRE(engine.gameOverReason() == GameOverReason::WALL);

    GameEngine running;
    running.endGame(GameOverReason::None);
    REQUIRE_FALSE(running.isGameOver());
}

// ---------------------------------------------------------------------------
// TC-MODE-04: obstacles in the engine
// ---------------------------------------------------------------------------

TEST_CASE("An Obstacle-mode session hands its 8 obstacles to the engine",
          "[obstacle][integration][TC-MODE-04]") {
    for (std::uint32_t seed = 0; seed < 50; ++seed) {
        INFO("seed " << seed);
        ModeSession session({GameMode::Obstacle, Difficulty::Medium}, seed);
        const GameEngine& engine = session.engine();

        REQUIRE(engine.obstacles() == session.modes().obstacles());
        REQUIRE(engine.obstacles().size() == 8);
        REQUIRE(engine.hasFood());
        REQUIRE_FALSE(contains(engine.obstacles(), engine.food()));
    }
}

TEST_CASE("Classic and Timed sessions give the engine no obstacles",
          "[obstacle][integration][TC-MODE-04][negative]") {
    const GameMode mode = GENERATE(GameMode::Classic, GameMode::Timed);
    CAPTURE(snake::toString(mode));

    ModeSession session({mode, Difficulty::Medium}, 4u);
    REQUIRE(session.engine().obstacles().empty());
}

TEST_CASE("Moving into an obstacle ends the game with OBSTACLE",
          "[obstacle][integration][TC-MODE-04]") {
    GameEngine engine({{5, 5}, {4, 5}, {3, 5}}, Direction::Right, {20, 15});
    engine.setObstacles({{6, 5}});

    engine.tick();

    REQUIRE(engine.isGameOver());
    REQUIRE(engine.gameOverReason() == GameOverReason::OBSTACLE);
    // The head never enters the obstacle cell.
    REQUIRE(engine.snake().front() == GridPosition{5, 5});
    REQUIRE(engine.snake().size() == 3);
}

TEST_CASE("Every generated obstacle is fatal when the snake runs into it",
          "[obstacle][integration][TC-MODE-04]") {
    int collisionsChecked = 0;

    for (std::uint32_t seed = 0; seed < 20; ++seed) {
        ModeManager modes({GameMode::Obstacle, Difficulty::Hard}, seed);
        modes.startSession({{15, 10}, {14, 10}, {13, 10}}, GridPosition{0, 0});
        const auto& layout = modes.obstacles();

        for (const GridPosition obstacle : layout) {
            // Snake approaching from the left, head next to the obstacle.
            const std::deque<GridPosition> body{
                {obstacle.x - 1, obstacle.y},
                {obstacle.x - 2, obstacle.y},
                {obstacle.x - 3, obstacle.y}};

            const bool fits = std::all_of(body.begin(), body.end(), [&](GridPosition cell) {
                return snake::isInsideGrid(cell) && !contains(layout, cell);
            });
            if (!fits) {
                continue;
            }

            GridPosition food{0, 0};
            while (contains(layout, food) ||
                   std::find(body.begin(), body.end(), food) != body.end()) {
                food.x += 1;
            }

            INFO("seed " << seed << ", obstacle (" << obstacle.x << ", " << obstacle.y << ")");
            GameEngine engine(body, Direction::Right, food);
            engine.setObstacles(layout);
            engine.tick();

            REQUIRE(engine.gameOverReason() == GameOverReason::OBSTACLE);
            ++collisionsChecked;
        }
    }

    REQUIRE(collisionsChecked > 50);
}

TEST_CASE("Food never respawns on an obstacle",
          "[obstacle][integration][TC-MODE-04]") {
    // Obstacles cover every cell except the snake, the food and one target.
    std::vector<GridPosition> obstacles;
    for (int y = 0; y < snake::config::GridRows; ++y) {
        for (int x = 0; x < snake::config::GridColumns; ++x) {
            if (!(y == 0 && x <= 2)) {
                obstacles.push_back({x, y});
            }
        }
    }

    GameEngine engine({{0, 0}}, Direction::Right, {1, 0});
    engine.setObstacles(obstacles);
    engine.tick();

    REQUIRE_FALSE(engine.isGameOver());
    REQUIRE(engine.snake().size() == 2);
    REQUIRE(engine.hasFood());
    REQUIRE(engine.food() == GridPosition{2, 0});
}

TEST_CASE("Food lying on a new obstacle is moved to a free cell",
          "[obstacle][integration][TC-MODE-04]") {
    GameEngine engine({{5, 5}, {4, 5}, {3, 5}}, Direction::Right, {10, 10});
    engine.setObstacles({{10, 10}, {11, 10}});

    REQUIRE(engine.hasFood());
    REQUIRE(engine.food() != GridPosition{10, 10});
    REQUIRE_FALSE(contains(engine.obstacles(), engine.food()));
    REQUIRE(std::find(engine.snake().begin(), engine.snake().end(), engine.food()) ==
            engine.snake().end());
}

TEST_CASE("Invalid obstacle cells are rejected",
          "[obstacle][integration][TC-MODE-04][negative]") {
    GameEngine engine({{5, 5}, {4, 5}, {3, 5}}, Direction::Right, {10, 10});

    REQUIRE_THROWS_AS(engine.setObstacles({{4, 5}}), std::invalid_argument);
    REQUIRE_THROWS_AS(engine.setObstacles({{-1, 0}}), std::invalid_argument);
    REQUIRE_THROWS_AS(engine.setObstacles({{snake::config::GridColumns, 0}}),
                      std::invalid_argument);

    engine.setObstacles({{8, 8}, {8, 8}});
    REQUIRE(engine.obstacles().size() == 1);
}

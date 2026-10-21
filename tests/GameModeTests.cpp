// Unit tests for C7 ModeManager: game modes, difficulty profiles, the Timed
// countdown and obstacle placement (issue #13, TC-MODE-01..04).

#include "snakextreme/DifficultyProfile.hpp"
#include "snakextreme/GameConfig.hpp"
#include "snakextreme/GameMode.hpp"
#include "snakextreme/ModeManager.hpp"
#include "snakextreme/ModeStrategy.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <deque>
#include <optional>
#include <random>
#include <stdexcept>

using namespace std::chrono_literals;

using snake::Difficulty;
using snake::GameMode;
using snake::GridPosition;
using snake::ModeManager;
using snake::Obstacles;

namespace {

// The engine's starting snake: head at the grid centre, heading right.
std::deque<GridPosition> startingSnake() {
    return {{15, 10}, {14, 10}, {13, 10}};
}

std::chrono::milliseconds expectedInterval(Difficulty difficulty) {
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

bool hasDuplicates(Obstacles cells) {
    for (std::size_t i = 0; i < cells.size(); ++i) {
        for (std::size_t j = i + 1; j < cells.size(); ++j) {
            if (cells[i] == cells[j]) {
                return true;
            }
        }
    }
    return false;
}

void requireValidLayout(const Obstacles& obstacles,
                        const std::deque<GridPosition>& snake,
                        std::optional<GridPosition> food,
                        std::size_t expectedCount = snake::config::ObstacleCount) {
    REQUIRE(obstacles.size() == expectedCount);
    REQUIRE_FALSE(hasDuplicates(obstacles));
    REQUIRE(snake::keepsBoardConnected(obstacles));

    for (const GridPosition cell : obstacles) {
        INFO("obstacle at (" << cell.x << ", " << cell.y << ")");
        REQUIRE(snake::isInsideGrid(cell));
        REQUIRE(std::find(snake.begin(), snake.end(), cell) == snake.end());
        if (food) {
            REQUIRE(cell != *food);
        }
        REQUIRE(snake::chebyshevDistance(cell, snake.front()) >=
                snake::config::ObstacleMinHeadDistance);
    }
}

// A snake with its head at `head` whose body covers every cell except
// `freeCells`. Snake cells can never hold obstacles but stay passable, so
// this limits placement to `freeCells` without blocking any paths.
std::deque<GridPosition> snakeCoveringAllBut(GridPosition head,
                                             const Obstacles& freeCells) {
    std::deque<GridPosition> body{head};
    for (int y = 0; y < snake::config::GridRows; ++y) {
        for (int x = 0; x < snake::config::GridColumns; ++x) {
            const GridPosition cell{x, y};
            const bool keepFree =
                std::find(freeCells.begin(), freeCells.end(), cell) != freeCells.end();
            if (cell != head && !keepFree) {
                body.push_back(cell);
            }
        }
    }
    return body;
}

} // namespace

// ---------------------------------------------------------------------------
// TC-MODE-01 / FR-008: mode selection
// ---------------------------------------------------------------------------

TEST_CASE("Exactly three game modes are offered and Classic is the default",
          "[modes][TC-MODE-01]") {
    REQUIRE(snake::AllGameModes.size() == 3);
    REQUIRE(snake::AllGameModes[0] == GameMode::Classic);
    REQUIRE(snake::AllGameModes[1] == GameMode::Timed);
    REQUIRE(snake::AllGameModes[2] == GameMode::Obstacle);

    REQUIRE(snake::DefaultGameMode == GameMode::Classic);
    REQUIRE(snake::SessionConfig{}.mode == GameMode::Classic);
    REQUIRE(ModeManager{}.mode() == GameMode::Classic);
}

TEST_CASE("Mode and difficulty names match the stored leaderboard values",
          "[modes][TC-MODE-01]") {
    REQUIRE(snake::toString(GameMode::Classic) == "Classic");
    REQUIRE(snake::toString(GameMode::Timed) == "Timed");
    REQUIRE(snake::toString(GameMode::Obstacle) == "Obstacle");
    REQUIRE(snake::toString(Difficulty::Easy) == "Easy");
    REQUIRE(snake::toString(Difficulty::Medium) == "Medium");
    REQUIRE(snake::toString(Difficulty::Hard) == "Hard");

    for (const GameMode mode : snake::AllGameModes) {
        REQUIRE(snake::parseGameMode(snake::toString(mode)) == mode);
    }
    for (const Difficulty difficulty : snake::AllDifficulties) {
        REQUIRE(snake::parseDifficulty(snake::toString(difficulty)) == difficulty);
    }

    // Persistence treats anything else as a schema error (SEC-003).
    REQUIRE_FALSE(snake::parseGameMode("").has_value());
    REQUIRE_FALSE(snake::parseGameMode("classic").has_value());
    REQUIRE_FALSE(snake::parseGameMode("Timed Challenge").has_value());
    REQUIRE_FALSE(snake::parseGameMode("Obstacle ").has_value());
    REQUIRE_FALSE(snake::parseDifficulty("hard").has_value());
    REQUIRE_FALSE(snake::parseDifficulty("Expert").has_value());
}

TEST_CASE("Each mode applies its own strategy and rules",
          "[modes][TC-MODE-01]") {
    const GameMode mode = GENERATE(from_range(snake::AllGameModes));
    CAPTURE(snake::toString(mode));

    ModeManager modes({mode, Difficulty::Medium}, 42u);
    modes.startSession(startingSnake(), GridPosition{25, 5});

    REQUIRE(modes.mode() == mode);
    REQUIRE(modes.strategy().mode() == mode);
    REQUIRE(snake::makeModeStrategy(mode)->mode() == mode);

    // Timer only in Timed; obstacles only in Obstacle.
    REQUIRE(modes.hasTimeLimit() == (mode == GameMode::Timed));
    REQUIRE(modes.remainingTime().has_value() == (mode == GameMode::Timed));
    REQUIRE(modes.obstacles().empty() == (mode != GameMode::Obstacle));
}

TEST_CASE("Food points per mode match the SRS baseline",
          "[modes][FR-016]") {
    REQUIRE(snake::ClassicMode{}.foodPoints() == 10);
    REQUIRE(snake::TimedMode{}.foodPoints() == 10);
    REQUIRE(snake::ObstacleMode{}.foodPoints() == 15);
    REQUIRE(ModeManager({GameMode::Obstacle, Difficulty::Easy}, 1u).foodPoints() == 15);
}

// ---------------------------------------------------------------------------
// TC-MODE-02 / FR-009: difficulty speed profiles
// ---------------------------------------------------------------------------

TEST_CASE("Difficulty profiles use 200, 120 and 70 ms tick intervals",
          "[difficulty][TC-MODE-02]") {
    REQUIRE(snake::tickInterval(Difficulty::Easy) == 200ms);
    REQUIRE(snake::tickInterval(Difficulty::Medium) == 120ms);
    REQUIRE(snake::tickInterval(Difficulty::Hard) == 70ms);

    for (const Difficulty difficulty : snake::AllDifficulties) {
        const auto profile = snake::difficultyProfile(difficulty);
        REQUIRE(profile.difficulty == difficulty);
        REQUIRE(profile.tickInterval == expectedInterval(difficulty));
    }

    REQUIRE(snake::AllDifficulties.size() == 3);
    REQUIRE(snake::AllDifficulties[0] == Difficulty::Easy);
    REQUIRE(snake::AllDifficulties[1] == Difficulty::Medium);
    REQUIRE(snake::AllDifficulties[2] == Difficulty::Hard);
}

TEST_CASE("All nine mode and difficulty combinations start at the difficulty's speed",
          "[difficulty][modes][TC-MODE-02]") {
    const GameMode mode = GENERATE(from_range(snake::AllGameModes));
    const Difficulty difficulty = GENERATE(from_range(snake::AllDifficulties));
    CAPTURE(snake::toString(mode), snake::toString(difficulty));

    ModeManager modes({mode, difficulty}, 7u);
    REQUIRE_NOTHROW(modes.startSession(startingSnake(), GridPosition{25, 15}));

    REQUIRE(modes.difficulty() == difficulty);
    REQUIRE(modes.tickInterval() == expectedInterval(difficulty));
}

// ---------------------------------------------------------------------------
// TC-MODE-03 / FR-010: Timed Challenge countdown
// ---------------------------------------------------------------------------

TEST_CASE("Timed Challenge starts with a 60 second countdown",
          "[timed][TC-MODE-03]") {
    REQUIRE(snake::TimedMode{}.timeLimit() == std::optional<std::chrono::milliseconds>{60s});

    ModeManager timed({GameMode::Timed, Difficulty::Medium}, 1u);
    timed.startSession(startingSnake(), std::nullopt);

    REQUIRE(timed.remainingTime() == std::optional<std::chrono::milliseconds>{60s});
    REQUIRE_FALSE(timed.isTimeUp());
}

TEST_CASE("Timed countdown is running at 59.9 s and ends at 60.0 s",
          "[timed][TC-MODE-03]") {
    ModeManager timed({GameMode::Timed, Difficulty::Medium}, 1u);
    timed.startSession(startingSnake(), std::nullopt);

    for (int step = 0; step < 599; ++step) {
        timed.update(100ms);
    }

    REQUIRE_FALSE(timed.isTimeUp());
    REQUIRE(timed.remainingTime() == std::optional<std::chrono::milliseconds>{100ms});

    timed.update(100ms);

    REQUIRE(timed.isTimeUp());
    REQUIRE(timed.remainingTime() == std::optional<std::chrono::milliseconds>{0ms});
}

TEST_CASE("A 60 Hz update step does not make the countdown drift",
          "[timed][TC-MODE-03]") {
    // The application updates at 60 Hz: sf::seconds(1.f / 60.f) is 16666 us.
    // Whole-millisecond steps (16 ms) would stretch the minute to 62.5 s.
    constexpr std::chrono::microseconds frame{16666};

    ModeManager timed({GameMode::Timed, Difficulty::Medium}, 1u);
    timed.startSession(startingSnake(), std::nullopt);

    for (int step = 0; step < 3600; ++step) {
        timed.update(frame);
    }
    // 3600 frames = 59.9976 s: 2.4 ms left, shown rounded up.
    REQUIRE_FALSE(timed.isTimeUp());
    REQUIRE(timed.remainingTime() == std::optional<std::chrono::milliseconds>{3ms});

    timed.update(frame);
    REQUIRE(timed.isTimeUp());
    REQUIRE(timed.remainingTime() == std::optional<std::chrono::milliseconds>{0ms});
}

TEST_CASE("Timed countdown never goes below zero",
          "[timed][TC-MODE-03][boundary]") {
    ModeManager timed({GameMode::Timed, Difficulty::Hard}, 1u);
    timed.startSession(startingSnake(), std::nullopt);

    timed.update(59s);
    timed.update(5s);
    REQUIRE(timed.isTimeUp());
    REQUIRE(timed.remainingTime() == std::optional<std::chrono::milliseconds>{0ms});

    timed.update(std::chrono::hours{10});
    REQUIRE(timed.remainingTime() == std::optional<std::chrono::milliseconds>{0ms});
}

TEST_CASE("Timed countdown ignores zero and negative steps",
          "[timed][TC-MODE-03][negative]") {
    ModeManager timed({GameMode::Timed, Difficulty::Easy}, 1u);
    timed.startSession(startingSnake(), std::nullopt);

    timed.update(0ms);
    timed.update(-5s);

    REQUIRE(timed.remainingTime() == std::optional<std::chrono::milliseconds>{60s});
}

TEST_CASE("Starting a new session resets the countdown",
          "[timed][TC-MODE-03]") {
    ModeManager timed({GameMode::Timed, Difficulty::Easy}, 1u);
    timed.startSession(startingSnake(), std::nullopt);
    timed.update(60s);
    REQUIRE(timed.isTimeUp());

    timed.startSession(startingSnake(), std::nullopt);

    REQUIRE_FALSE(timed.isTimeUp());
    REQUIRE(timed.remainingTime() == std::optional<std::chrono::milliseconds>{60s});
}

TEST_CASE("Classic and Obstacle modes never time out",
          "[timed][TC-MODE-03][negative]") {
    const GameMode mode = GENERATE(GameMode::Classic, GameMode::Obstacle);
    CAPTURE(snake::toString(mode));

    ModeManager modes({mode, Difficulty::Medium}, 3u);
    modes.startSession(startingSnake(), std::nullopt);
    modes.update(std::chrono::minutes{10});

    REQUIRE_FALSE(modes.isTimeUp());
    REQUIRE_FALSE(modes.remainingTime().has_value());
}

// ---------------------------------------------------------------------------
// TC-MODE-04 / FR-011: obstacle placement and collision rules
// ---------------------------------------------------------------------------

TEST_CASE("Obstacle mode places 8 valid obstacles for 500 seeds",
          "[obstacle][TC-MODE-04]") {
    const auto snakeBody = startingSnake();
    const GridPosition food{20, 4};

    REQUIRE(snake::config::ObstacleCount == 8);

    for (std::uint32_t seed = 0; seed < 500; ++seed) {
        INFO("seed " << seed);
        ModeManager modes({GameMode::Obstacle, Difficulty::Medium}, seed);
        modes.startSession(snakeBody, food);
        REQUIRE(modes.obstacles().size() == 8);
        requireValidLayout(modes.obstacles(), snakeBody, food);
    }
}

TEST_CASE("Placement skips cells that would seal off part of the board",
          "[obstacle][TC-MODE-04]") {
    // Only four cells are eligible. Choosing (1,0) and (0,1) together would
    // seal (0,0) in its corner; any other pair keeps the board connected.
    const Obstacles eligibleCells{{0, 0}, {1, 0}, {0, 1}, {29, 19}};
    std::deque<GridPosition> body{{15, 10}};
    for (int y = 0; y < snake::config::GridRows; ++y) {
        for (int x = 0; x < snake::config::GridColumns; ++x) {
            const GridPosition cell{x, y};
            const bool eligible =
                std::find(eligibleCells.begin(), eligibleCells.end(), cell) !=
                eligibleCells.end();
            if (cell != GridPosition{15, 10} && !eligible) {
                body.push_back(cell);
            }
        }
    }

    for (std::uint32_t seed = 0; seed < 200; ++seed) {
        INFO("seed " << seed);
        std::mt19937 random{seed};
        const Obstacles placed = snake::placeObstacleCells(2, random, body, std::nullopt, 3);
        REQUIRE(placed.size() == 2);
        REQUIRE(snake::keepsBoardConnected(placed));
    }
}

TEST_CASE("Placement never returns a layout that splits the board",
          "[obstacle][TC-MODE-04][regression]") {
    // Regression: the first version re-drew a splitting layout up to 64
    // times and then returned the last one anyway. Here the only eligible
    // cells are row 5, so 30 obstacles can only form a wall that cuts the
    // board in two. Placement must stop at 29 and leave a gap.
    Obstacles row;
    for (int x = 0; x < snake::config::GridColumns; ++x) {
        row.push_back({x, 5});
    }
    const auto body = snakeCoveringAllBut({0, 0}, row);

    for (std::uint32_t seed = 0; seed < 20; ++seed) {
        INFO("seed " << seed);
        std::mt19937 random{seed};
        const Obstacles placed = snake::placeObstacleCells(
            row.size(), random, body, std::nullopt,
            snake::config::ObstacleMinHeadDistance);

        requireValidLayout(placed, body, std::nullopt, row.size() - 1);
    }
}

TEST_CASE("Obstacle mode refuses to start rather than split the board",
          "[obstacle][TC-MODE-04][regression]") {
    // Only 8 cells are eligible, and taking both (1,0) and (0,1) would seal
    // the corner cell (0,0). At most 7 obstacles fit safely, so the session
    // must refuse to start and keep the previous session's obstacles.
    const Obstacles freeCells{{1, 0}, {0, 1}, {10, 15}, {12, 15},
                              {14, 15}, {16, 15}, {18, 15}, {20, 15}};
    const auto crowded = snakeCoveringAllBut({29, 19}, freeCells);

    for (std::uint32_t seed = 0; seed < 20; ++seed) {
        INFO("seed " << seed);
        ModeManager modes({GameMode::Obstacle, Difficulty::Medium}, seed);
        modes.startSession(startingSnake(), GridPosition{25, 5});
        const Obstacles before = modes.obstacles();
        REQUIRE(before.size() == 8);

        REQUIRE_THROWS_AS(modes.startSession(crowded, std::nullopt), std::runtime_error);
        REQUIRE(modes.obstacles() == before);

        // The placement underneath stops at 7 and never seals the corner.
        std::mt19937 random{seed};
        const Obstacles placed = snake::placeObstacleCells(
            8, random, crowded, std::nullopt, snake::config::ObstacleMinHeadDistance);
        requireValidLayout(placed, crowded, std::nullopt, 7);
    }
}

TEST_CASE("Placement keeps the board connected however many obstacles are asked for",
          "[obstacle][TC-MODE-04]") {
    // Dense layouts split the board easily, so this pushes the safety check
    // much harder than the 8 obstacles of a real game.
    const auto snakeBody = startingSnake();
    const GridPosition food{3, 3};

    for (std::uint32_t seed = 0; seed < 200; ++seed) {
        const std::size_t count = 8 + seed % 150;
        INFO("seed " << seed << ", " << count << " obstacles");
        std::mt19937 random{seed};
        const Obstacles placed = snake::placeObstacleCells(
            count, random, snakeBody, food, snake::config::ObstacleMinHeadDistance);

        requireValidLayout(placed, snakeBody, food, count);
    }
}

TEST_CASE("Obstacles respect the head distance near a corner",
          "[obstacle][TC-MODE-04][boundary]") {
    // Head in the top-left corner: the clear zone is clipped by the walls.
    const std::deque<GridPosition> snakeBody{{2, 0}, {1, 0}, {0, 0}};

    for (std::uint32_t seed = 0; seed < 200; ++seed) {
        INFO("seed " << seed);
        ModeManager modes({GameMode::Obstacle, Difficulty::Hard}, seed);
        modes.startSession(snakeBody, GridPosition{0, 19});
        requireValidLayout(modes.obstacles(), snakeBody, GridPosition{0, 19});
    }
}

TEST_CASE("The same seed reproduces the same obstacle layout",
          "[obstacle][TC-MODE-04]") {
    ModeManager first({GameMode::Obstacle, Difficulty::Medium}, 1234u);
    ModeManager second({GameMode::Obstacle, Difficulty::Medium}, 1234u);
    first.startSession(startingSnake(), GridPosition{3, 3});
    second.startSession(startingSnake(), GridPosition{3, 3});

    REQUIRE(first.obstacles() == second.obstacles());

    bool anyDifferent = false;
    for (std::uint32_t seed = 1; seed <= 10 && !anyDifferent; ++seed) {
        ModeManager other({GameMode::Obstacle, Difficulty::Medium}, 1234u + seed);
        other.startSession(startingSnake(), GridPosition{3, 3});
        anyDifferent = other.obstacles() != first.obstacles();
    }
    REQUIRE(anyDifferent);
}

TEST_CASE("Classic and Timed modes place no obstacles",
          "[obstacle][TC-MODE-04][negative]") {
    const GameMode mode = GENERATE(GameMode::Classic, GameMode::Timed);
    CAPTURE(snake::toString(mode));

    ModeManager modes({mode, Difficulty::Medium}, 99u);
    modes.startSession(startingSnake(), GridPosition{20, 4});

    REQUIRE(modes.obstacles().empty());
    REQUIRE_FALSE(modes.isObstacle({20, 5}));
}

TEST_CASE("Placement returns fewer obstacles only when the board is out of room",
          "[obstacle][TC-MODE-04][boundary]") {
    // The snake fills every cell except three far from its head.
    const Obstacles freeCells{{29, 19}, {28, 19}, {29, 18}};
    std::deque<GridPosition> body{{0, 0}};
    for (int y = 0; y < snake::config::GridRows; ++y) {
        for (int x = 0; x < snake::config::GridColumns; ++x) {
            const GridPosition cell{x, y};
            const bool keepFree =
                std::find(freeCells.begin(), freeCells.end(), cell) != freeCells.end();
            if (cell != GridPosition{0, 0} && !keepFree) {
                body.push_back(cell);
            }
        }
    }

    std::mt19937 random{5u};
    Obstacles placed = snake::placeObstacleCells(
        8, random, body, std::nullopt, snake::config::ObstacleMinHeadDistance);

    REQUIRE(placed.size() == 3);
    for (const GridPosition cell : freeCells) {
        REQUIRE(std::find(placed.begin(), placed.end(), cell) != placed.end());
    }

    REQUIRE(snake::placeObstacleCells(0, random, startingSnake(), std::nullopt, 3).empty());
}

TEST_CASE("Layouts that cut the board into pieces are detected",
          "[obstacle][TC-MODE-04]") {
    REQUIRE(snake::keepsBoardConnected({}));
    REQUIRE(snake::keepsBoardConnected({{1, 0}, {1, 1}}));
    // (0,0) is sealed off by two obstacles and the walls.
    REQUIRE_FALSE(snake::keepsBoardConnected({{1, 0}, {0, 1}}));

    Obstacles wall;
    for (int x = 0; x < snake::config::GridColumns; ++x) {
        wall.push_back({x, 5});
    }
    REQUIRE_FALSE(snake::keepsBoardConnected(wall));
}

TEST_CASE("Obstacle cells are fatal only in Obstacle mode",
          "[obstacle][TC-MODE-04]") {
    const Obstacles obstacles{{5, 5}};

    REQUIRE(snake::ObstacleMode{}.isFatal({5, 5}, obstacles));
    REQUIRE_FALSE(snake::ObstacleMode{}.isFatal({6, 5}, obstacles));
    REQUIRE_FALSE(snake::ClassicMode{}.isFatal({5, 5}, obstacles));
    REQUIRE_FALSE(snake::TimedMode{}.isFatal({5, 5}, obstacles));

    ModeManager modes({GameMode::Obstacle, Difficulty::Medium}, 11u);
    modes.startSession(startingSnake(), std::nullopt);
    for (const GridPosition cell : modes.obstacles()) {
        REQUIRE(modes.isObstacle(cell));
        REQUIRE(modes.isFatal(cell));
    }
    REQUIRE_FALSE(modes.isFatal(startingSnake().front()));
}

TEST_CASE("Leaving the grid is fatal in every mode",
          "[modes][boundary]") {
    const GameMode mode = GENERATE(from_range(snake::AllGameModes));
    CAPTURE(snake::toString(mode));
    const auto strategy = snake::makeModeStrategy(mode);

    REQUIRE(strategy->isFatal({-1, 0}, {}));
    REQUIRE(strategy->isFatal({snake::config::GridColumns, 0}, {}));
    REQUIRE(strategy->isFatal({0, -1}, {}));
    REQUIRE(strategy->isFatal({0, snake::config::GridRows}, {}));
    REQUIRE_FALSE(strategy->isFatal({0, 0}, {}));
    REQUIRE_FALSE(strategy->isFatal(
        {snake::config::GridColumns - 1, snake::config::GridRows - 1}, {}));
}

TEST_CASE("Chebyshev distance counts king moves",
          "[obstacle]") {
    REQUIRE(snake::chebyshevDistance({0, 0}, {0, 0}) == 0);
    REQUIRE(snake::chebyshevDistance({0, 0}, {3, 1}) == 3);
    REQUIRE(snake::chebyshevDistance({5, 5}, {3, 8}) == 3);
    REQUIRE(snake::chebyshevDistance({5, 5}, {7, 7}) == 2);
}

// Integration tests for the game wiring the desktop app runs: main-menu
// selection, the selected mode and speed, the Timed countdown with pause and
// restart, obstacles applied to the engine, and scoring per mode. They drive
// GameplayController, the same object AppManager forwards keys and updates to.

#include "snakextreme/GameConfig.hpp"
#include "snakextreme/GameEngine.hpp"
#include "snakextreme/GameMode.hpp"
#include "snakextreme/GameplayController.hpp"
#include "snakextreme/MenuSelection.hpp"
#include "snakextreme/ModeManager.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <vector>

using namespace std::chrono_literals;

using snake::Difficulty;
using snake::Direction;
using snake::GameEngine;
using snake::GameMode;
using snake::GameOverReason;
using snake::GameplayController;
using snake::GridPosition;
using snake::MenuInput;
using snake::Screen;
using snake::SessionConfig;

namespace {

// One fixed update of the app loop: sf::seconds(1.f / 60.f).asMicroseconds().
constexpr std::chrono::microseconds AppStep{16666};

using Millis = std::optional<std::chrono::milliseconds>;

bool sameConfig(const SessionConfig& a, const SessionConfig& b) {
    return a.mode == b.mode && a.difficulty == b.difficulty;
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

// Snake on the left of row 10 heading right; food at `food` (off its path
// unless a test puts it there).
GameEngine leftStart(GridPosition food = {2, 18}) {
    return GameEngine({{2, 10}, {1, 10}, {0, 10}}, Direction::Right, food);
}

bool contains(const std::vector<GridPosition>& cells, GridPosition cell) {
    return std::find(cells.begin(), cells.end(), cell) != cells.end();
}

bool rowClear(const std::vector<GridPosition>& obstacles, int row, int fromX, int toX) {
    for (int x = fromX; x <= toX; ++x) {
        if (contains(obstacles, {x, row})) {
            return false;
        }
    }
    return true;
}

// A game started with `config` whose path along row 10 is free of
// obstacles up to `toX`; searches seeds because obstacles are random.
GameplayController clearRowGame(SessionConfig config, int toX) {
    for (std::uint32_t seed = 0; seed < 200; ++seed) {
        GameplayController game;
        REQUIRE(game.start(config, leftStart(), seed));
        if (rowClear(game.engine().obstacles(), 10, 3, toX)) {
            return game;
        }
    }
    FAIL("no seed with a clear row");
    return {};
}

// Snake covering every cell except `freeCells`, head at (29, 19).
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

} // namespace

// ---------------------------------------------------------------------------
// Main-menu selection (FR-008, FR-009)
// ---------------------------------------------------------------------------

TEST_CASE("The menu starts on Classic and Medium", "[menu]") {
    const SessionConfig selection{};
    REQUIRE(selection.mode == GameMode::Classic);
    REQUIRE(selection.difficulty == Difficulty::Medium);
}

TEST_CASE("Left and Right cycle through the three modes", "[menu]") {
    SessionConfig selection{};
    selection = snake::applyMenuInput(selection, MenuInput::NextMode, Screen::MainMenu);
    REQUIRE(selection.mode == GameMode::Timed);
    selection = snake::applyMenuInput(selection, MenuInput::NextMode, Screen::MainMenu);
    REQUIRE(selection.mode == GameMode::Obstacle);
    selection = snake::applyMenuInput(selection, MenuInput::NextMode, Screen::MainMenu);
    REQUIRE(selection.mode == GameMode::Classic);
    selection = snake::applyMenuInput(selection, MenuInput::PreviousMode, Screen::MainMenu);
    REQUIRE(selection.mode == GameMode::Obstacle);

    // Changing the mode never touches the difficulty.
    REQUIRE(selection.difficulty == Difficulty::Medium);
}

TEST_CASE("Up and Down change the difficulty and stop at Easy and Hard", "[menu]") {
    SessionConfig selection{};
    selection = snake::applyMenuInput(selection, MenuInput::HarderDifficulty, Screen::MainMenu);
    REQUIRE(selection.difficulty == Difficulty::Hard);
    selection = snake::applyMenuInput(selection, MenuInput::HarderDifficulty, Screen::MainMenu);
    REQUIRE(selection.difficulty == Difficulty::Hard);

    selection = snake::applyMenuInput(selection, MenuInput::EasierDifficulty, Screen::MainMenu);
    selection = snake::applyMenuInput(selection, MenuInput::EasierDifficulty, Screen::MainMenu);
    REQUIRE(selection.difficulty == Difficulty::Easy);
    selection = snake::applyMenuInput(selection, MenuInput::EasierDifficulty, Screen::MainMenu);
    REQUIRE(selection.difficulty == Difficulty::Easy);

    REQUIRE(selection.mode == GameMode::Classic);
}

TEST_CASE("Every mode and difficulty pair can be selected from the menu", "[menu]") {
    const GameMode mode = GENERATE(from_range(snake::AllGameModes));
    const Difficulty difficulty = GENERATE(from_range(snake::AllDifficulties));

    SessionConfig selection{};
    for (int i = 0; i < 3 && selection.mode != mode; ++i) {
        selection = snake::applyMenuInput(selection, MenuInput::NextMode, Screen::MainMenu);
    }
    for (int i = 0; i < 2; ++i) {
        selection = snake::applyMenuInput(selection, MenuInput::EasierDifficulty, Screen::MainMenu);
    }
    for (int i = 0; i < 2 && selection.difficulty != difficulty; ++i) {
        selection = snake::applyMenuInput(selection, MenuInput::HarderDifficulty, Screen::MainMenu);
    }

    REQUIRE(selection.mode == mode);
    REQUIRE(selection.difficulty == difficulty);
}

TEST_CASE("The selection cannot change during a game", "[menu]") {
    const Screen screen = GENERATE(Screen::Gameplay, Screen::Paused, Screen::NameEntry,
                                 Screen::Leaderboard, Screen::Settings, Screen::GameOver, Screen::Exiting);
    const MenuInput input = GENERATE(MenuInput::PreviousMode, MenuInput::NextMode,
                                     MenuInput::HarderDifficulty, MenuInput::EasierDifficulty);

    const SessionConfig selection{GameMode::Timed, Difficulty::Easy};
    REQUIRE(sameConfig(snake::applyMenuInput(selection, input, screen), selection));
}

// ---------------------------------------------------------------------------
// Starting a game with the selected settings
// ---------------------------------------------------------------------------

TEST_CASE("Starting applies the selected mode, speed, timer and obstacles",
          "[gameplay][TC-MODE-01][TC-MODE-02]") {
    const GameMode mode = GENERATE(from_range(snake::AllGameModes));
    const Difficulty difficulty = GENERATE(from_range(snake::AllDifficulties));
    CAPTURE(snake::toString(mode), snake::toString(difficulty));

    GameplayController game;
    REQUIRE(game.start({mode, difficulty}));

    REQUIRE(game.hasGame());
    REQUIRE_FALSE(game.isGameOver());
    REQUIRE(sameConfig(game.config(), {mode, difficulty}));
    REQUIRE(game.tickInterval() == expectedInterval(difficulty));
    REQUIRE(game.score() == 0);

    // Fresh engine: the usual three-segment snake at the centre.
    REQUIRE(game.engine().snake().size() == snake::config::InitialSnakeLength);
    REQUIRE(game.engine().snake().front() == GridPosition{15, 10});

    REQUIRE(game.remainingTime() == (mode == GameMode::Timed ? Millis{60s} : Millis{}));

    const auto& obstacles = game.engine().obstacles();
    const auto& body = game.engine().snake();
    REQUIRE(obstacles.size() == (mode == GameMode::Obstacle ? 8u : 0u));
    for (const GridPosition cell : obstacles) {
        REQUIRE(std::find(body.begin(), body.end(), cell) == body.end());
        REQUIRE(cell != game.engine().food());
    }
}

TEST_CASE("The engine gets exactly the obstacles the ModeManager placed",
          "[gameplay][TC-MODE-04]") {
    for (std::uint32_t seed = 0; seed < 20; ++seed) {
        INFO("seed " << seed);
        const SessionConfig config{GameMode::Obstacle, Difficulty::Medium};

        GameplayController game;
        REQUIRE(game.start(config, leftStart(), seed));

        snake::ModeManager reference(config, seed);
        const GameEngine start = leftStart();
        reference.startSession(start.snake(), start.food());

        REQUIRE(game.engine().obstacles() == reference.obstacles());
        REQUIRE(game.engine().obstacles().size() == 8);
    }
}

TEST_CASE("Before any game nothing is running", "[gameplay]") {
    GameplayController game;
    REQUIRE_FALSE(game.hasGame());
    REQUIRE_FALSE(game.isGameOver());
    REQUIRE_FALSE(game.remainingTime().has_value());

    const GridPosition head = game.engine().snake().front();
    game.update(1s, Screen::Gameplay);
    REQUIRE(game.engine().snake().front() == head);
}

// ---------------------------------------------------------------------------
// Selected speed (FR-009)
// ---------------------------------------------------------------------------

TEST_CASE("The snake moves at the selected difficulty's speed",
          "[gameplay][TC-MODE-02]") {
    const GameMode mode = GENERATE(from_range(snake::AllGameModes));
    const Difficulty difficulty = GENERATE(from_range(snake::AllDifficulties));
    CAPTURE(snake::toString(mode), snake::toString(difficulty));

    GameplayController game = clearRowGame({mode, difficulty}, 17);

    // One second of play in 1 ms updates: Easy 5, Medium 8, Hard 14 moves.
    for (int i = 0; i < 1000; ++i) {
        game.update(1ms, Screen::Gameplay);
    }
    const int expectedMoves = 1000 / static_cast<int>(expectedInterval(difficulty).count());
    REQUIRE(game.engine().snake().front() == GridPosition{2 + expectedMoves, 10});
    REQUIRE_FALSE(game.isGameOver());
}

TEST_CASE("The app's 60 Hz step keeps the same speed",
          "[gameplay][TC-MODE-02]") {
    const Difficulty difficulty = GENERATE(from_range(snake::AllDifficulties));
    CAPTURE(snake::toString(difficulty));

    GameplayController game = clearRowGame({GameMode::Classic, difficulty}, 17);

    // 60 app updates = 999,960 us. Moves happen only for whole intervals.
    for (int i = 0; i < 60; ++i) {
        game.update(AppStep, Screen::Gameplay);
    }
    const auto elapsed = AppStep * 60;
    const auto interval = std::chrono::microseconds{expectedInterval(difficulty)};
    const int expectedMoves = static_cast<int>(elapsed / interval);
    REQUIRE(game.engine().snake().front() == GridPosition{2 + expectedMoves, 10});
}

TEST_CASE("Turns use the engine's reversal rule and two-turn queue", "[gameplay]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Medium}, leftStart(), 1u));

    game.changeDirection(Direction::Left); // reversal: ignored
    game.changeDirection(Direction::Up);
    game.changeDirection(Direction::Left);
    for (int i = 0; i < 2; ++i) {
        game.update(120ms, Screen::Gameplay);
    }

    REQUIRE(game.engine().snake().front() == GridPosition{1, 9});
}

// ---------------------------------------------------------------------------
// Timed mode: countdown, pause, expiry, restart (FR-010)
// ---------------------------------------------------------------------------

TEST_CASE("The countdown advances per update, not per move",
          "[gameplay][timed][TC-MODE-03]") {
    const Difficulty difficulty = GENERATE(from_range(snake::AllDifficulties));
    CAPTURE(snake::toString(difficulty));

    GameplayController game;
    REQUIRE(game.start({GameMode::Timed, difficulty}, leftStart(), 1u));

    for (int i = 0; i < 30; ++i) {
        game.update(AppStep, Screen::Gameplay);
    }

    // 30 x 16,666 us = 499,980 us whatever the speed; shown rounded up.
    REQUIRE(game.remainingTime() == Millis{59501ms});
}

TEST_CASE("Pausing freezes movement and the countdown",
          "[gameplay][timed][TC-MODE-03]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Timed, Difficulty::Medium}, leftStart(), 1u));

    game.update(100ms, Screen::Gameplay); // 100 ms into the first 120 ms move
    const GridPosition head = game.engine().snake().front();
    REQUIRE(head == GridPosition{2, 10});
    REQUIRE(game.remainingTime() == Millis{59900ms});

    // Paused (or back on the menu): updates change nothing.
    for (int i = 0; i < 600; ++i) {
        game.update(AppStep, i % 2 == 0 ? Screen::Paused : Screen::MainMenu);
    }
    REQUIRE(game.engine().snake().front() == head);
    REQUIRE(game.remainingTime() == Millis{59900ms});

    // Resuming continues where it stopped: 20 ms completes the first move.
    game.update(20ms, Screen::Gameplay);
    REQUIRE(game.engine().snake().front() == GridPosition{3, 10});
    REQUIRE(game.remainingTime() == Millis{59880ms});
}

TEST_CASE("Time up ends the game and stops all movement",
          "[gameplay][timed][TC-MODE-03]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Timed, Difficulty::Easy},
                       GameEngine({{10, 10}, {9, 10}, {8, 10}}, Direction::Right, {29, 19}),
                       1u));

    // Easy: one move per 200 ms update. Circle a 6 x 6 square to stay alive.
    const Direction turns[] = {Direction::Down, Direction::Left, Direction::Up, Direction::Right};
    int turn = 0;
    for (int update = 1; update <= 299; ++update) {
        game.update(200ms, Screen::Gameplay);
        if (update % 6 == 0) {
            game.changeDirection(turns[turn]);
            turn = (turn + 1) % 4;
        }
    }
    REQUIRE_FALSE(game.isGameOver());
    REQUIRE(game.remainingTime() == Millis{200ms});

    game.update(100ms, Screen::Gameplay); // 59.9 s
    REQUIRE_FALSE(game.isGameOver());
    const GridPosition head = game.engine().snake().front();
    const int score = game.score();

    game.update(100ms, Screen::Gameplay); // 60.0 s
    REQUIRE(game.isGameOver());
    REQUIRE(game.gameOverReason() == GameOverReason::TIME_UP);
    REQUIRE(game.remainingTime() == Millis{0ms});
    REQUIRE(game.engine().snake().front() == head);

    for (int i = 0; i < 100; ++i) {
        game.update(200ms, Screen::Gameplay);
    }
    REQUIRE(game.engine().snake().front() == head);
    REQUIRE(game.score() == score);
    REQUIRE(game.gameOverReason() == GameOverReason::TIME_UP);
}

TEST_CASE("Restarting resets the countdown, score and movement but keeps the high score",
          "[gameplay][timed][TC-MODE-03]") {
    
GameplayController game;

// Food starts directly ahead. Place subsequent food at the
// last free cell so the snake does not score again along its
// straight path before hitting the wall.
const GameEngine initialEngine(
    {{2, 10}, {1, 10}, {0, 10}},
    Direction::Right,
    {3, 10},
    [](const std::vector<GridPosition>& freeCells) {
        return freeCells.back();
    });

REQUIRE(game.start(
    {GameMode::Timed, Difficulty::Medium},
    initialEngine,
    1u));

    game.update(120ms, Screen::Gameplay);
    REQUIRE(game.score() == 10);
    for (int i = 0; i < 100; ++i) {
        game.update(100ms, Screen::Gameplay); // 10 s; the snake hits the wall on the way
    }
    REQUIRE(game.remainingTime() < Millis{60s});

    REQUIRE(game.start({GameMode::Timed, Difficulty::Medium}));

    REQUIRE(game.remainingTime() == Millis{60s});
    REQUIRE(game.score() == 0);
    REQUIRE(game.highScore() == 10);
    REQUIRE_FALSE(game.isGameOver());

    // The movement accumulator starts empty: no move before a full interval.
    const GridPosition head = game.engine().snake().front();
    game.update(119ms, Screen::Gameplay);
    REQUIRE(game.engine().snake().front() == head);
    game.update(1ms, Screen::Gameplay);
    REQUIRE(game.engine().snake().front() != head);
}

TEST_CASE("A game that already ended keeps its countdown", "[gameplay][timed]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Timed, Difficulty::Hard}, leftStart(), 1u));

    for (int i = 0; i < 200 && !game.isGameOver(); ++i) {
        game.update(70ms, Screen::Gameplay); // straight into the right wall
    }
    REQUIRE(game.gameOverReason() == GameOverReason::WALL);

    const auto left = game.remainingTime();
    game.update(5s, Screen::Gameplay);
    REQUIRE(game.remainingTime() == left);
}

// ---------------------------------------------------------------------------
// Obstacles and scoring (FR-011, FR-016)
// ---------------------------------------------------------------------------

TEST_CASE("Running into an obstacle ends the game with OBSTACLE",
          "[gameplay][obstacle][TC-MODE-04]") {
    int collisions = 0;

    for (std::uint32_t seed = 0; seed < 60; ++seed) {
        GameplayController game;
        REQUIRE(game.start({GameMode::Obstacle, Difficulty::Hard}, leftStart(), seed));

        // Only seeds that put an obstacle in the snake's straight path.
        const auto& obstacles = game.engine().obstacles();
        int firstObstacle = -1;
        for (int x = 3; x < snake::config::GridColumns && firstObstacle < 0; ++x) {
            if (contains(obstacles, {x, 10})) {
                firstObstacle = x;
            }
        }
        if (firstObstacle < 0) {
            continue;
        }

        INFO("seed " << seed << ", obstacle at x=" << firstObstacle);
        for (int i = 0; i < 1000 && !game.isGameOver(); ++i) {
            game.update(AppStep, Screen::Gameplay);
        }
        REQUIRE(game.gameOverReason() == GameOverReason::OBSTACLE);
        REQUIRE(game.engine().snake().front() == GridPosition{firstObstacle - 1, 10});
        ++collisions;
    }

    REQUIRE(collisions >= 5);
}

TEST_CASE("Food scores 10 in Classic and Timed and 15 in Obstacle",
          "[gameplay][scoring]") {
    const GameMode mode = GENERATE(from_range(snake::AllGameModes));
    CAPTURE(snake::toString(mode));

    GameplayController game;
    REQUIRE(game.start({mode, Difficulty::Easy}, leftStart({3, 10}), 7u));
    game.update(200ms, Screen::Gameplay);

    const int points = mode == GameMode::Obstacle ? 15 : 10;
    REQUIRE(game.engine().snake().size() == 4);
    REQUIRE(game.score() == points);
    REQUIRE(game.highScore() == points);
}

// ---------------------------------------------------------------------------
// A session that cannot start
// ---------------------------------------------------------------------------

TEST_CASE("A game that cannot start leaves the previous game untouched",
          "[gameplay][failure]") {
    GameplayController game;
    REQUIRE(game.start({GameMode::Classic, Difficulty::Medium}, leftStart({3, 10}), 1u));
    game.update(120ms, Screen::Gameplay);
    REQUIRE(game.score() == 10);
    const auto snakeBefore = game.engine().snake();

    // Only 8 free cells, two of which would seal the corner (0, 0): Obstacle
    // mode cannot place its 8 obstacles safely, so the session refuses.
    const std::vector<GridPosition> freeCells{{1, 0}, {0, 1}, {10, 15}, {12, 15},
                                              {14, 15}, {16, 15}, {18, 15}, {20, 15}};
    GameEngine crowded(snakeCoveringAllBut(freeCells), Direction::Left);

    REQUIRE_FALSE(game.start({GameMode::Obstacle, Difficulty::Hard}, crowded, 3u));
    REQUIRE_FALSE(game.lastError().empty());

    REQUIRE(sameConfig(game.config(), {GameMode::Classic, Difficulty::Medium}));
    REQUIRE(game.engine().snake() == snakeBefore);
    REQUIRE(game.score() == 10);
    REQUIRE(game.hasGame());

    // A normal start works again afterwards and clears the error.
    REQUIRE(game.start({GameMode::Obstacle, Difficulty::Hard}));
    REQUIRE(game.lastError().empty());
}

TEST_CASE("A first game that cannot start leaves no game running",
          "[gameplay][failure]") {
    const std::vector<GridPosition> freeCells{{1, 0}, {0, 1}, {10, 15}, {12, 15},
                                              {14, 15}, {16, 15}, {18, 15}, {20, 15}};
    GameplayController game;
    REQUIRE_FALSE(game.start({GameMode::Obstacle, Difficulty::Medium},
                             GameEngine(snakeCoveringAllBut(freeCells), Direction::Left), 5u));
    REQUIRE_FALSE(game.hasGame());
    REQUIRE_FALSE(game.isGameOver());
}

// ---------------------------------------------------------------------------
// Text shown in the HUD and the window title
// ---------------------------------------------------------------------------

TEST_CASE("The countdown is shown as m:ss, rounded up", "[gameplay][hud]") {
    REQUIRE(snake::formatCountdown(60000ms) == "1:00");
    REQUIRE(snake::formatCountdown(59999ms) == "1:00");
    REQUIRE(snake::formatCountdown(59000ms) == "0:59");
    REQUIRE(snake::formatCountdown(10000ms) == "0:10");
    REQUIRE(snake::formatCountdown(9001ms) == "0:10");
    REQUIRE(snake::formatCountdown(9000ms) == "0:09");
    REQUIRE(snake::formatCountdown(1ms) == "0:01");
    REQUIRE(snake::formatCountdown(0ms) == "0:00");
    REQUIRE(snake::formatCountdown(-5ms) == "0:00");
    REQUIRE(snake::formatCountdown(125000ms) == "2:05");
}

TEST_CASE("Every game-over reason has a message", "[gameplay][hud]") {
    REQUIRE(snake::describeGameOver(GameOverReason::WALL) == "You hit the wall");
    REQUIRE(snake::describeGameOver(GameOverReason::SELF) == "You ran into yourself");
    REQUIRE(snake::describeGameOver(GameOverReason::OBSTACLE) == "You hit an obstacle");
    REQUIRE(snake::describeGameOver(GameOverReason::TIME_UP) == "Time's up");
    REQUIRE(snake::describeGameOver(GameOverReason::BOARD_FULL) == "Board full - you win");
}

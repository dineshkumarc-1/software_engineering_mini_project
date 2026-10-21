# Sprint 1 — Game modes, difficulty, build and CI

Issues **#5, #10, #11, #12, #13** (owner: B V Tejas), PR #37. Built on top of the Catch2 engine work from PR #36, which is now merged into `main`.

**In scope:** the mode, difficulty, build and CI code and its automated tests. **Separate work, not claimed here:** wiring the modes into the running application (#19 screens, #3 movement loop in PR #35) and runtime/demo validation of the game on Windows 10/11.

## #12 — Difficulty speed profiles (FR-009)

| Acceptance criterion | Implementation | Validation |
| --- | --- | --- |
| Easy profile, 200 ms tick | `difficultyProfile(Difficulty::Easy)` in `DifficultyProfile.hpp`, value from `config::EasyMoveIntervalMs` | `static_assert` in the header; TC-MODE-02 tests |
| Medium profile, 120 ms tick | same, `MediumMoveIntervalMs` | same |
| Hard profile, 70 ms tick | same, `HardMoveIntervalMs` | same; integration test counts 5 / 8 / 14 moves in one second |

`ModeManager::tickInterval()` returns the interval for the session's difficulty, independent of mode. The Sprint 2 speed boost (FR-013, issue #14) scales this value; it does not change the profile.

## #11 — Game mode strategy (FR-008, FR-010, FR-011; SAD C7)

| Acceptance criterion | Implementation | Validation |
| --- | --- | --- |
| Polymorphic `IModeStrategy` | `ClassicMode`, `TimedMode`, `ObstacleMode` behind `IModeStrategy` (`ModeStrategy.hpp`), `makeModeStrategy()` factory, `ModeManager` per session | TC-MODE-01 |
| Timed: 60 s countdown, game over at zero | `TimedMode::timeLimit()` = 60 s; `ModeManager::update(dt)`, `remainingTime()` (never negative), `isTimeUp()`; engine `endGame(GameOverReason::TIME_UP)` | TC-MODE-03 unit + integration (running at 59.9 s, `TIME_UP` at 60.0 s) |
| Obstacle: 8 static obstacles with collision | `ObstacleMode::placeObstacles()` → 8 cells; engine `setObstacles()` blocks food and ends the game with `GameOverReason::OBSTACLE` | TC-MODE-04 unit + integration |

Placement rules for obstacles (SRS Addendum §4): free cells only, never on the snake or food, at least 3 cells from the head. The distance is Chebyshev (`max(|dx|, |dy|)`), so the 5×5 square around the head is clear; this satisfies "≥ 3 cells" under either distance reading.

The board also stays connected, so food can always be reached (not required by the SRS, added because a sealed corner with food in it would stall a game). Obstacles are added one at a time in random order, and a cell is skipped if it would cut off any open cell. The layout is therefore connected after every step, so no returned layout can split the board. (The first version re-drew whole layouts and, after 64 failed tries, returned the last one even though it split the board; the regression tests below cover this.)

**Failure handling.** If the mode's 8 obstacles cannot all be placed safely, `ModeManager::startSession()` throws `std::runtime_error` and leaves the previous session unchanged, instead of starting Obstacle mode with fewer obstacles. That needs a nearly full board, so it cannot happen when a new game starts: a fresh 30 × 20 board always has hundreds of eligible cells whose whole 3 × 3 neighbourhood is open, and an obstacle there cannot cut anything off.

### Engine changes (small, additive)

`GameEngine` (C6) stays mode-agnostic. It gained:

- `GameOverReason::OBSTACLE` and `GameOverReason::TIME_UP` (names from SAD §4.3.1).
- `setObstacles(cells)` / `obstacles()` — blocked cells; food never spawns on them, and food already on one is moved.
- `endGame(reason)` — ends a running game for a rule decided outside the engine (the Timed countdown). Ignored if the game is already over.

Self-collision stays in the engine (it knows whether the tail vacates its cell, SAD OI-2). `IModeStrategy::isFatal()` therefore covers walls and mode hazards only, a deliberate narrowing of the SAD §4.3.4 signature.

### Wiring into the application loop

The UI stories (#19 screens, #3 movement loop) own `AppManager`. These are the lines it needs:

```cpp
// On Start, with the player's choice from the Mode & Difficulty screen:
engine_ = GameEngine{};
modes_ = ModeManager{SessionConfig{mode, difficulty}};
modes_.startSession(engine_.snake(), engine_.food()); // throws only on a nearly full board: go back to the menu
engine_.setObstacles(modes_.obstacles());

// In update(step), only while the Gameplay screen is active (so pause freezes the timer):
modes_.update(std::chrono::microseconds{step.asMicroseconds()}); // not asMilliseconds(): 16 ms steps drift
if (modes_.isTimeUp()) {
    engine_.endGame(GameOverReason::TIME_UP);
}
const auto moveInterval = sf::milliseconds(static_cast<sf::Int32>(modes_.tickInterval().count()));
// ...existing accumulator loop that calls engine_.tick() every moveInterval...
```

`tests/ModeEngineIntegrationTests.cpp` drives the two classes exactly this way.

### Note for PR #34 (scoring)

`GameMode` now lives in `snakextreme/GameMode.hpp`. PR #34 declares an identical `snake::GameMode` in `ScoreService.hpp`; when both are merged, replace that declaration with `#include "snakextreme/GameMode.hpp"` (same enumerators, same order). Per-mode points are also available as `ModeManager::foodPoints()`.

## #5 — CMake build targets and Catch2 (NFR-005, NFR-006)

| Acceptance criterion | Implementation | Validation |
| --- | --- | --- |
| Modular targets for engine, modes, persistence, UI, test runner | `snake_engine`, `snake_modes`, `snake_persistence` (placeholder until #15 adds sources), `snake_app_state` + `snake_presentation` (UI), `check` (builds and runs all tests). `snake_add_module()` in `cmake/SnakeModules.cmake` | Configure fails if an SFML-free module links SFML; `layering_core_is_sfml_free` CTest follows includes from core sources and fails on any SFML header |
| Clean build on Windows 10/11 (MSVC / MinGW) | `/W4 /permissive- /utf-8` for MSVC; MinGW uses the GCC flags. SFML 2.5.1 fetch now configures under CMake 4 (`CMAKE_POLICY_VERSION_MINIMUM`), which current runners and Homebrew ship | Windows MSVC and MinGW-w64 builds and tests pass in CI on GitHub's Windows runners, and so does the full MSVC app build (see Validation). Running the game on Windows 10/11 is separate demo work |
| Catch2 v3 test target | `cmake/SnakeCatch2.cmake`: installed Catch2 3.x if found, else pinned v3.8.1 download; headers treated as system headers. `snake_add_catch2_test()` registers each test executable with CTest | `game_engine_tests`, `game_mode_tests` |

Options: `SNAKE_BUILD_APP`, `SNAKE_FETCH_SFML`, `SNAKE_FETCH_CATCH2`, `SNAKE_WARNINGS_AS_ERRORS`. Presets: `headless`, `app` (CMake 3.21+).

Adding a module later is one line, for example:

```cmake
snake_add_module(snake_persistence SOURCES src/persistence/SecureFileStore.cpp)
```

`snake_warnings(<target>)` keeps its name, so branches that already call it still work.

## #10 — GitHub Actions CI (NFR-005, NFR-006, NFR-007)

`.github/workflows/ci.yml` runs on every pull request, every push to `main`, and on demand.

| Job (status check name) | What it does |
| --- | --- |
| Tests (Linux GCC) | Headless configure/build/test, warnings as errors |
| Tests (macOS Clang) | Same, warnings as errors |
| Tests (Windows MSVC) | Same, warnings shown but not fatal yet |
| Tests (Windows MinGW-w64) | Same via MSYS2 UCRT64, warnings not fatal yet |
| App build (Windows MSVC) | Full app with SFML 2.5.1 (static), tests, uploads `SnakeXtreme-windows-x64` (the `.exe`) as a 14-day artifact |
| App build (Linux GCC) | Full app with SFML 2.5.1, tests |

Warnings become fatal on MSVC and MinGW once each has a green run; that tightening belongs with static analysis in #23.

**Making the checks required (repo owner, once).** Settings → Branches → Add branch protection rule for `main` → tick *Require status checks to pass before merging* → select the six job names above. Until then, the checks appear on every PR but do not block merging.

## #13 — Game-mode tests (TC-MODE-01..04)

`tests/GameModeTests.cpp` (unit) and `tests/ModeEngineIntegrationTests.cpp` (modes + engine), one `game_mode_tests` executable, 37 Catch2 test cases. Run one area with a tag, e.g. `game_mode_tests "[TC-MODE-03]"`.

| Test case | Requirement | Covered by | Status |
| --- | --- | --- | --- |
| TC-MODE-01 | FR-008 | Exactly three modes, Classic default; each mode's rules (timer only in Timed, obstacles only in Obstacle); stored names round-trip, bad names rejected | Logic automated. The on-screen part (selection list, HUD showing the mode) waits for the Mode & Difficulty screen (#19) |
| TC-MODE-02 | FR-009, FR-002 | Profiles 200/120/70 ms; all 9 mode × difficulty combinations; snake moves 5/8/14 cells in one simulated second | Automated |
| TC-MODE-03 | FR-010 | 60 s limit; running at 59.9 s, `TIME_UP` at 60.0 s; no drift at the app's 60 Hz step; never negative; zero/negative steps ignored; restart resets; pause freezes; final state kept; untimed modes never expire | Automated |
| TC-MODE-04 | FR-011 | 8 obstacles on valid cells for 500 seeds (plus a corner start); no board split, including two regression cases where a safe full layout does not exist (placement stops short; the session refuses to start and keeps the previous one) and dense layouts of up to 157 obstacles; same seed same layout; no obstacles in Classic/Timed; engine collision → `OBSTACLE`, head never enters; every generated obstacle fatal; food never on an obstacle; invalid cells rejected | Automated |

Timing uses explicit millisecond steps and randomness uses fixed seeds, so every run is repeatable (STP §5 determinism).

## Validation (21 Oct 2026)

### CI on PR #37

[Run 39553515636](https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/39553515636): all 6 jobs passed: Linux GCC, macOS Clang, Windows MSVC and Windows MinGW-w64 tests, plus the full app builds on Windows MSVC and Linux GCC, with the Windows `.exe` artifact. This confirms the Windows build. It does not cover running the game on Windows 10/11 or the NFR-001 frame-rate check.

### Local

Environment: Ubuntu 24.04 container, no Windows machine.

| Check | Result |
| --- | --- |
| Headless build, GCC 13.3, `-Werror` | PASS — 4/4 CTest (app state, engine, modes, layering) |
| Headless build, Clang 18, `-Werror` | PASS — 4/4 |
| GCC with AddressSanitizer + UBSan | PASS — 4/4, no reports |
| MinGW-w64 GCC cross-compile for Windows, `-Werror` | PASS — compiles and links `.exe` tests (not run: no Windows) |
| Full app with fetched SFML 2.5.1, CMake 3.28 and CMake 4.4 | PASS — builds, 4/4 tests; app launches under Xvfb |
| `cmake --preset headless` / build / test (CMake 4.4) | PASS |
| `check` target | PASS |
| Mutation check: 9 deliberate rule breaks (obstacle not fatal, food on obstacle, time-up off by one, head distance, split layouts allowed, 7 obstacles, 50 s timer, Hard 80 ms, food ignored) | All caught by tests or by compile-time checks |
| Layering check with an SFML include planted in a core header | Fails as intended |
| `actionlint` on the workflow | PASS |
| Obstacle-placement fix: the PR's original placement code put back in place | Both regression tests fail, reproducing the split-layout bug |
| Obstacle-placement fix: connectivity check skipped / shortfall ignored / session changed before refusing | Each caught by a regression test |

Not executed here or in CI: running the game on Windows 10/11 (runtime/demo validation, TC-PERF-01) and the modes inside the running application, which is not wired yet (see "Wiring").

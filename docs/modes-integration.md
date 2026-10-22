# Modes and difficulty in the running game

Integration of the merged mode/difficulty backend (PR #37) into the desktop app, built on top of the score HUD (PR #34) and movement controls (PR #35). Owner: B V Tejas. The engine, mode rules and scoring are reused unchanged.

## How it fits together

`AppManager` keeps the window, the screen state and the main-menu selection (`SessionConfig selectedConfig_`, Classic / Medium by default). It forwards keys and fixed-step updates to a `GameplayController`, which holds the engine, the `ModeManager` and the existing `GameSession` score. The controller is SFML-free (`snake_gameplay`), so the integration tests run exactly the code the game runs instead of a test-only copy. `Renderer` only draws a `FrameView` that `AppManager` fills each frame.

## Requested changes

| # | Request | Implementation | Automated tests (`gameplay_tests`) |
| --- | --- | --- | --- |
| 1 | Select mode and difficulty before starting | Main menu: Left/Right (or A/D) cycle Classic → Timed → Obstacle; Up/Down (or W/S) change Easy ↔ Medium ↔ Hard and stop at the ends; Enter starts. Rules in `MenuSelection.hpp` (`applyMenuInput`), which ignores input on any screen except the main menu. The menu shows both rows, the selected values, what each mode does, the snake speed and the controls | Defaults, cycling, clamping, all 9 pairs reachable, no change during gameplay, pause or exit |
| 2 | Connect `ModeManager` on Start | `GameplayController::start`: fresh `GameEngine`, `ModeManager{selected}`, `startSession(engine.snake(), engine.food())`, `engine.setObstacles(...)`, score reset (high score kept), movement accumulator cleared. The new game is set up on the side and swapped in only when complete. If the session cannot start, the app stays on the menu and shows the error there and in the title | Settings, speed, timer and obstacles for all 9 pairs; obstacles equal to a `ModeManager` with the same seed; a failed start keeps the previous game and its score |
| 3 | Use the difficulty speeds | The hard-coded 0.120 s interval is replaced by `ModeManager::tickInterval()`. The 60 Hz fixed-step loop and the movement accumulator are unchanged | One second of play gives 5 / 8 / 14 moves in every mode; the app's 16,666 µs step gives the same speeds |
| 4 | Timed mode | `ModeManager::update(std::chrono::microseconds{step.asMicroseconds()})` once per gameplay update, not per move. When `isTimeUp()`: `engine.endGame(GameOverReason::TIME_UP)`, no further movement. Updates outside the Gameplay screen are ignored, so pause freezes movement and the countdown; Start / Enter-to-play-again creates a new `ModeManager`, so the countdown restarts at 1:00 | Countdown per update; pause and menu freeze both and resume continues exactly; expiry at 60.0 s with no further movement; restart resets time, score and accumulator but keeps the high score; a game that already ended keeps its time |
| 5 | Presentation | Obstacles are brown blocks with a dark inner edge (food stays a red circle, the snake green squares). Top-left panel: score and high score. Top-right panel: mode · difficulty and, in Timed, the countdown (orange from 0:10, red at 0:00). Panels fade when part of the board is underneath. Game over shows the reason (wall, yourself, obstacle, time up) and final score, placed in the half of the board away from the snake's head. The window title mirrors this as a fallback if the font cannot be loaded | Reason texts and countdown format (m:ss, rounded up) |
| 6 | Scoring and controls | `gameSession_.tick(engine, selectedMode)`; points unchanged (Classic/Timed 10, Obstacle 15). WASD/arrows, the engine's reversal rule and two-turn queue, pause/resume and the font loading code are unchanged. Enter after a game over plays again with the same settings; M returns to the menu | 10 / 10 / 15 per food through the real tick; turns go through the engine's queue and reversal rule |
| 7 | Validation | Below | |

## Validation (22 Oct 2026)

### Automated

All 7 CTest suites pass: the 6 existing ones and the new `gameplay_tests`, which has 22 test cases. Builds: GCC 13.3 and Clang 18 headless with `-Werror`, GCC with AddressSanitizer + UBSan, the full app with SFML 2.5.1 and `-Werror`, and a MinGW-w64 cross-compile of the full app and tests for Windows with `-Werror` (built, not run). SFML's headers are now treated as system headers when SFML is downloaded, so warnings only cover project code.

### Runtime checks of the real game (Linux only)

Platform actually tested: **Linux**, in an Ubuntu 24.04 container, running the real `SnakeXtreme` binary (GCC 13.3, SFML 2.5.1 static) on an Xvfb virtual display with Mesa software OpenGL. Input was real keyboard events sent through XTEST (xdotool). State was read from the window title and from screenshots; a small autopilot steered towards food and obstacles. **Not run on Windows 10/11 or macOS**; there, CI only builds and runs the headless tests.

59 checks, all passing:

| Check | Result |
| --- | --- |
| Menu defaults, arrow keys and WASD change the selection, Esc on the menu exits with code 0 | Pass |
| WASD steers; a reversal is ignored | Pass |
| All 9 mode × difficulty pairs start with the right title/HUD, obstacle count and countdown, and M returns to the menu with the selection kept | Pass |
| Measured move interval (average over 8 moves) | Classic 200.5 / 118.7 / 71.1 ms, Timed 199.6 / 120.9 / 69.2 ms, Obstacle 199.1 / 118.4 / 71.1 ms (Easy / Medium / Hard) |
| Obstacles visible in Obstacle mode, none otherwise | 8, 8, 7 (the 7 had one obstacle under a HUD corner, which the checker skips) |
| Classic food +10 twice, then self-collision ("You ran into yourself"); Enter plays again (score 0, high kept); wall ("You hit the wall") | Pass |
| Obstacle food +15 twice, then steering into an obstacle ("You hit an obstacle") | Pass |
| Timed: pause for 3 s freezes the countdown and the board, resume continues, +10 per food, game over "Time's up" at 0:00, nothing moves afterwards, Enter restarts at 1:00 with score 0 and the high score kept | Pass |

Not exercised at runtime: the menu error after a failed start (it cannot happen on a fresh board; covered by tests), focus-loss pause (unchanged code), BOARD_FULL.

### Note for recording gameplay (Task #27)

The text (menu, HUD, game-over reason) needs `assets/fonts/DejaVuSans.ttf` next to the executable. A local build copies it there. The current Windows CI artifact contains only the `.exe`; draft PR #38 adds the assets folder to it.

# SnakeXtreme — Team 16

[![CI](https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/workflows/ci.yml/badge.svg)](https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/workflows/ci.yml)

Offline C++17/SFML snake game. The submitted requirements, design and test-plan PDFs are in the repository root.

## Current implementation: issue #4

Presentation/window scaffold only:
- A fixed-size **900 × 600** SFML window, with rendering capped at 60 FPS.
- Separate `snake_app_state` (SFML-free) and `snake_presentation` libraries.
- Initial application flow: MainMenu → Gameplay placeholder ↔ Paused → MainMenu.
- Window-close events and Esc from the menu terminate cleanly.
- Focus loss pauses the gameplay placeholder; resume is explicit.
- Fixed-step application update seam, separate from drawing and event handling.
- No external fonts, textures, music or other assets required.

**Not implemented:** snake movement/collisions, functional menus/name entry, modes, scores, persistence, audio playback, or the full screen stack. These belong to later issues. The grid is a preview, not a playable game. A 60 FPS cap is not proof of the NFR-001 minimum on Windows hardware.

## Build on Windows (Visual Studio 2022)

Install Git, CMake 3.20+ and Visual Studio's **Desktop development with C++** workload. In a Developer PowerShell:

```powershell
git clone https://github.com/dineshkumarc-1/software_engineering_mini_project.git
cd software_engineering_mini_project
cmake -S . -B build -DSNAKE_FETCH_SFML=ON -DBUILD_SHARED_LIBS=OFF
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\Release\SnakeXtreme.exe
```

The first configure downloads SFML **2.5.1** and needs internet access. Graphics, window, system and audio are enabled; network remains disabled. Alternatively install a compatible **SFML 2.x** development package and configure without `SNAKE_FETCH_SFML`, setting `SFML_DIR` to its CMake configuration directory if needed. Prebuilt SFML must match compiler, architecture and runtime.

## Linux build (development/testing only)

With a compiler, CMake and the X11/OpenGL/FreeType/udev plus OpenAL/FLAC/Ogg/Vorbis development dependencies installed:

```sh
cmake -S . -B build -DSNAKE_FETCH_SFML=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/SnakeXtreme
```

Linux is a stretch platform, not a replacement for required Windows 10/11 validation.

## Headless state tests (no SFML download or display needed)

```sh
cmake -S . -B build-headless -DSNAKE_BUILD_APP=OFF
cmake --build build-headless
ctest --test-dir build-headless --output-on-failure
```

Engine and game-mode tests use **Catch2 v3**. An installed Catch2 3.x is used if CMake finds one; otherwise v3.8.1 is downloaded on first configure (`-DSNAKE_FETCH_CATCH2=OFF` forbids the download). `cmake --build <dir> --target check` builds every test and runs the whole suite. The issue #4 navigation checks keep their small dependency-free harness.

With CMake 3.21+ the same builds are available as presets (output in `build/<preset>`):

```sh
cmake --preset headless && cmake --build --preset headless && ctest --preset headless
cmake --preset app      && cmake --build --preset app      && ctest --preset app
```

CI runs these checks on every pull request and push to `main` (Linux GCC, macOS Clang, Windows MSVC and MinGW-w64, plus full app builds on Windows and Linux). See [docs/modes-build-ci.md](docs/modes-build-ci.md).

## Game modes and difficulty (issues #11, #12)

`snake_modes` (SAD C7, SFML-free) provides the three mode strategies and the difficulty speeds. Values come from `GameConfig.hpp` (SRS Addendum §4).

| Mode | Time limit | Obstacles | Points per food |
| --- | --- | --- | --- |
| Classic (default) | none | none | 10 |
| Timed | 60 s countdown, game over (`TIME_UP`) at zero | none | 10 |
| Obstacle | none | 8 static cells, game over (`OBSTACLE`) on contact | 15 |

| Difficulty | Move interval |
| --- | --- |
| Easy | 200 ms |
| Medium (default) | 120 ms |
| Hard | 70 ms |

The main menu selects the mode and difficulty before a game starts, and the HUD shows them next to the score, the high score and the Timed countdown. See [docs/modes-integration.md](docs/modes-integration.md).

## Controls

| Screen | Key | Action |
| --- | --- | --- |
| Main menu | Up / Down (or W / S), Enter / click | Select New Game, Leaderboard, Settings or Exit |
| Name Entry | Type / Backspace, Enter, Esc | Edit (max 15), validate / continue, cancel |
| Mode selection | Left / Right (or A / D) | Change mode: Classic, Timed, Obstacle |
| Mode selection | Up / Down (or W / S), Enter | Change difficulty; start |
| Settings | Up / Down, Left / Right, Enter / click | Select row, adjust value, capture key / activate |
| Settings | Drag Music / SFX slider | Preview that channel live (0–100) |
| Settings | F5 | Play SFX test tone at the current SFX volume |
| Settings | Save & back / Cancel (Esc) | Persist and apply / discard edits and restore audio |
| Gameplay | Remapped primary keys plus WASD | Steer the snake |
| Gameplay | Esc | Pause game and audio |
| Game over | Enter | Play again with the same settings |
| Paused | Esc / Enter | Resume game and audio |
| Gameplay / Paused / Game over | M | Return to the menu |
| Any screen | Window close button | Exit |

Mode and difficulty can only change before a session starts. The window title
mirrors the current state and settings as a text fallback. Leaderboard persistence
is still pending integration (#15/#17/#21); the menu explicitly labels it pending.

### Audio and Settings (#20 / C5)

A short original, code-generated musical loop and preview/food/game-over tones
play through SFML Audio. No external audio assets are downloaded. Music and SFX
have separate gains; slider edits preview immediately. Save persists the values;
Cancel restores the previously saved levels. The background loop pauses with the
game. No-device or buffer failure uses silent fallback and a visible Settings
notice; preferences still work. Human listening on Windows is still required for
TC-SET-02 acceptance. A null-output test checks routing, not what a user hears.

On Linux with OpenAL Soft: `ALSOFT_DRIVERS=null ./build/audio_backend_smoke`
checks the real SFML backend without a speaker or display. It is an optional app
build executable, not part of display/audio-independent CTest. Normal playback
must not use the `null` driver.

Windows packages require `openal32.dll` next to `SnakeXtreme.exe`; CMake copies
the matching SFML 2.5.1 bundled DLL. Keep the entire CI artifact together,
including `assets/` and `THIRD_PARTY_NOTICES.md`. See the OpenAL notice for source
and licence information. The Linux/macOS system OpenAL runtime is not bundled.

## Structure

```text
include/snakextreme/AppState.hpp       SFML-free navigation model
src/app/AppState.cpp                  State transitions
include/snakextreme/AppManager.hpp     Window and application lifecycle
src/presentation/AppManager.cpp        Events, fixed-step update seam, shutdown
src/presentation/Renderer.cpp          Menu, board, HUD and pause/game-over screens
src/main.cpp                          Entry point and exception boundary
tests/AppStateTests.cpp               Headless navigation checks
docs/issue-4.md                       Scope, traceability and validation
include/snakextreme/GameMode.hpp       Mode/difficulty enums and stored names
include/snakextreme/DifficultyProfile.hpp  Easy/Medium/Hard move intervals
include/snakextreme/ModeStrategy.hpp   IModeStrategy: Classic, Timed, Obstacle
include/snakextreme/ModeManager.hpp    Per-session mode state, countdown, obstacles
src/modes/                            Mode strategies and obstacle placement
tests/GameModeTests.cpp               TC-MODE-01..04 unit tests (Catch2)
tests/ModeEngineIntegrationTests.cpp  Modes wired to the engine (Catch2)
cmake/                                Module/test helpers, Catch2 setup, layering check
CMakePresets.json                     headless and app build presets
.github/workflows/ci.yml              CI pipeline
docs/modes-build-ci.md                Issues #5, #10-#13: scope, traceability, validation
include/snakextreme/MenuSelection.hpp  Main-menu mode/difficulty choice
include/snakextreme/GameplayController.hpp  One game: engine + ModeManager + score
src/gameplay/                         Game wiring used by AppManager (SFML-free)
tests/GameplayControllerTests.cpp     Menu and game-wiring integration tests (Catch2)
docs/modes-integration.md             Modes and difficulty in the running game
```

Future engine integration should expose plain C++ game state; `Renderer` translates that state into SFML drawing. No SFML headers or types should be added to engine or persistence modules. Application's 60 Hz update seam is **not** the snake's movement interval (Easy 200 ms / Medium 120 ms / Hard 70 ms).

## Core Engine Test Coverage — TC-CORE-01–08

The headless engine tests validate core game logic independently of SFML
rendering and real keyboard input. Run them with:

```sh
cmake --build build-headless
ctest --test-dir build-headless --output-on-failure
```

| Test case | Requirement | Coverage status | Evidence / remaining work |
| --- | --- | --- | --- |
| TC-CORE-01 | Initial game state and startup | Partially covered; integration pending | Engine initialization is tested. First rendered frame, 10 application launches, and the <= 2000 ms startup target require application-level integration checks. |
| TC-CORE-02 | Difficulty-based movement intervals | Partially covered; timing integration pending | Required intervals are Easy 200 ms, Medium 120 ms, and Hard 70 ms. Verify configured values and measure actual tick timing with a fake clock and system-level timestamps. |
| TC-CORE-03 | Arrow-key and WASD controls | Partially covered; keyboard integration pending | Engine direction changes and queued turns are unit-tested. Actual arrow-key/WASD event handling requires integration testing. |
| TC-CORE-04 | Reject 180-degree reversals | Covered by engine unit tests | Direct and queued reversal rejection are tested. |
| TC-CORE-05 | Wall collision at all four boundaries | Covered by engine unit tests | Left, right, top, and bottom boundary collisions verify game-over state and the WALL reason. |
| TC-CORE-06 | Self-collision and tail-vacate behavior | Covered by engine unit tests | Self-collision and moving into a tail cell that vacates on the same tick are tested. |
| TC-CORE-07 | Food placement and respawning | Partially covered by engine unit tests | Tests cover 1,000 independent respawn trials, free-cell placement, the one-free-cell case, and a full board. Statistical uniformity is not established by these checks. |
| TC-CORE-08 | Food consumption and snake growth | Covered by engine unit tests | Tests verify one-segment growth after eating and ten consecutive consumptions in one game, from length 3 to length 13. |

### Integration checks still required

- Verify initial rendering and startup latency across 10 launches.
- Verify actual movement intervals using a fake clock and 50 system timestamps per required difficulty, applying the specified +/-10% tolerance.
- Verify arrow-key and WASD input through the running application.
- Record integration-test results separately from headless unit-test results.

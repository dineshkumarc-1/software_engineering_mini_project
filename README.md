# SnakeXtreme — Team 16

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

The first configure downloads SFML **2.5.1** and needs internet access. Audio/network modules are deliberately disabled for this window-only story. Alternatively install a compatible **SFML 2.x** development package and configure without `SNAKE_FETCH_SFML`, setting `SFML_DIR` to its CMake configuration directory if needed. Prebuilt SFML must match compiler, architecture and runtime.

## Linux build (development/testing only)

With a compiler, CMake and the X11/OpenGL/FreeType/udev development dependencies installed:

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

The small tests use explicit checks (including in Release builds), not `assert`. No test framework is downloaded; Catch2 integration remains with the build/test-environment story.

## Controls for this scaffold

| Screen | Key | Action |
| --- | --- | --- |
| Main menu | Enter | Open grid preview |
| Main menu | Esc | Exit |
| Gameplay placeholder | Esc | Pause |
| Paused | Esc / Enter | Resume |
| Gameplay / Paused | M | Return to menu |
| Any screen | Window close button | Exit |

The title bar shows the current screen and shortcuts. The menu's triangle and pause bars are deliberately font-free placeholders, not final UI.

## Structure

```text
include/snakextreme/AppState.hpp       SFML-free navigation model
src/app/AppState.cpp                  State transitions
include/snakextreme/AppManager.hpp     Window and application lifecycle
src/presentation/AppManager.cpp        Events, fixed-step update seam, shutdown
src/presentation/Renderer.cpp          Placeholder screen rendering
src/main.cpp                          Entry point and exception boundary
tests/AppStateTests.cpp               Headless navigation checks
docs/issue-4.md                       Scope, traceability and validation
```

Future engine integration should expose plain C++ game state; `Renderer` translates that state into SFML drawing. No SFML headers or types should be added to engine or persistence modules. Application's 60 Hz update seam is **not** the snake's movement interval (Easy 200 ms / Medium 120 ms / Hard 70 ms).

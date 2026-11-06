# Start-up and new-game measurements (#22): profiling only

Author: B V TEJAS. AI-assisted. **This is measurement instrumentation, not a fix and not acceptance.** It changes no start-up order or behaviour. Everything below is opt-in through `--perf-probe`. Without that option, no marks are taken and no file is written.

## Two different "2-second" questions

| Question | Requirement | Measured by |
| --- | --- | --- |
| How long from **confirming a new game** until the grid and snake are shown? | **FR-001 / TC-CORE-01** (STP): confirm mode/difficulty, log the first frame with the 30×20 grid and snake; **≤ 2000 ms in all 10 runs**, Classic/Medium, reference hardware | `new_games` (below) |
| How long from **starting the program** until the main menu is shown? | Tracked in #22 as "cold launch < 2 s". I found no STP test case for it; the owner should confirm where this target comes from and what it is checked against | `first_frame_us`, `startup_phases_us` and `collect_launch.py` |

Opening the audio device happens at program start-up, before the menu. It is not part of the new-game interval.

## What the probe records

All times are steady-clock microseconds since `main` started, the same clock as `first_frame_us`.

- **`startup_phases_us`:** one mark just after each step finishes.
  - `audio_objects_constructed`: the audio member exists. In the current code this is where the OpenAL device opens.
  - `window_created`: the window and the first OpenGL context exist.
  - `renderer_ready`: the renderer and its font are loaded.
  - `settings_ready`: settings are loaded, or regenerated and saved.
  - `audio_started` or `audio_unavailable`: music is playing, or the silent fallback is active.

  `analyze_performance.py` and `collect_launch.py` also report `startup_step_us`, the time since the previous mark on a timeline that includes `first_frame`. A step also contains anything else that ran since the previous mark.
- **`new_games`:** one sample per new game.
  - `trigger`: `mode_selection`, or `restart` from Game Over.
  - `confirmed_us`: when SFML handed the confirming key or click to the game.
  - `first_grid_us`: when `display()` returned for the first Gameplay frame (the grid and the snake). It is `null` if no Gameplay frame followed.

  The analyzer reports the distribution of `first_grid_us − confirmed_us` and how many exceed 2000 ms. "Display returned" is not the time light reaches the screen. The confirming event's time does not include the OS queue before SFML.

## TC-CORE-01 procedure on Windows (10 runs)

Use the EXE built from the commit under test. Record its source SHA, size and SHA-256.

For each run *n* = 1…10:

1. Start the game:

   ```
   SnakeXtreme.exe --perf-probe C:\path\NEW\core01-<n>.json
   ```
2. Choose New Game, enter a name, and select **Classic / Medium**.
3. Press **Enter** to confirm.
4. When the grid and snake appear, play briefly, then quit normally so the report is written: Esc pauses, M returns to the menu, and Esc on the menu exits.

Afterwards:

```powershell
python tools\analyze_performance.py (Get-ChildItem C:\path\NEW\core01-*.json).FullName
```

PowerShell expands the file list; Python does not expand `*` itself on Windows.

From each run, keep `new_game_timing.confirm_to_first_grid_us` and `over_2000ms`. The STP's pass condition is that all 10 runs are at most 2000 ms, with the snake at 3 segments in the centre of the grid heading right; check the snake by eye or from a screenshot. Report every run, including failed or abandoned ones.

## Start-up timeline procedure

Run:

```
python tools\collect_launch.py SnakeXtreme.exe NEW-dir --runs 5 --source-snapshot <SHA>
```

Use **normal audio**: do not set `ALSOFT_DRIVERS`. Each run in `launch-summary.json` carries `startup_phases_us` and `startup_step_us`.

To get a "first launch after boot" run, reboot, wait about 2 minutes, then use `--runs 1`. The collector does not clear caches, so record the cache condition you established yourself.

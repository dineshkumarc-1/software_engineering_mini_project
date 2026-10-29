# Power-ups (issue #14)

C8 PowerUpManager and the Speed, Multiplier and Shrink effects (FR-012 to FR-015), plus the business rule that only one effect is active at a time (SRS 6.5). Design: SAD 3.4, 4.1 and 4.3.4 (IPowerUpManager), ADR-002 (tick interval = difficulty interval × boost factor), ADR-003 (Strategy) and ADR-005 (seeded random numbers in tests). Values: SRS Addendum v1.1 section 4. Owner: B V Tejas.

## Rules

| Rule | Value | Where |
| --- | --- | --- |
| Spawn | A power-up of a random type appears on a random free cell (not snake, food or obstacle) 8–15 s after the previous one was due; the first one 8–15 s after the start. At most one on the board | FR-012, Addendum |
| Lifetime | An uncollected power-up disappears after 5 s. If one were still on the board when the next is due, it would be moved to the new cell | Issue #14, Addendum ("relocated") |
| Food | New food never lands on the power-up's cell, unless it is the only free cell | FR-012 (the power-up keeps an unoccupied cell) |
| Speed | Movement interval × 0.6 for 5 s: Easy 200 → 120 ms, Medium 120 → 72 ms, Hard 70 → 42 ms (exact fraction 3/5) | FR-013 |
| Multiplier | Food points × 2 for 10 s: Classic and Timed 20, Obstacle 30 | FR-014 |
| Shrink | The tail loses one segment at once, never below 3 segments; nothing runs afterwards | FR-015 |
| One effect | Collecting a power-up first ends the running effect, then applies the new one (Shrink too). Collecting the same type again restarts its time | SRS 6.5 |
| Pause | Effect time, power-up lifetime and spawn schedule only advance during play, so the pause screen and the menu freeze them | FR-024, TC-UI-03 |
| New game | No power-up, no effect, normal speed and no multiplier | |

## Design

```text
include/snakextreme/PowerUp.hpp         PowerUpType, TickScale, PowerUp, ActiveEffect, PowerUpSchedule,
                                        IPowerUpTarget, IPowerUpEffect + SpeedEffect, MultiplierEffect,
                                        ShrinkEffect (Strategy), HUD text helpers
include/snakextreme/PowerUpManager.hpp  Spawn schedule, pick-up, single active effect
src/powerups/                           snake_powerups library (SFML-free, layering check included)
```

- **PowerUpManager** follows the SAD's IPowerUpManager. `update(elapsed, board, target)` and `onHeadMoved(head, target)` also take the board to place power-ups on and the target the effects change, so the manager keeps no references. `active()` and `onGrid()` are as in the SAD. `place()` puts a power-up on a given cell for tests.
- **Effects** act only through `IPowerUpTarget`, which has three operations: the tick scale, the score multiplier and shrink. The engine and the score service do not know which power-ups exist.
- **Engine hooks** (additive): `GameEngine::shrink(segments, minimumLength)` and `setFoodExclusions()` / `foodExclusions()`.
- **Score**: `ScoreService::setMultiplier()` / `multiplier()` (C9 `setMultiplier` in the SAD), forwarded by `GameSession`. `reset()` also clears the multiplier.
- **GameplayController** owns the manager and the tick scale. Each update runs the Timed countdown, then the moves, each followed by the pick-up check, then the power-up timers. `tickInterval()` is the difficulty interval × the scale. Test-only additions: `start(config, engine, seed, PowerUpSchedule)` and `placePowerUp()`.
- **Renderer / AppManager**: each type is a diamond with its own colour and sign (blue », gold x2, violet −), so power-ups differ by shape from the round food and the square snake and obstacles. A power-up blinks during its last 1.5 s. The running effect is shown at the top centre as name, seconds left and an emptying bar, e.g. "Speed boost 4.9s". It is hidden once the game is over and is also shown in the window title.

## Test traceability (STP)

| Test case | Unit (`tests/PowerUpTests.cpp`) | System (`tests/PowerUpGameplayTests.cpp`, the GameplayController the app runs) |
| --- | --- | --- |
| TC-PWR-01 spawn interval | First power-up after 8–15 s (25 seeds). 10 simulated minutes × 3 seeds × 1 ms and 60 Hz steps: every gap 8–15 s, free cell, one at a time, gone after exactly 5 s. Only-free-cell and no-free-cell boards | 10 minutes of real play × 2 seeds with the snake moving: gaps 8–15 s, the power-up never shares a cell with snake, food or obstacle |
| TC-PWR-02 Speed 5 s | Interval 3/5 for 5.000 s, then 1/1. HUD text 5.0s → 4.9s → 0.1s | Medium: moves 72 ms apart for 5.0 s (± 1 tick), then 120 ms. Tick interval read every update. Easy/Medium/Hard → 120/72/42 ms |
| TC-PWR-03 ×2 for 10 s | Food at 1 s, 9.9 s and 10.1 s: +20, +20, +10 | Food eaten 0.96 s, 9.96 s and 10.08 s after pick-up: +20, +20, +10 |
| TC-PWR-04 Shrink | Lengths 6, 4, 3 → 5, 3, 3; consumed, nothing running | Same through a real move onto the power-up |
| TC-PWR-05 one effect | Speed, then Multiplier after 2 s: Speed reverted at that moment, no late revert or speed change in the next 10 s, Speed works again | Same in play with the snake moving |

Further tests cover the engine and score hooks, Shrink ending an effect, re-collecting the same type, the 5 s lifetime, relocation, schedule validation, pause, game over, restart, the food exclusion, a normal `start()` getting a power-up within 15 s, and placement validation. The Timed time-up test in `tests/GameplayControllerTests.cpp` now turns random power-ups off, because it steers in a square at one move per update and a Speed power-up there would break that.

## Validation (28 Oct 2026)

### Automated

All 8 CTest suites pass. The new `powerup_tests` suite has 41 test cases. Builds:

- GCC 13.3 and Clang 18, headless, with `-Werror`
- GCC with AddressSanitizer + UBSan (Debug)
- the full app with SFML 2.5.1 and `-Werror`
- a MinGW-w64 cross-compile of the app and tests for Windows with `-Werror` (built, not run)

cppcheck 2.13 reports nothing in the new code. Its remaining findings in older files are left for issue #23.

Merge check against the open pull requests #42, #44, #45, #46 and #48: each merges without conflicts. Merged with #46, the app builds with `-Werror` and all 12 suites pass. Merged with #48, all 11 suites pass.

### Runtime checks of the real game (Linux only)

Run on **Linux** in an Ubuntu 24.04 container: the real `SnakeXtreme` binary (GCC 13.3, SFML 2.5.1 static) on an Xvfb virtual display. Input was real keyboard events through XTEST (xdotool). State was read from screenshots and the window title. An autopilot played Classic / Medium for 150 s and steered to every power-up it saw. 16 of 16 checks passed:

| Check | Result |
| --- | --- |
| Power-ups keep appearing | 13 in 150 s, all three types, every gap between appearances 8.6–12.9 s; 12 were collected |
| Speed: title and HUD show "Speed boost 5.0s" right after pick-up | Pass |
| Speed: measured move interval during the boost (6 pick-ups, 29 moves each) | 71.5–72.4 ms |
| Speed: the boost ends after ~5 s, then normal speed | 5.07–5.09 s (the observer adds about one move of latency), then 119.0–120.1 ms |
| Multiplier: title and HUD show "Score x2 10.0s"; pausing for 3 s freezes it | Pass (10.0s before and during the pause) |
| Food with ×2 / without | +20 each time (3 foods) / +10 each time (19 foods) |
| Shrink (5 pick-ups) | 24 → 23, 24 → 23, 23 → 22, 22 → 21, 21 → 20 |

Not covered at runtime:

- Windows 10/11 and macOS. There, CI builds the code and runs the headless tests.
- The Hard and Easy boosts, the 3-segment minimum and the 5 s lifetime of an uncollected power-up. The unit and system tests above cover them.

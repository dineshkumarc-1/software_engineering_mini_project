# Issue #19 — first implementation slice

## Branch dependency

This work is based on Bharath's PR #42 head:
`22c1e86d15c5652bfa130ff4115f7e07ec14879a`.
It reuses `isValidPlayerName` and `MaxPlayerNameLength`. It does not modify
Bharath's validator. Merge/update that dependency before integrating this slice.
Do not apply the patch directly to Sprint 1 main without the dependency.

## Implemented

- Main Menu: New Game, Leaderboard, Settings, Exit; keyboard selection and
  mouse confirmation of the four menu buttons.
- New Game -> Name Entry -> Mode/Difficulty -> Gameplay.
- SFML TextEntered handling, Backspace, 15-character input cap, inline errors,
  rejection of invalid/non-ASCII input, blank-name rejection and trimmed submission.
- Name validation reuses the shared validator. UI-side blank/trim handling
  compensates for its current whitespace behavior without changing its contract.
- Back navigation and explicit game-over state, restart and return to menu.
- Existing movement, scoring, modes and pause logic retained.
- SFML-free navigation and name-entry checks, including 10,000 input events.

## Not implemented / not claimed complete

- Leaderboard persistence, display and score submission (#15/#17/#21).
- Settings controls, persistence, audio and themes (#20).
- Power-ups and their pause behavior (#14).
- Fixed-capacity input buffer compliance assessment (SEC-006), sanitizers,
  full STP execution, performance and Windows runtime validation.
- Mouse confirmation outside the four main-menu buttons.

Settings and Leaderboard are explicitly labelled pending integration, not shown
as functioning features. Do not close #19 or mark Sprint 2 complete yet.

## Validation

- Linux headless configure/build with `SNAKE_WARNINGS_AS_ERRORS=ON`.
- All 8 registered CTest suites passed; these are not all STP cases.
- GCC C++17 syntax-only compile of AppManager.cpp, Renderer.cpp and main.cpp
  against SFML 2.5.1 headers with -Wall -Wextra -Wpedantic -Werror.
- No graphical application link/run or Windows runtime test performed here.

## Next manual checks

Run the full app: select every menu destination with mouse and keyboard; type
blank, invalid, 1-, 15- and 16-character names; cancel and go back; start all nine
mode/difficulty combinations; pause/resume; collide and restart; close from each
screen. Record actual results and defects before claiming UI acceptance.

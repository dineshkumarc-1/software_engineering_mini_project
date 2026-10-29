# Issue #19 — first implementation slice

## Branch dependency

This work is based on Bharath's PR #42 head:
`517c169433b4139d047acaead806314dd524de03` (updated integration).
The three validator source/test files are synchronized unchanged from that
upstream snapshot; PR #42 remains a separate draft dependency. The UI reuses
`isAllowedPlayerNameCharacter`, `isValidPlayerName`, `sanitizePlayerName` and
`MaxPlayerNameLength`. Merge that dependency before integrating this slice.
Do not apply the patch directly to Sprint 1 main without the dependency.

## Implemented

- Main Menu: New Game, Leaderboard, Settings, Exit; keyboard selection and
  mouse confirmation of the four menu buttons.
- New Game -> Name Entry -> Mode/Difficulty -> Gameplay.
- SFML TextEntered handling, Backspace, 15-character input cap, inline errors,
  rejection of invalid/non-ASCII input, blank-name rejection and trimmed submission.
- Character entry uses the shared character helper (spaces allowed while typing).
  Submission calls the shared complete-name validator, then the shared sanitizer
  for normalization only after validation succeeds. No UI-side blank/trim rules.
- Back navigation and explicit game-over state, restart and return to menu.
- Existing movement, scoring, modes and pause logic retained.
- SFML-free navigation and name-entry checks, including 10,000 input events.

## Not implemented / not claimed complete

- Leaderboard persistence, display and score submission (#15/#17/#21).
- Settings controls, persistence, audio and themes (#20).
- Power-ups and their pause behavior (#14).
- Fixed-capacity input buffer compliance assessment (SEC-006), full-app sanitizer
  review, full STP execution, performance and Windows runtime validation.
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

## Shared-validator integration verification (2026-10-27 UTC)

Source dependency: Bharath PR #42 revision
`517c169433b4139d047acaead806314dd524de03`; copied validator header/source/tests
match that revision exactly. No edits to the shared validator implementation.
AI-assisted UI adapter changes, not a merge or final approval of PR #42.

Environment: Amazon Linux 2023, GCC 11.5.0, CMake 4.4.4, Catch2 v3.8.1.
Commands: `cmake --preset headless -DSNAKE_WARNINGS_AS_ERRORS=ON`,
`cmake --build --preset headless -j4`, `ctest --preset headless --output-on-failure`,
`./build/headless/app_state_tests`, `./build/headless/input_validator_tests --reporter compact`,
`ctest --test-dir build/headless -R '^gameplay_tests$' --repeat until-fail:20 --output-on-failure`.

Results: 8/8 suites pass on the UI branch; 186 navigation/name-entry checks pass;
shared validator has 9 cases / 34 assertions; gameplay passes 20 consecutive runs.
Raw logs: `evidence/validator-ui-{ctest,navigation,unit,repeat,sanitizers}.txt`.
The UI model/navigation harness also passes GCC ASan+UBSan with leak detection:
compile AppStateTests.cpp, AppState.cpp and InputValidator.cpp with
`-fsanitize=address,undefined -fno-omit-frame-pointer`, then run with
`ASAN_OPTIONS=detect_leaks=1`. Agent runtime-library path setup is environment-only.
Presentation still passes strict syntax checking with SFML 2.5.1 headers.

Automated UI-model integration is verified; actual SFML desktop/manual checks
remain pending. Pre-serialization validation in #15 is not implemented here:
there is no leaderboard persistence serializer on this base. Keep #16/#19 and
PR #42 open/draft pending that integration and final acceptance. Shared sanitizer
resource-bound review (SEC-006) is not signed off by these bounded UI tests.

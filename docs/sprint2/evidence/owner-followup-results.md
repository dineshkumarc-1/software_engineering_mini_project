# Owner follow-up evidence — issues #19, #20, #22

AI-assisted owner implementation by C Dinesh Kumar. Tested working tree based
on published Settings `b7d11d423b96c42d55b54029c5afe7ff9631d6c3`, with new
mouse controls, hit-test fix and recovered/revalidated probes. The tested
source files are identified by `owner-tested-source-sha256.txt`; this avoids
claiming the parent SHA includes unpublished changes. Match these hashes to
the eventual PR head to identify the tested source. Documentation-only files
and generated reports are not executable source inputs.

## Actual execution environment

Amazon Linux 2023 x64; GCC 11.5.0; CMake 4.4.4; Python 3.13;
Catch2 3.8.1; nlohmann/json 3.11.3; SFML 2.5.1; local OpenAL Soft 1.23.1;
Xvfb/xdotool with software GL and null audio. Agent dependency installation and
local `/data/audio-deps`/sanitizer runtime paths are environment workarounds,
not requirements to change the project's pinned dependencies.

## Executed results

| Check | Actual result | Raw evidence |
| --- | --- | --- |
| Headless warnings-as-errors build and CTest | 11/11 suites pass | owner-headless-ctest.txt |
| Full desktop warnings-as-errors build and CTest | 11/11 suites pass | owner-app-ctest.txt |
| Navigation/name/pointer boundary model | 262 checks pass | owner-navigation-unit.txt |
| Probe model/queue/report regression | 11 cases, 81 assertions pass | owner-probe-unit.txt |
| Python report analyzer | 10 tests pass | owner-python-tests.txt |
| Real desktop navigation/Settings | 23 checks pass | owner-navigation-desktop.txt |
| Existing real desktop Settings/recovery | 17 checks pass | owner-settings-desktop.txt |
| Actual SFML/OpenAL routing (null device) | 7 checks pass | owner-audio-backend.txt |
| ASan+UBSan with leak detection | navigation 262; probes 81; Settings 191 pass | owner-sanitizers.txt |
| Gameplay repeat until failure | 20 consecutive suite runs pass | owner-gameplay-repeat.txt |
| Real probe tooling smoke | 100 synthetic requests; three launch traces; no name sentinel leak/drop/invalid samples | owner-probe-desktop.txt, probes/*.json |
| Invalid probe CLI without report | exit 2 before window creation | owner-probe-cli.txt |

Runtime navigation checks include invalid/blank/15-character name input; Back;
three confirmation-click launch (plus typing); mouse selection/start of all nine
mode/difficulty combinations; pause/resume/menu; actual wall collision/restart;
leaderboard placeholder/back; Settings gap clicks; duplicate-key rejection;
independent slider drag outside the track with clamp; mouse Save; clean exit.
Existing recovery checks include actual missing/corrupt-config startup,
regeneration, Cancel/relaunch, themes/remaps, and unavailable-audio fallback.

## Reproduction commands

```
cmake --preset headless -DSNAKE_WARNINGS_AS_ERRORS=ON
cmake --build --preset headless -j4
ctest --preset headless --output-on-failure
cmake --preset app -DSNAKE_WARNINGS_AS_ERRORS=ON -DCMAKE_PREFIX_PATH=/data/audio-deps
cmake --build --preset app -j4
ctest --preset app --output-on-failure
python -m unittest discover -s tests -p test_performance_tools.py -v
LD_LIBRARY_PATH=/data/audio-deps/lib python tools/desktop_navigation_smoke.py build/app/SnakeXtreme
LD_LIBRARY_PATH=/data/audio-deps/lib python tools/desktop_settings_smoke.py build/app/SnakeXtreme
LD_LIBRARY_PATH=/data/audio-deps/lib python tools/desktop_probe_smoke.py build/app/SnakeXtreme NEW-output-dir
```

Sanitizers: Debug headless configure with `-fsanitize=address,undefined
-fno-omit-frame-pointer` and sanitizer linker flags; run app_state_tests,
performance_probe_tests and settings_tests with `ASAN_OPTIONS=detect_leaks=1`.
Runtime library path setup may be necessary on this agent, not standard CI.

## Explicitly not accepted / still pending

No actual target Windows hardware run, human audible check, independent
first-time-user walkthrough, verified cold-cache launch or full end-to-end OS
input latency measurement was performed. The short Linux traces are diagnostic
smoke only, not 3x5-minute power-up-active Hard/Obstacle performance acceptance.
C13/logging, leaderboard and power-ups remain separate integration dependencies.
CI on the new published PR head must be checked separately. Keep issues open,
PR draft, and release/demo/freeze unapproved. No thresholds are waived.

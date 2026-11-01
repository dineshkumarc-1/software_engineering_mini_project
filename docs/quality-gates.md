# Quality gates: sanitizers, static analysis, banned APIs, portability (issue #23)

Covers SEC-005 (TC-SEC-05, step 2), NFR-005 (TC-MNT-01) and NFR-006 (TC-PORT-01), plus the STP exit criterion "no sanitiser or cppcheck findings". Owner: B V Tejas.

## Gates

| Gate | What fails it | Where it runs | Run it yourself |
| --- | --- | --- | --- |
| Clang ASan + UBSan | Any AddressSanitizer or UndefinedBehaviorSanitizer report in any test. Leak detection is on, and errors abort (`-fno-sanitize-recover=all`, `halt_on_error=1`) | CI job **Sanitizers (Clang ASan + UBSan)**, Ubuntu 24.04, Clang 18, Debug | See below |
| cppcheck | Any finding of severity warning, style, performance or portability in `src/`, `include/` or `tests/` | CI job **Static analysis (cppcheck)**, Ubuntu 24.04, cppcheck 2.13 | `cmake -P cmake/RunCppcheck.cmake` |
| Banned APIs (SEC-005) | Any call in `src/` or `include/` to:<br>• commands or code loading: `system`, `popen`, `exec*`, `spawn*`, `dlopen`, `LoadLibrary*`, `ShellExecute*`, `CreateProcess*`, `WinExec`<br>• unbounded C strings: `strcpy`, `strcat`, `wcscpy`, `wcscat`, `gets`<br>• any printf-family function, so no format string can ever come from a file | CTest `security_banned_api_scan`, in every CI job | `cmake -DSNAKE_SOURCE_DIR=. -P cmake/CheckBannedApis.cmake` |
| Scanner self-test | The scan misses one of the 12 planted calls in `tests/fixtures/banned-api/bad`, or reports anything in the look-alike code in `tests/fixtures/banned-api/clean` (comments, strings, `fgets`, `std::filesystem`, `ecosystem()`, …) | CTest `security_banned_api_scan_finds_planted_calls`, `security_banned_api_scan_ignores_lookalikes` | `ctest -R banned_api` |
| Layering (TC-MNT-01) | An SFML include in engine, modes, power-up, scoring, gameplay, persistence or app-state code. Since every presentation header includes SFML, this also catches a core file that includes presentation code | CTest `layering_core_is_sfml_free`, in every CI job | `ctest -R layering` |
| Warnings as errors | Any compiler warning in project code | Linux GCC and macOS Clang test jobs | `-DSNAKE_WARNINGS_AS_ERRORS=ON` |

Sanitizer build, as in CI:

```sh
cmake -S . -B build-san -DSNAKE_BUILD_APP=OFF -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DSNAKE_WARNINGS_AS_ERRORS=ON \
  "-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer" \
  "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined"
cmake --build build-san --parallel
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 ctest --test-dir build-san --output-on-failure
```

On Ubuntu, the Clang sanitizer runtime comes from the `libclang-rt-18-dev` package.

**Exceptions** must be justified on the line itself:
- cppcheck: `// cppcheck-suppress <id>` on the line before, with a comment saying why. The only ones today are in the two `constexpr` parse functions in `GameMode.hpp`, because `std::find_if` is not `constexpr` before C++20.
- Banned APIs: `banned-api-ok: <reason>` on the same line. There are none today.

## Findings fixed for this issue

cppcheck 2.13 found 6 issues, all fixed:
- `GameEngine::setObstacles` now takes its cells by const reference.
- The constexpr loops in `GameMode.hpp` are suppressed, with the reason above.
- Three test findings: a by-value parameter, a constant `REQUIRE` turned into `static_assert`, and a reference made const.

The power-up code in #49, on which this branch is stacked, was already clean.

On 28 Oct, the open pull requests would have added these findings when merged. In the integrated candidate #53 (`dbf08b5`), all of them are fixed or justified, and cppcheck reports 0 findings:

| PR | Finding | In candidate #53 |
| --- | --- | --- |
| #42 / #44 / #45 / #46 / #48 / #50 | `src/validation/InputValidator.cpp:21` and `:49`: useStlAlgorithm (use `std::all_of` / `std::none_of` and `std::copy_if`) | Resolved |
| #46 | `include/snakextreme/UiHitTest.hpp:43`: useStlAlgorithm (`std::find_if`); `src/diagnostics/PerformanceProbe.cpp:53` and `:80`: local variable `input` shadows a function | Resolved |
| #48 / #50 | `src/persistence/LeaderboardRepository.cpp:303`: useStlAlgorithm (`std::count_if`) | Resolved |

No open pull request has a banned-API hit, and neither has the candidate.

## Final candidate: `main` at `7c6733d` (30 Oct 2026)

The candidate Dinesh pinned for final QA: `main` `7c6733d95a4fa61a3af83c413a27d1ad2f40d44b`, tree `e83825372214b6c11296c59d47301cf236a5f716`. Every gate below ran unchanged on the unmodified tree, on Ubuntu 24.04 with GCC 13.3.0, Clang 18.1.3 and cppcheck 2.13.0, tests as a normal user. Evidence: [`docs/qa/evidence/issue-18-23-final-candidate-7c6733d.txt`](qa/evidence/issue-18-23-final-candidate-7c6733d.txt).

| Gate | Result |
| --- | --- |
| Clang ASan + UBSan, CI settings, all tests | **21/21 pass, 0 sanitizer reports** |
| GCC Release, `-Werror`, game and all tests | **21/21 pass** |
| cppcheck (`cmake -P cmake/RunCppcheck.cmake`) | **0 findings** |
| Banned-API scan and its self-tests | **0 hits in 47 files**; finds 12/12 planted calls, nothing in the look-alikes |
| Layering (TC-MNT-01) | **Pass**: 35 SFML-free files, no SFML include |
| Relevant runtime checks (game on Xvfb) | Leaderboard 69/69 (including D7's six columns), refused save 6/6, security UI 23/23, power-ups 17/17, #53 smokes 29/23/22. Harness start-up failures and one 22/23 security-UI run are recorded in the evidence; each passed on rerun |
| **Line coverage of C6–C11** (STP exit criterion ≥ 80 %) | **Met.** C6 GameEngine 99.0 %, C7 ModeManager 96.7 %, C8 PowerUpManager 97.3 %, C9 ScoreService 96.0 %, C10 InputValidator 97.3 %, C11 LeaderboardRepository 92.5 %; C6–C11 together 96.4 % (832 of 863 lines). GCC `--coverage`, all 21 headless suites, gcovr 8.8. Report: [`docs/qa/evidence/coverage-C6-C11-7c6733d.txt`](qa/evidence/coverage-C6-C11-7c6733d.txt). The SFML presentation code is not measured |
| TC-PORT-01, real Windows 10 and Windows 11 | **Pending owner review.** Windows 11: clean build, 20/20 CTest and a manual run are recorded for `3fcc586`. Windows 10: see the table below |

### After PR #60: `main` at `3fcc586` (30 Oct 2026, Linux)

`main` `3fcc586d740d2b9b461193e482d6861ca8f905e5` (tree `41b54378c177d7d1e01ff17f2bf7b22663789ec6`) includes #60. It differs from `7c6733d` in product code only in `src/audio/AudioModel.cpp`, the Game Over alert. The gates that this change can affect were rerun unchanged on the clean tree, with the same tools and settings as above and tests as a normal user. Evidence: [`docs/qa/evidence/gates-3fcc586-after-pr60.txt`](qa/evidence/gates-3fcc586-after-pr60.txt). These results are kept **separate**: they are not results for `7c6733d`, and they say nothing about any Windows EXE.

| Gate | Result on `3fcc586` |
| --- | --- |
| Clang ASan + UBSan + leak detection, all tests | **21/21 pass, 0 sanitizer reports**; `audio_model_tests` 161 assertions in 5 cases |
| GCC Release, `-Werror`, game and all tests | **21/21 pass** |
| cppcheck | **0 findings** |
| Banned-API scan and its self-tests | **0 hits in 47 files**; finds the 12 planted calls |
| Layering (TC-MNT-01) | **Pass**: 35 SFML-free files |
| Line coverage of C6–C11 | **Not rerun.** No C6–C11 file changed, so the `7c6733d` result above still describes this code |
| Real game on Xvfb, Windows, manual and listening checks | **Not run here** |

The earlier results below are kept as they were recorded.

## Results after integration (29 Oct 2026)

Rerun on the integrated candidate **#53 at `dbf08b5`**, which contains this branch together with every other Sprint 2 branch. #52's test update (test, tool and report files only, no product code) was added on top. Ubuntu 24.04. Evidence: [docs/qa/evidence/issue-18-candidate-ctest-sanitizers.txt](qa/evidence/issue-18-candidate-ctest-sanitizers.txt) in #52.

| Check | Result |
| --- | --- |
| Clang 18 ASan + UBSan, CI settings, game and all tests, run as a normal user | **21/21 CTest tests pass, 0 sanitizer reports.** Six known-defect tests (D1–D5, #52) are tagged `[!shouldfail]` and still fail as expected; they are acceptance blockers, not passes |
| GCC 13.3 Release, `-Werror`, game and all tests, normal user | 21/21 pass |
| The real game built with Clang ASan + UBSan (SFML included) on Xvfb | **0 sanitizer reports and no leak at exit** in every run: power-ups 120 s (16/16 checks), 5 games plus corrupt boards and a refused save (`tools/leaderboard_ui_check.py`), the name-entry and privacy checks, and #53's leaderboard, navigation and settings smoke tests |
| cppcheck 2.13 on `src/`, `include/` and `tests/` | **0 findings** |
| Banned-API scan | **0 hits in 46 files.** The self-test finds 12/12 planted calls and reports nothing in the look-alike code |
| Layering | Pass |

### With the persistence fix (#48 at `31602b5`, then `0d516f3`)

The same checks were run again with Bharath's persistence fix merged into the candidate, plus #52's promoted tests. Evidence: `docs/qa/evidence/issue-18-run3-fix-verification.txt` in #52.

| Check | Result |
| --- | --- |
| Clang 18 ASan + UBSan and GCC 13.3, game and all tests, normal user | 21/21 pass, 0 sanitizer reports. The real ASan game also reports nothing in every runtime check |
| cppcheck 2.13 | **0 findings with `0d516f3`.** `31602b5` had one (`src/persistence/SecureFileStore.cpp:445`, knownConditionTrueFalse); Bharath's `0d516f3` removed it. All 21 suites were re-run on `0d516f3` with the same results |
| Banned-API scan | 0 hits in 46 files |

## Results on this branch (28 Oct 2026)

All checks below were run on this branch, which is main plus #49 plus this change.

| Check | Result |
| --- | --- |
| Clang 18 ASan + UBSan, CI settings, all 11 CTest tests | 11/11 pass, 0 sanitizer reports. The instrumentation is active: a planted heap overflow is reported |
| GCC 13.3 ASan + UBSan | 11/11 pass, 0 reports |
| The real game built with Clang ASan + UBSan (SFML included), played for 150 s on Xvfb by the power-up autopilot | 0 reports, including leaks at exit. See [Real game under sanitizers](#real-game-under-sanitizers) |
| cppcheck 2.13 on `src/`, `include/` and `tests/` | 0 findings |
| Banned-API scan | 0 hits in 26 files. The self-test finds 12/12 planted calls and reports nothing in the look-alike code |
| Layering | Pass |

### Real game under sanitizers

The real `SnakeXtreme` binary was built with Clang 18 and `-fsanitize=address,undefined -fno-sanitize-recover=all`, with SFML 2.5.1 compiled with the same flags. It ran on Linux on an Xvfb display with:

```text
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:log_path=<dir>/asan
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1:log_path=<dir>/ubsan
```

The power-up autopilot played Classic / Medium for 150.8 s: 1,393 moves, 13 power-ups seen, and Speed, Multiplier and Shrink collected. Along the way it paused and resumed, then left through the menu (M, then Esc).

**Result:** the process exited normally with code 0, with **no AddressSanitizer, UndefinedBehaviorSanitizer or LeakSanitizer report** and no sanitizer log file written. All 13 gameplay checks of the run passed.

## TC-MNT-01 Module separation (NFR-005)

Step 1, the dependency check: `layering_core_is_sfml_free` follows every include of the SFML-free modules and finds 0 SFML includes. In CMake, `snake_assert_core_is_sfml_free()` stops the configure step if a core module links SFML. Dependencies only point down:

```text
snake_presentation (SFML) ──► snake_gameplay ──► snake_modes, snake_powerups ──► snake_engine
                          ──► snake_app_state     snake_scoring
```

Nothing below the presentation layer links SFML or includes a presentation header. In #53 the layering check also covers `persistence`, `audio` and `diagnostics`. `src/validation` is not on its list yet, though it has no SFML include today; adding it is a one-word change.

Step 2, the review of modules against unit-test targets:

| Module | Test suite(s) | Status |
| --- | --- | --- |
| Engine (`snake_engine`, C6) | `game_engine_tests`, `game_mode_tests` (integration), `powerup_tests` (shrink and food hooks) | Pass |
| Modes (`snake_modes`, C7) | `game_mode_tests` | Pass |
| Power-ups (`snake_powerups`, C8) | `powerup_tests` | Pass (#49) |
| Scoring (`snake_scoring`, C9) | `score_service`, `game_session`, `powerup_tests` (multiplier) | Pass |
| UI logic (`snake_app_state` C1 navigation, `MenuSelection`, `snake_gameplay`) | `app_state_flow`, `gameplay_tests`, `powerup_tests` | Pass |
| Persistence (`snake_persistence`, C11–C13) | Placeholder on main. In candidate #53: `secure_file_store_tests`, `leaderboard_repository_tests`, `leaderboard_model_tests`, `security_corpus_tests`, `secure_write_fault_tests` | Pass in #53, except the six known-defect tests (D1–D5), which fail as expected |
| Settings (`snake_settings`) and name validation (`snake_validation`) | In candidate #53: `settings_tests`, `settings_corpus_tests`, `input_validator_tests` | Pass in #53 |
| Audio model and performance probe (`snake_audio_model`, `snake_probe`) | In candidate #53: `audio_model_tests`, `performance_probe_tests` | Pass in #53 |

## Issue #23 — final accepted evidence and disposition

**Status: Closed / Completed.** Dinesh explicitly accepted the final criterion
table and closed Issue #23 after reviewing PR #75 at
`52416fc62d9e44ed978fc0892d862c9d0a2fa900`. This decision covers Issue #23's
acceptance criteria only. It is not final QA sign-off, release approval, PR #69
acceptance, macOS crash-injection acceptance, or closure of Issue #24.

| Criterion | Tested candidate / build | Result | Evidence and limitations |
| --- | --- | --- | --- |
| Linux ASan/UBSan | `3fcc586d740d2b9b461193e482d6861ca8f905e5`, Clang 18.1.3 | 21/21 tests; zero reported sanitizer findings. CI sanitizer job passed on `26a1f86e05d470fd52500fd0e91de372b3b48f05`. | Preserve the distinct candidate and CI identities. |
| Linux static analysis | `3fcc586d740d2b9b461193e482d6861ca8f905e5`, cppcheck 2.13 | Zero findings under the approved gate. | This is the approved-gate result, not a claim that every possible analyzer or configuration was run. |
| Windows 11 build and execution | `3fcc586d740d2b9b461193e482d6861ca8f905e5`; MSVC executable | Clean build; 20/20 tests; actual runtime evidence. The initial leaderboard-after-restart mismatch passed a controlled retest. EXE size 1,896,960 bytes; SHA-256 `3496FD8C9E8ED95FB886BEF272DBAF9D62A181237E3FDB39474FF7627C3E42B5`. | Preserve the initial mismatch. Its root cause remains unproven. |
| Windows 10 build and execution | `09bbf96e14d9b8f037d46b4b55e07d32aeacf26c`; separate Windows 10 executable | Raw build evidence; CTest 20/20; audio backend 7/7; live-process transitions. EXE size 1,906,688 bytes; SHA-256 `780B5AB208B7FD6DF91916E4E386848E179FE4B71CBA27BC4BCEE60207D2E946`. | Owner accepted source applicability because only 11 documentation files differ from `3fcc586` and no build/test inputs differ. This is not binary equivalence. The raw archive does not independently establish warnings-as-errors or a clean-checkout build; no build exit code or client-area screenshot was recorded, dependencies were reused, and the EXE hash could not be recomputed from the archive. |

The older `7c6733d` portability checklist below is historical and must not
override the later accepted evidence. Keep all source and executable identities
separate. Do not publish the original Windows 10 archive further until personal
paths are redacted in a separate sharing copy; preserve the original. The
supplemental Windows 11 GCC evidence is not the primary accepted MSVC evidence.
Issue #24's native crash-injection gap remains a separate pending decision.


## TC-PORT-01 Windows 10 / 11 build and run (NFR-006)

The checklist and machine-status table in this section describe the earlier `7c6733d` candidate and are retained for historical traceability. They do not override the later accepted Issue #23 evidence above.

**Needs:**
- Visual Studio 2022 with the "Desktop development with C++" workload
- Git
- CMake 3.20 or newer (included with Visual Studio)

Steps, in a "Developer PowerShell for VS 2022":

```powershell
git clone https://github.com/dineshkumarc-1/software_engineering_mini_project.git
cd software_engineering_mini_project
cmake -S . -B build -DSNAKE_FETCH_SFML=ON -DBUILD_SHARED_LIBS=OFF
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
.\build\Release\SnakeXtreme.exe
```

Checklist (record each result in the table below). Build the final candidate, **`main` at `7c6733d95a4fa61a3af83c413a27d1ad2f40d44b`**: after cloning, run `git checkout 7c6733d95a4fa61a3af83c413a27d1ad2f40d44b`. CI built a Windows package from the same commit (Actions run 40994581558, `SnakeXtreme.exe` 1,902,080 bytes, SHA-256 `8442dcb9a8ea16d347af70b8e4af9b739399633f1d9e053885964a2a170b14fd`). Running that package is useful too, but TC-PORT-01 needs a clean build from source on each PC. If the candidate changes, record the new SHA and never relabel older results.

**PR #60 (30 Oct 2026).** `main` is now at `3fcc586d740d2b9b461193e482d6861ca8f905e5`, which includes #60. #60 changes the Game Over sound in `src/audio/AudioModel.cpp`, so a Windows EXE built from `7c6733d` does not contain it. Build whichever SHA Dinesh pins as the final candidate: `7c6733d` for the baseline, or `3fcc586` or later if #60 is included. Record that SHA in the table. A result recorded for one SHA never counts for another.

1. Configure and build finish with **0 errors**.
2. `ctest` passes **every** test.
3. The game window opens with the menu text visible, so the bundled font loaded.
4. Play one game in each mode:
   - Classic: eat food, +10.
   - Timed: the countdown runs down from 1:00.
   - Obstacle: hitting a brown block shows "You hit an obstacle".
5. Collect at least one power-up. The HUD shows its name and seconds left.
6. Esc pauses and resumes. M returns to the menu. Esc on the menu exits.
7. Closing the window with its X also exits cleanly.
8. Leaderboard and settings: after a game, the main menu's Leaderboard lists the score. Changing a setting survives a restart.

Record for each machine:
- **Tested SHA:** the full commit (`git rev-parse HEAD`).
- **Build artifact:** `build\Release\SnakeXtreme.exe`, its size and its SHA-256 (`Get-FileHash .\build\Release\SnakeXtreme.exe`).
- **OS version:** edition, version and OS build from `winver`, for example "Windows 11 Pro 23H2, build 22631.4317".
- **Compiler and build:** Visual Studio and MSVC version (`cl` prints it), CMake version, the configure line and build type.
- **Build and run results:** errors and warnings of the build, the CTest summary line, and steps 3–8.
- **Evidence:** the configure, build and CTest output as text, a `winver` screenshot, and screenshots of the menu, one game and the leaderboard. Attach them to issue #23 or add them under `docs/qa/evidence/`.

| Machine | Status | Tested SHA | Build artifact (size, SHA-256) | OS version (`winver`) | Compiler / build | Build | CTest | Steps 3–8 | Evidence | Tester | Date |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Windows 10 PC | **Built and run on `09bbf96` (PR #60 source). Source-identical to merged `3fcc586` outside `docs/`** (`git diff --name-only 09bbf96 3fcc586` lists only 11 `docs/` files; no build or test input differs); the owner accepted this for the Windows 10 criterion on #23. **This is not binary equivalence:** the Windows 10 EXE keeps its own hash and was not built from the merge commit. The earlier report on `7c6733d` (EXE `FE3AA1316D634611F3D690665C5D802696BE80DA5124C34B2475D05391CEF1F3`) stays recorded separately in the register and its raw logs are still not reviewed | `09bbf96e14d9b8f037d46b4b55e07d32aeacf26c` (tree `241976240f9e9bf7065efc92ebb244ba92daf6d7`; clean status reported, not in a raw file) | 1,906,688 bytes, `780B5AB208B7FD6DF91916E4E386848E179FE4B71CBA27BC4BCEE60207D2E946` | Windows 10 Pro 10.0.19045 (from the configure log) | MSVC 19.44.35229.0 (VS 2022 Build Tools), CMake 4.4.4, Release (`app` preset) | Raw log has 0 compiler or linker warnings; exit code not recorded; every target and the EXE were produced | **20/20** (raw `03-ctest.txt`; the 21st suite is Linux-only) | Live run on the built EXE: wall collision to Game Over, restart, second collision, SFX 0, pause/resume (title transitions), audio backend 7/7, collision alert heard by one listener; no client-area screenshot | `PR60-WINDOWS-AUDIO-EVIDENCE-20261030.zip` (SHA-256 `d9efe8991cd774a908d64d85fd766ffaed3a385ddbb72e3323c21a364cf3c985`), attached to #23, not committed | Dinesh (harness run), reviewed by Tejas | 30 Oct 2026 |
| Windows 11 PC | **Built and run on `3fcc586`; owner review Pending.** MSVC (the documented configuration) is the primary result. A separate GCC/MinGW build is supplemental (custom MSYS2 toolchain, raw logs private, partial manual walkthrough). Original manual checklist 7 of 8: the leaderboard-after-restart check failed once, then passed in a controlled retest with the same EXE and working directory; the cause of the first mismatch is not proven | `3fcc586d740d2b9b461193e482d6861ca8f905e5` (tree `41b54378c177d7d1e01ff17f2bf7b22663789ec6`, clean checkout) | **MSVC:** 1,896,960 bytes, `3496FD8C9E8ED95FB886BEF272DBAF9D62A181237E3FDB39474FF7627C3E42B5`. **GCC (supplemental):** 1,495,228 bytes, `c043cd941e55888a13e06cbce5c049729dcb27118e1997f6d941b5e4287e992c` | Windows 11 Home Single Language 10.0.26200 | MSVC 19.42 (VS 2022), CMake, Release, `-DSNAKE_FETCH_SFML=ON -DBUILD_SHARED_LIBS=OFF` | Exit 0 | **20/20** (inventory names match; the 21st suite is Linux-only) | MSVC manual checklist 7 of 8 plus the controlled persistence retest (pass) | [Windows 11 record](sprint2/evidence/windows11-issue23-24-preflight.md), [MSVC logs](sprint2/evidence/windows11-msvc-logs-20261101/run-summary.txt), [manual results](sprint2/evidence/manual-gameplay-results.txt), [persistence retest](sprint2/evidence/msvc-persistence-retest-20261101/persistence-retest-summary.md) | Bharath (@DuoCoder986) | 31 Oct and 1 Nov 2026 |

For this historical `7c6733d` table, Pending, Not tested, and Reported describe the evidence recorded for that candidate at that time. The later Issue #23 disposition is Closed / Completed, as recorded above. Windows Server CI is not a substitute for native manual evidence. Issue #23 closure does not grant final QA sign-off or release approval.

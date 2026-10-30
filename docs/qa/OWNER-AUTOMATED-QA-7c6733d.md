# Owner automated QA — candidate 7c6733d

**Execution date:** 30 October 2026 (UTC)  
**Status:** Linux automated verification passed; final QA sign-off **Pending**.  
**Execution:** Automated agent on Dinesh's request; not a human or Windows test claim.

## Exact build identity

- Unmodified game source: `7c6733d95a4fa61a3af83c413a27d1ad2f40d44b`; tree `e83825372214b6c11296c59d47301cf236a5f716`.
- Local fresh build: Amazon Linux 2023 x86_64, GCC 11.5.0; CMake/CTest 4.4.4; Catch2 3.8.1. Cached third-party sources were used for Catch2, JSON and SFML 2.5.1. Desktop configure uses the app preset's `BUILD_SHARED_LIBS=OFF` setting and warnings as errors.
- Sanitizer build: Clang 15.0.7, ASan + UBSan, no recovery, leak detection enabled. Coverage: separate GCC Debug `--coverage -O0` build, gcovr 8.8.
- Fresh local Linux binary SHA-256: `3c0d35c41d63e0596ab61980e42efc5aa31a44219f23274124403a2ff93e6666`; size 1240040 bytes. This is **not** the Windows EXE.
- Enhanced leaderboard harness from Tejas PR #58 head `48bec8c5da9b198984fe91f2946b2b8607c5910c` ran against the unchanged game source above. No product or assertion edits were made.
- Official candidate [CI run](https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/40994581558) and [Windows artifact](https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/40994581558/artifacts/12950672120) remain the handoff references. Windows EXE SHA-256 `8442dcb9a8ea16d347af70b8e4af9b739399633f1d9e053885964a2a170b14fd`, size 1,902,080 bytes. That Windows binary was **not executed here**.

## Executed results

| Check | Actual result | Scope |
|---|---|---|
| Fresh Release desktop build + CTest | 21/21 passed | Includes core, persistence, settings, deterministic fuzz/corpus, fault injection, layering and banned-API gate tests |
| Fresh ASan/UBSan headless CTest | 21/21 passed; no sanitizer findings in output | Leak detection and fatal sanitizer reports enabled |
| Fresh coverage-build CTest | 21/21 passed | Separate clean GCC counters |
| Python tooling unit tests | 33/33 passed | unittest discovery in `tests/test_*.py` |
| cppcheck 2.13.0 | No findings | Existing unchanged warning/style/performance/portability gate |
| Banned APIs | 0 hits in 47 files | Existing gate, plus planted-call and lookalike controls in CTest |
| Layering | Passed, 35 SFML-free files | Existing unchanged gate |
| Real SFML/OpenAL backend | 7/7 passed | Null output; gain behavior only, not audible listening |
| Navigation UI | 23/23 passed | Blank/invalid/length-limited names, nine mode/difficulty combinations, mouse/pause/restart, duplicate key refusal, slider clamps |
| Settings UI | 22/22 passed | Independent volume previews, Save/Cancel, themes, remap persistence, failed-save transaction, relaunch, corrupt-config fallback, no-backend usability |
| Leaderboard UI smoke | 29/29 passed | Six-column table, adversarial widths, empty/corrupt/missing data and navigation |
| Security UI | 23/23 passed | Real 10,000-character input stress, unsafe characters, bounded buffer, limited runtime-file privacy checks |
| Tejas enhanced leaderboard UI | 75/75 passed | `01,02,04,sec04`: real games/restart, six columns + Back, recovery and normal-user permission refusal |
| Power-up runtime | 21/21 passed in this 150-second run | Speed, multiplier and shrink collected; actual dynamic checks, not a fixed STP test-case count |
| D6 focused regression | 4/4 assertions, 1 case | Empty and whitespace-only boards return warnings |
| Security corpus focused run | 732 assertions, 21 cases passed | All 25 valid + 25 malformed fixtures; deterministic tests, not coverage-guided fuzzing |
| Fault-injection focused run | 214 assertions, 8 cases passed | Existing Linux injection and preservation/cleanup protection |
| Probe smoke, three separate runs | All passed; 100 requested records and 3 launch traces each | Each had 4 accepted / 96 rejected inputs; **not** 100 accepted inputs or target-performance acceptance |

These rows overlap. Do not sum them into a count of unique STP cases or claim all 44 STP cases passed.

The four key-remapping concerns have **automated Linux evidence**: changed binding and duplicate rejection exercised in UI; `SettingsTests.cpp` checks every new primary, permanent WASD aliases and removal of each old primary. They are **not newly completed Windows manual results**. F5 was sent during Linux Settings smoke; null output cannot establish audible SFX playback.

## Component coverage

| Component | Covered / executable lines | Line coverage | >=80% |
|---|---|---|---|
| C6 | 209/213 | 98.12% | Passed |
| C7 | 205/212 | 96.70% | Passed |
| C8 | 178/183 | 97.27% | Passed |
| C9 | 24/25 | 96.00% | Passed |
| C10 | 36/37 | 97.30% | Passed |
| C11 | 180/196 | 91.84% | Passed |

Mapping is preserved in `evidence/owner-automated-7c6733d/component-coverage.json`. Counts include executable header lines. Different compiler versions produce slightly different line inventories than Tejas's GCC13 report; this independent GCC11 run meets the same per-component criterion. Whole selected source/header inventory: 1,678/1,790 lines (93.74%); component criterion is C6–C11, not a claim that every file exceeds 80%.

## Setup failures and reruns — not hidden

1. This sandbox's uid1000 initially inherited effective Linux capabilities, including DAC override. Read-only-folder tests therefore unexpectedly wrote successfully, failing two suites. A control experiment verified write refusal after removing all capabilities. Full sanitizer and coverage suites were rerun with `setpriv --bounding-set=-all --inh-caps=-all --ambient-caps=-all`; both passed 21/21. No test was skipped or weakened. Original invalid-environment outputs are retained in `setup-attempts.txt`.
2. Missing OpenGL/runtime development dependencies were restored. An initial manual desktop configure omitted the app preset's static-library flag and failed shared-library linkage; the final configure explicitly matches `BUILD_SHARED_LIBS=OFF`, then builds and passes. These were setup/configuration attempts, not passing test evidence.

## Reproduction

Use a separate clean checkout of the exact source SHA, not a modified developer workspace:

```sh
git checkout --detach 7c6733d95a4fa61a3af83c413a27d1ad2f40d44b
cmake --preset app -DSNAKE_WARNINGS_AS_ERRORS=ON
cmake --build --preset app --parallel 3
ctest --preset app
python -m unittest discover -s tests -p 'test_*.py'
cmake -P cmake/RunCppcheck.cmake
```

On this elevated sandbox, wrap **test** commands in `setpriv --bounding-set=-all --inh-caps=-all --ambient-caps=-all`; normal developer machines should already have no effective capabilities. Confirm a chmod500-directory write is refused. Dependencies include Xvfb, xdotool, xwininfo, Mesa/OpenGL, SFML2.5.1, OpenAL, Pillow and cppcheck2.13.

Separate sanitizer configure: app OFF, Debug, Clang, `-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer`, linker `-fsanitize=address,undefined`; CTest env `ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1`, `UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1`.

Separate coverage configure: app OFF, Debug, GCC, `--coverage -O0`, linker `--coverage`; remove old `.gcda` counters before running CTest, then gcovr for project `src/` and `include/`.

Run the existing desktop scripts sequentially against the freshly built app in isolated fixtures: `desktop_navigation_smoke.py`, `desktop_settings_smoke.py`, `desktop_leaderboard_smoke.py`, `security_ui_smoke.py`, `powerup_runtime_check.py` (150 seconds), and `desktop_probe_smoke.py` (three new output directories). Run Tejas's enhanced `leaderboard_ui_check.py` from its exact PR58 worktree with selector `01,02,04,sec04`; do not copy it away from its fixtures. Xvfb/software rendering and null audio are explicit scope limitations.

## Still pending — cannot be passed on this machine

| Item | What is still required |
|---|---|
| #23 TC-PORT-01 Windows10 | A real Windows10 source build/run of exact candidate; OS/MSVC/CMake, build output, CTest, EXE size/hash and gameplay evidence. Ayushi's lack of access is an access gap, not a pass. Reassign to someone with access or obtain an explicit approved scope deviation. |
| #23 TC-PORT-01 Windows11 | Same real-PC source-build evidence on Windows11. CI artifact execution alone does not replace the source build. |
| Windows/macOS manual and crash cases | Execute required supported-platform cases and native crash/failure checks, or explicitly approve/document a scope change. Linux injection does not prove native Windows/macOS behavior. |
| Audible audio and F5 | A person must listen to independent channels, preview/Save/Cancel and F5 on the candidate; null output cannot establish this. |
| #24 final-candidate privacy | Bharath's historical samples lack exact build provenance. Rerun the exact candidate EXE with source SHA/hash, process-owned TCP+UDP and actual enforced Windows Firewall/offline gameplay evidence; publish redacted records and limits. Linux source scans are not a Windows privacy rerun. |
| #22 performance / usability | Reference Windows hardware, three 300-second Hard/Obstacle campaigns with power-up activity, 100 suitable randomized inputs and required volunteer/launch evidence. Probe smoke is diagnostic only. |
| #25 final QA sign-off | Reconcile the 44-case approved STP/requirements matrix, native manual evidence, accepted deviations and residual defects. Preserve historical runs rather than relabeling them. |
| #26 and #28 | Owner UAT/release decision, package/tag/freeze and demo only after accepted gates. No release authorization is given by this report. |

No final QA sign-off, Windows10/11 pass, universal no-network guarantee, target performance pass or human usability pass is asserted.

## Published evidence

All paths below are under `docs/qa/evidence/owner-automated-7c6733d/`:

- `execution-transcript.txt`: sanitized final test outputs, including commands' result scope.
- `setup-attempts.txt`: initial invalid-environment failures and diagnosis.
- `build-manifest.json`: exact source, tool and local binary identities; no local usernames/hostnames.
- `component-coverage.json` and `coverage-lines.txt`: component mapping and per-file line results.
- `probe-scope.json`: only non-sensitive probe counts; raw runtime machine reports remain private.
- `lb02-ten-entries.png` and `lb02-empty-board.png`: synthetic leaderboard fixtures; ten-entry screenshot independently inspected for all six fields and Back.

This evidence is additive to Tejas PR58 and Ayushi's earlier QA reports. It does not rewrite their histories, merge PR58, change game code or move pending acceptance work to Done.

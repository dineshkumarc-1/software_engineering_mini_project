# QA Test Execution Log

**Project:** SnakeXtreme  
**Issue:** #25 — QA Sign-off, Test Summary Report & Documentation  
**Sprint:** Sprint 2 (26–30 October 2026)  
**Tester:** Ayushi Srivastava  
**Status:** Preliminary testing — final sign-off pending

## 1. Test Environment

### Run A — Earlier Headless Test Run

- Operating system: macOS
- Build system: CMake
- Test framework: Catch2
- Build preset: `headless`
- Test branch: `review/issue-16-validator`
- Reported result: 8 passed, 0 failed
- Reported execution time: 1.10 seconds
- Exact tested commit and compiler/tool versions: Not captured in the original record

### Run B — Reproducible Headless Test Run

- Execution date: 27 October 2026
- Operating system: macOS ARM64
- Tested commit: `aa1cc1e22319f018f7dd515e77f70d45d5a3712d`
- Build preset: `headless`
- Compiler: Apple Clang 21.0.1 (`clang-2100.1.1.101`)
- CMake: 4.4.4
- CTest: 4.4.4
- Test framework: Catch2
- Build result: Successful
- CTest result: 7 passed, 0 failed
- Total CTest execution time: 2.70 seconds

Environment and raw output files:

- `docs/qa/evidence/headless-environment-aa1cc1e.txt`
- `docs/qa/evidence/ctest-output-aa1cc1e.txt`

The environment capture was recorded at 17:08:35 IST. This is the environment-capture time, not a separately measured CTest start time.

### Run C — Windows x64 Manual Tests

- Reported execution date: 28 October 2026
- Platform: Windows x64
- Test type: Manual functional testing
- Reported result: 7 passed, 0 failed
- Artifact name: `SnakeXtreme-windows-x64`
- Artifact ID: `12712437257`
- Artifact page: https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/40456104168/artifacts/12712437257
- Workflow run: https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/40456104168
- Workflow job: App build (Windows MSVC)
- Workflow job URL: https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/40456104168/job/122029676098
- Workflow result: Succeeded
- Workflow run commit SHA: `53465b167128caa787c182d36a859a51a5fec8a9`
- Artifact usage: Downloaded and launched for Windows x64 manual testing.
- Individual screenshots, recordings, or other evidence references: Not yet attached.

These results are based on the reported manual observations documented in Section 5.1. They are separate from the automated headless CTest runs.

The recorded SHA identifies the source revision checked out by the documented workflow. The workflow and artifact references are included to support build traceability.

## 2. Commands Executed

The following commands were executed for Run B at the tested commit:

```bash
cmake --preset headless
cmake --build --preset headless
ctest --preset headless --output-on-failure
```

The CTest output was captured using:

```bash
ctest --preset headless --output-on-failure 2>&1 | tee ctest-output.txt
```

## 3. Results — Run A

The original recorded run reported the following results:
| Registered test | Result |
|---|---|
| `app_state_flow` | Passed |
| `score_service` | Passed |
| `input_validator_tests` | Passed |
| `game_session` | Passed |
| `game_engine_tests` | Passed |
| `game_mode_tests` | Passed |
| `gameplay_tests` | Passed |
| `layering_core_is_sfml_free` | Passed |

- Total registered CTest tests: 8
- Passed: 8
- Failed: 0
- Reported pass rate: 100%
- Reported execution time: 1.10 seconds

The exact tested commit and tool versions were not captured for this run.

## 4. Results — Run B

The new CTest run at commit `aa1cc1e22319f018f7dd515e77f70d45d5a3712d` reported:
| Registered test | Result |
|---|---|
| `app_state_flow` | Passed |
| `score_service` | Passed |
| `game_session` | Passed |
| `game_engine_tests` | Passed |
| `game_mode_tests` | Passed |
| `gameplay_tests` | Passed |
| `layering_core_is_sfml_free` | Passed |

- Total registered CTest tests: 7
- Passed: 7
- Failed: 0
- Pass rate: 100%
- Total execution time: 2.70 seconds

The two runs have different registered test inventories. In particular, `input_validator_tests` appears in Run A but not Run B. These results must remain separate; the later run does not establish that the validator test was executed at the newer commit.

## 5. Preliminary Findings

- The headless configuration and build succeeded for Run B.
- All seven CTest tests registered for Run B passed.
- All eight tests in the earlier Run A were reported as passing.
- The earlier validator unit tests covered allowed characters, invalid characters, length limits, sanitization, and maximum sanitized output length. This is historical evidence for the validator component on `review/issue-16-validator`, not a test result for the current QA branch.
- Historical commit `d2a8a36` on `review/issue-16-validator` contains the validator source, header, tests, and CMake registration. These files and registration are absent from the current QA branch's tracked source. Searches of tracked C++ source on the current QA branch found no matches for the historical validator APIs or the searched player-name validation terms. This does not rule out equivalent validation under different names or in unsearched locations.
- The existing local `build/headless/input_validator_tests` executable is a build artifact. Its individual passing result does not establish that the current QA branch can build or reproduce the validator tests from source. UI input integration and pre-persistence sanitization remain pending verification. Confirm whether equivalent validation exists in the current implementation and meets the approved requirements before drawing a conformance conclusion.
- The automated results cover only the registered headless CTest tests for their respective runs.

## 5.1 Windows Manual Tests — Reported Results

The following results were reported for manual testing of the Windows x64 artifact on 28 October 2026.
| Test ID | Test area | Expected behavior | Reported actual result | Status |
|---|---|---|---|---|
| WIN-MAN-01 | Audio controls — music 0%, SFX 100% | Music is silent while sound effects remain audible. | Music was silent; eating sound was audible. | Reported pass |
| WIN-MAN-02 | Audio controls — music 100%, SFX 0% | Music remains audible while sound effects are silent. | Music was audible; eating sound was silent. | Reported pass |
| WIN-MAN-03 | Settings Save/Cancel | Cancel restores the previous setting rather than applying the pending change. | Cancel restored the previous setting. | Reported pass |
| WIN-MAN-04 | Pause/Resume | Esc pauses and resumes gameplay. | Esc paused and resumed gameplay. | Reported pass |
| WIN-MAN-05 | Settings persistence | Saved settings remain in effect after relaunch. | Saved settings persisted after relaunch. | Reported pass |
| WIN-MAN-06 | Key remapping | Both configured upward movement keys move the snake upward. | W and Up Arrow both moved the snake upward. | Reported pass |
| WIN-MAN-07 | Theme switching | Selecting a theme updates the displayed theme and visual appearance. | The theme name and appearance changed. | Reported pass |

**Reported manual-test total:** 7 tests; 7 reported passes; 0 reported failures.

The total above covers only WIN-MAN-01 through WIN-MAN-07. The F5 observation and supplemental key-remapping checks are excluded from this total.

### Additional Key-Remapping Coverage

The following results were reported for Windows x64 manual testing on 28 October 2026 using the `SnakeXtreme-windows-x64` artifact from workflow run `40456104168`.
| Test area | Expected behavior | Reported actual result | Status |
|---|---|---|---|
| Change a movement binding | The newly assigned key moves the snake in the intended direction. | Changed Move Up from Up Arrow to U; pressing U moved the snake upward. | Reported pass |
| Duplicate-key rejection | Assigning one key to two actions is rejected with an error message. | With Move Up assigned to U, assigning U to Move Down was rejected and an error message appeared. | Reported pass |
| WASD aliases after remapping to U | WASD aliases continue to move the snake in their respective directions. | W, A, S, and D worked in their respective directions after Move Up was changed to U. | Reported pass |
| Old primary key after remapping to U | The replaced primary key no longer performs the remapped action. | Up Arrow no longer moved the snake upward after Move Up was changed to U. | Reported pass |
| Approved STP v1.0 I/K/J/L remapping sequence | I, K, J, and L control the intended movement directions. | Move Up/Down/Left/Right were changed to I/K/J/L; each key moved the snake in its intended direction. | Reported pass |
| WASD aliases after remapping to I/K/J/L | WASD aliases remain functional after the prescribed remapping. | W, A, S, and D continued to work in their respective directions. | Reported pass |
| Old primary keys after remapping to I/K/J/L | The old primary keys stop performing their previous movement actions. | This behavior was not separately verified after the I/K/J/L remapping. | Pending |

The U remapping checks are supplemental observations. The I/K/J/L sequence directly exercises the remapping sequence specified by STP v1.0, TC-SET-01 (FR-026). These results are based on reported manual observations and have not been independently reproduced during this documentation update.

### Additional Observation — F5 During Gameplay

- **Test date:** 28 October 2026
- **Environment:** Windows x64
- **Artifact:** `SnakeXtreme-windows-x64`
- **Workflow run:** `40456104168`
- **Tested workflow commit SHA:** `53465b167128caa787c182d36a859a51a5fec8a9`
- **Action:** Pressed F5 while the snake was moving.
- **Actual result:** The snake continued moving; gameplay did not pause.
- **Expected behavior:** Approved STP v1.0 TC-UI-03 specifies Esc for pausing and resuming gameplay. It does not specify F5 as a pause control.
- **Disposition:** Exploratory observation recorded. It is not counted among the seven original Windows passes and is not classified as a defect based on the currently cited requirement.
- **Implementation note:** The inspected gameplay key handler explicitly handles Escape for pause/resume but contains no explicit F5 handling in the relevant key switch.

### Evidence and Traceability Limitations

- The Windows app-build workflow checked out commit `53465b167128caa787c182d36a859a51a5fec8a9`. The workflow job and artifact references are recorded in Section 1, and the artifact was downloaded and launched for manual testing.
- Individual screenshots, recordings, or other evidence references have not yet been attached to this log.
- The reported observations have not been independently reproduced as part of this documentation update.
- The seven reported Windows tests still need to be mapped to their corresponding approved STP v1.0 test-case IDs.

These manual results must not be combined with the automated CTest totals or treated as evidence that the complete planned test inventory has passed.

## 6. Outstanding QA Work

The following activities still require execution or verification against the project test plan:

- Complete execution of the planned STP test cases.
- Verify that 100% of planned High-priority test cases pass.
- Verify UI and gameplay integration.
- Verify persistence and leaderboard behavior against all applicable requirements.
- Execute performance and input-latency testing.
- Complete security, portability, and applicable static-analysis checks.
- Review outstanding defects and release exit criteria.
- Complete requirements traceability against SRS v1.0, SRS Addendum v1.1, SAD v1.0, and STP v1.0.
- Attach or reference available evidence for the Windows manual tests.
- Map the seven reported Windows tests to approved STP test-case IDs.
- Revisit the F5 observation if further requirements evidence identifies F5 as a required pause control; the currently cited STP v1.0 TC-UI-03 specifies Esc for pausing and resuming.
- The changed-binding, duplicate-key rejection, and WASD-after-remapping observations are recorded in Section 5. The remaining key-remapping check is to verify whether the old primary movement keys stop working after switching to I/K/J/L.
- Verify all other required Windows platform, integration, and acceptance tests.
- Complete the test summary report with traceable execution metrics.

## 7. Preliminary QA Conclusion

Both recorded headless runs reported successful results for their respective registered CTest inventories:

- Run A: 8/8 tests passed, with 0 reported failures.
- Run B: 7/7 tests passed, with 0 reported failures, at commit `aa1cc1e22319f018f7dd515e77f70d45d5a3712d`.

Additionally, seven Windows x64 manual tests were reported as passing on 28 October 2026. The associated artifact, workflow job, and workflow commit SHA are recorded in Section 1. Individual evidence references have not yet been attached, and the reported manual results still require mapping to approved STP v1.0 test-case IDs.

The F5 observation and supplemental key-remapping observations are recorded separately and are not included in the seven reported Windows passes. The old-primary-key behavior after switching to I/K/J/L remains unverified.

The automated and manual results cover different test activities and must be reported separately. They do not establish completion of the full planned test inventory, successful execution of every High-priority test case, complete requirements conformance, or satisfaction of all release exit criteria.

**Final QA sign-off remains pending. Issue #25 must remain open until the planned test cases, outstanding integration and platform checks, requirements deviations, defect status, and applicable release exit criteria have been assessed.**

## 8. Evidence and Maintenance

The recorded headless execution evidence is maintained in:

- `docs/qa/evidence/headless-environment-aa1cc1e.txt`
- `docs/qa/evidence/ctest-output-aa1cc1e.txt`

The execution history and requirement findings are also documented in:

- `docs/qa/QA_TEST_CHECKLIST.md`
- `docs/qa/REQUIREMENTS_DEVIATION_MATRIX.md`

Update this log whenever additional tests are executed or existing results are reproduced, corrected, or superseded.

For every test, record the test-case ID, actual result, environment, tested build or commit, and supporting evidence. Record failures with their expected behavior, observed behavior, reproduction steps, and associated defect reference.

Do not mark a test as passed without supporting execution evidence, and do not make a final QA sign-off decision until the applicable release exit criteria have been assessed.

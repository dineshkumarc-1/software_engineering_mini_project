# QA Test Execution Log

**Project:** SnakeXtreme  
**Issue:** #25 — QA Sign-off, Test Summary Report & Documentation  
**Sprint:** Sprint 2 (26–30 October 2026)  
**Tester:** Ayushi Srivastava  
**Status:** Preliminary testing — final sign-off pending

## Current-candidate owner automation (separate from historical Runs A–C)

On 30 October 2026, automated QA was executed against unchanged source `7c6733d95a4fa61a3af83c413a27d1ad2f40d44b` on Amazon Linux 2023, not Windows. Fresh Release, ASan/UBSan and coverage CTest each passed 21/21; 33 Python tests and the unchanged cppcheck/layering/banned-API gates passed. All executed real-window Linux harnesses passed.

[Full owner report and sanitized evidence](OWNER-AUTOMATED-QA-7c6733d.md) records exact counts, component coverage, Tejas PR58 tooling revision, permission-environment failures and clean reruns. Null audio is not audible acceptance; Linux UI tests are not native Windows/macOS tests. No historical run/artifact has been relabeled and no final QA sign-off has been given.

## Documentation reconciliation / received Windows report (not a new execution)

See [Windows 10evidence register](WINDOWS10-EVIDENCE-REGISTER.md) for the received baseline 7c6733dlocalbuild20/20 CTest,33/33 Python,D6/D7reportedresults and exactEXEhash. Referenced raw logs/client-area screenshot have not all been independently reviewed here. Dinesh personally played andaccepted functionality; no per-case audio-channel values,measurements or volunteers are invented.

The [44-case matrix](STP_EXECUTION_MATRIX.md) and [Test Summary Report](TEST_SUMMARY_REPORT.md) reconcile published evidence against the submittedPDF. NFR-004 means3 primary screens; TC-PWR-05 means effectreplacement; CMake'sLinux-only fault suite explains20versus21CTest entries. Historical runs below remain unchanged. PR #60audio 09bbf96is separate unmergedsource,not covered by the baselineEXE.

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
- Artifact page: [SnakeXtreme Windows x64 artifact](https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/40456104168/artifacts/12712437257)
- Workflow run: [Windows build workflow run](https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/40456104168)
- Workflow job: App build (Windows MSVC)
- Workflow job URL: [App build (Windows MSVC)](https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/40456104168/job/122029676098)
- Workflow result: Succeeded
- Workflow run commit SHA: `53465b167128caa787c182d36a859a51a5fec8a9`
- Artifact usage: Downloaded and launched for Windows x64 manual testing.
- Individual screenshots, recordings, or other evidence references: Not yet attached.

These results are based on the reported manual observations documented in Section 5.1. They are separate from the automated headless CTest runs.

The recorded SHA identifies the source revision checked out by the documented workflow. The workflow and artifact references are included to support build traceability. This SHA must not automatically be treated as the exact tested artifact version unless that relationship is verified.

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
- The earlier validator unit tests covered allowed characters, invalid characters, length limits, sanitization, and maximum sanitized output length.
- Searches of the inspected source and header directories on the earlier review branch did not identify calls to `isValidPlayerName()` or `sanitizePlayerName()` outside the validator component itself.
- UI input integration and pre-persistence sanitization remain pending verification.
- The automated results cover only the registered headless CTest tests for their respective runs.

### 5.1 Windows Manual Tests — Reported Results

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

Review the other applicable approved requirements before deciding whether F5 requires corrective action. Do not classify the observation as a defect solely because F5 does not pause gameplay.

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
- Review any additional approved requirement that may specify F5 behavior; STP v1.0 TC-UI-03 specifies Esc, not F5, for pause/resume.
- Verify whether the old primary movement keys stop working after the prescribed I/K/J/L remapping.
- Verify all other required Windows platform, integration, and acceptance tests.
- Complete the test summary report with traceable execution metrics.

### Windows 11 Offline Network Collector — Limited Evidence

- **Platform:** Windows 11 Home Single Language.
- **Evidence source:** Collaborator-reported result and reviewed collector JSON, referenced by the PR #50 evidence note.
- **Raw evidence:** The JSON was retained outside Git because it contains local machine and path details; it is not independently available in this repository.
- **Process monitoring:** SnakeXtreme remained running in 60 of 60 samples.
- **Process-owned network endpoints:** Zero TCP connection records and zero UDP endpoint records were reported across the collected snapshots.
- **Outbound firewall rule:** ActiveStore checks reportedly found the enabled outbound block rule scoped to the game executable at both the beginning and end of collection.
- **Gameplay:** The collaborator reported that normal gameplay remained usable with the rule enabled.
- **Timing limitation:** Although the configured collection duration was 60 seconds, sample timestamps spanned approximately 98.35 seconds because of per-loop query overhead.
- **Result classification:** Limited supporting evidence; not a complete network-security acceptance result.

The collector observations apply only to this Windows 11 collection. They do not establish that all network activity is impossible, demonstrate Windows 10 conformance, or prove acceptance of the final integrated candidate. The exact final candidate SHA and corresponding artifact remain to be established.

**Outstanding:** Verify traceability to the approved requirements and STP test cases, execute the corresponding Windows 10 test, and repeat applicable checks against the identified final integrated candidate.

**Final QA sign-off remains pending.**

## 7. Preliminary QA Conclusion

Both recorded headless runs reported successful results for their respective registered CTest inventories:

- Run A: 8/8 tests passed, with 0 reported failures.
- Run B: 7/7 tests passed, with 0 reported failures, at commit `aa1cc1e22319f018f7dd515e77f70d45d5a3712d`.

Additionally, seven Windows x64 manual tests were reported as passing on 28 October 2026. The associated artifact, workflow run, and workflow commit SHA are recorded in Section 1. Individual evidence references have not yet been attached, and the reported manual results still require mapping to approved STP v1.0 test-case IDs.

Supplemental key-remapping observations are recorded separately. The old-primary-key behavior after the prescribed I/K/J/L remapping remains pending verification. The F5 observation is exploratory and is not included in the seven reported Windows passes.

The Windows 11 offline network collector result is recorded as limited supporting evidence. Its scope is restricted to the reported Windows 11 collection and does not establish Windows 10 conformance or final-candidate acceptance.

The automated and manual results cover different test activities and must be reported separately. They do not establish completion of the full planned test inventory, successful execution of every High-priority test case, complete requirements conformance, or satisfaction of all release exit criteria.

**Final QA sign-off remains pending. Issue #25 must remain open until the planned test cases, outstanding integration and platform checks, requirements deviations, defect status, and applicable release exit criteria have been assessed.**

## 8. Evidence and Maintenance

The recorded headless execution evidence is maintained in:

- `docs/qa/evidence/headless-environment-aa1cc1e.txt`
- `docs/qa/evidence/ctest-output-aa1cc1e.txt`

The execution history and requirement findings are also documented in:

- `docs/qa/QA_TEST_CHECKLIST.md`
- `docs/qa/REQUIREMENTS_DEVIATION_MATRIX.md`

The Windows 11 collector's raw JSON is not included in this repository. Refer to the PR #50 evidence note for its provenance.

Update this log whenever additional tests are executed or existing results are reproduced, corrected, or superseded.

For every test, record the test-case ID, actual result, environment, tested build or commit, and supporting evidence. Record failures with their expected behavior, observed behavior, reproduction steps, and associated defect reference.

Do not mark a test as passed without supporting execution evidence, and do not make a final QA sign-off decision until the applicable release exit criteria have been assessed.

<!-- Evidence reconciliation: Windows performance and grid review -->

### Evidence reconciliation — later evidence review

This section supplements, and does not replace, historical execution records. The Windows performance archive's 11 raw probe JSON files pass the unchanged analyzer's schema/timestamp checks, but the available continuous Hard/Obstacle segments are only 3, 1, and 5 seconds. Three automated attempts each had one qualifying second; the idle-menu run had no gameplay. The required three 300-second sessions therefore remain pending. The 100-input trace reports 18 accepted, 82 rejected, and zero drops; its 1–6 microsecond interval is internal dequeue-to-registration timing rather than queue-inclusive acceptance. Launch measurements remain cache-unknown/warm diagnostics.

The reported grid-results JSON's title-based timing pass does not prove first-grid-render acceptance. The reported TC-CORE-01 image depicts a conversation rather than the game grid. The related grid artifacts were not found in this checkout for independent reinspection; retain the historical record and qualify its evidentiary scope rather than silently replacing it.

A separate fresh Linux Release regression on merged revision `3fcc586` / tree `41b54378c177d7d1e01ff17f2bf7b22663789ec6` succeeded with warnings-as-errors. CTest passed 21/21, Python passed 33/33, audio reported 161 assertions across 5 cases, cppcheck 2.13.0 reported zero findings, and sequential Xvfb UI checks passed navigation (23), Settings (22), and leaderboard (29). The initial parallel run's Settings missing-window and leaderboard X11 startup failures remain in the preserved logs; unchanged sequential reruns passed, and no cause was established. Permission tests ran with effective capabilities dropped and gates unchanged. The Linux EXE SHA-256 was `4abc22684ff77b433f7a900194ff2dc6a2f8c8fa1811b00030ff831c558f8b34`.

This Linux run is not a Windows-target campaign, fresh sanitizer/coverage evidence, privacy/volunteer acceptance, or release approval. The redacted 19-file evidence ZIP was supplied in the owner conversation, not committed here. Preserve earlier historical run records and reported Windows manual results with their existing build/artifact limitations. Issues #22/#25 and final QA sign-off remain pending.

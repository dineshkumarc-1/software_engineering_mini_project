# Sprint 2 QA Test Checklist

**Project:** SnakeXtreme  
**QA Issue:** #25 — QA Sign-off, Test Summary Report & Documentation  
**Sprint:** Sprint 2 (26–30 October 2026)  
**QA Owner:** Ayushi Srivastava  
**Status:** In progress — final QA sign-off pending

## 1. Purpose

Track planned QA activities, execution evidence, defects, and release criteria throughout Sprint 2.

This checklist is a working document. Test cases must be checked against STP v1.0 before the final execution count is reported. Do not mark a test as passed without recording the actual result and available evidence.

Automated headless results and Windows manual results are recorded separately because they were obtained in different environments.

The approved STP contains 44 planned test cases. This is the planned inventory, not a final pass count. Execution status and final results must be based on individual test outcomes and supporting evidence recorded in the [authoritative STP execution matrix](STP_EXECUTION_MATRIX.md).

## Current reconciliation (supersedes stale generic Not Started/integration-blocked labels)

The integrated validator,UI,persistence and Settings are present; do not interpret historical generic rows below as missing current code. Implementation issues15/16/17/19/20/21were accepted by Dinesh andclosed. Use the [44-case authoritative matrix](STP_EXECUTION_MATRIX.md),[Test Summary Report](TEST_SUMMARY_REPORT.md),[Windows evidence register](WINDOWS10-EVIDENCE-REGISTER.md) and [current deviation matrix](REQUIREMENTS_DEVIATION_MATRIX.md) for current scope/status. The submitted STP has44 cases,43 requirements and30 Highcases. Mapping is complete; finalexecution/signoff is not.

Case-specific listening/visual/performance/volunteer/privacy evidence must not be invented from general owner acceptance. NewPR #60audio is unmerged product code and needs separate Windows build/listening. Historical runs below retain their actual identities and observations.

## 2. Execution Status Definitions

- **Not Started:** No execution attempt has been made.
- **In Progress:** Testing has begun but is incomplete.
- **Passed:** The expected behavior was observed and sufficient evidence was recorded.
- **Reported Pass:** A passing result was reported, but supporting evidence or artifact identification is incomplete.
- **Failed:** Actual behavior did not meet the expected result; record a defect.
- **Blocked:** Testing cannot proceed because of an identified dependency.
- **Not Applicable:** The test is not applicable, with a reason documented.

Use the status definitions consistently across this checklist and the execution log.

## 3. Preliminary Automated Test Execution

These results are historical headless CTest evidence. They do not establish that the latest `main` branch passes.

The execution log records two distinct headless runs. Their results must not be combined into one test count.

### 3.1 Earlier recorded run

| ID | Registered CTest test | Recorded result | Evidence |
|---|---|---|---|
| AUTO-01 | `app_state_flow` | Passed | Preliminary test execution log |
| AUTO-02 | `score_service` | Passed | Preliminary test execution log |
| AUTO-03 | `input_validator_tests` | Passed | Preliminary test execution log |
| AUTO-04 | `game_session` | Passed | Preliminary test execution log |
| AUTO-05 | `game_engine_tests` | Passed | Preliminary test execution log |
| AUTO-06 | `game_mode_tests` | Passed | Preliminary test execution log |
| AUTO-07 | `gameplay_tests` | Passed | Preliminary test execution log |
| AUTO-08 | `layering_core_is_sfml_free` | Passed | Preliminary test execution log |

**Recorded total for this run:** 8 registered tests; 8 passed; 0 failed.

The exact tested commit and tool versions for this run were not recorded. Do not treat the results as reproducible evidence for the current branch.

### 3.2 Later reproducible headless run

The later reproducible run is recorded against commit `aa1cc1e22319f018f7dd515e77f70d45d5a3712d`, on macOS ARM64, using Apple Clang 21.0.1 and CMake/CTest 4.4.4.

| Result | Recorded value |
|---|---|
| Build | Successful |
| CTest tests passed | 7 |
| CTest tests failed | 0 |
| Raw output | `docs/qa/evidence/ctest-output-aa1cc1e.txt` |
| Environment record | `docs/qa/evidence/headless-environment-aa1cc1e.txt` |

**Important:** These registered CTest entries are not necessarily equivalent to the full set of planned STP test cases. The two headless runs have different recorded test inventories.

The later run does not include `input_validator_tests` in its recorded seven-test inventory. Historical validator evidence must not be treated as proof that the current branch builds or passes those tests.

### 3.3 Current-branch verification

The following activities remain necessary before current-branch automated test completion can be claimed:

- Build the current QA branch using the project's supported build procedure.
- Run all available registered CTest tests.
- Record the tested commit SHA, compiler, build tools, commands, and environment.
- Preserve the raw test output.
- Investigate failures and missing test targets.
- Confirm the required sanitizer, static-analysis, and coverage jobs have run.
- Map automated tests to approved STP test cases where applicable.

Do not infer that every STP test case passed from the CTest totals alone.

### 3.4 Owner automated verification — exact candidate 7c6733d

A new independent Linux-only run on 30 October 2026 passed fresh Release CTest 21/21, ASan/UBSan CTest 21/21, coverage CTest 21/21, 33 Python tests, cppcheck (zero findings), and all executed desktop UI harnesses. C6–C11 line coverage is 91.84%–98.12%.

See [Owner automated QA report](OWNER-AUTOMATED-QA-7c6733d.md) for exact SHA/binary/tool identities, all actual counts, original setup failures, sanitized transcripts and pending native/manual cases. Tejas PR58's enhanced leaderboard harness passed 75/75 against unchanged game source 7c6733d; these are assertions/checks, not 75 unique STP cases.

Historical results below retain their original run identities and observations. Dinesh has personally reported successful Windows 10 gameplay. This is an owner-reported gameplay result, not independent confirmation that every prescribed manual observation or acceptance threshold passed.

For current platform status, consult the [Windows 10 evidence register](WINDOWS10-EVIDENCE-REGISTER.md). Windows 10 remains reported evidence pending supporting-log review. Detailed independent-channel and F5 listening checks remain Pending until the corresponding observations are supplied. At SFX volume 0, F5 silence is expected. Available themes are Classic, Neon, and Retro—not Dark. The baseline Windows 10 report does not cover PR #60 audio commit `09bbf96`.

Windows 11 remains Not tested against the identified final candidate. Earlier Windows 11 privacy sampling is historical evidence and does not establish candidate `7c6733d` portability or privacy acceptance. Final-candidate firewall/offline evidence, cold-cache launch acceptance, performance/usability campaigns, unsupported manual checks, and final STP reconciliation remain Pending. **Final QA sign-off remains pending.**

## 4. Windows x64 Manual-Test Results

Seven Windows x64 manual checks were reported as passing on 28 October 2026.

The execution record identifies the following workflow and artifact information:

| Field | Recorded information |
|---|---|
| Artifact | `SnakeXtreme-windows-x64` |
| Artifact ID | `12712437257` |
| Workflow run ID | `40456104168` |
| Job | `App build (Windows MSVC)` |
| Job ID | `122029676098` |
| Workflow commit SHA | `53465b167128caa787c182d36a859a51a5fec8a9` |
| Individual screenshots or recordings | Not attached in the available record |

The workflow commit SHA identifies the workflow's recorded source revision. It must not automatically be treated as the exact tested artifact version unless that relationship is verified.

For that reason, these results remain **Reported Pass**, pending evidence reconciliation.

| ID | Test area | Verification activity | Reported result | Follow-up |
|---|---|---|---|---|
| WIN-01 | Audio settings — music 0, SFX 100 | Verify music is silent while the eating sound remains audible. | Reported Pass | Confirm the artifact and available evidence. Map to `TC-SET-02`. |
| WIN-02 | Audio settings — music 100, SFX 0 | Verify music is audible while the eating sound is silent. | Reported Pass | Confirm the artifact and available evidence. Map to `TC-SET-02`. |
| WIN-03 | Settings Save/Cancel | Verify Cancel restores the previous setting. | Reported Pass | Confirm the exact steps and expected result before mapping. |
| WIN-04 | Pause/Resume | Verify Esc pauses and resumes gameplay. | Reported Pass | Confirm timer freezing and state preservation required by `TC-UI-03`. |
| WIN-05 | Settings persistence | Verify saved settings persist after relaunch. | Reported Pass | Confirm bindings, volumes, theme, and `config.json` layout required by `TC-SET-04`. |
| WIN-06 | Key remapping | Verify W and Up Arrow both move the snake upward. | Reported Pass | Map to `TC-SET-01`; complete outstanding old-key verification. |
| WIN-07 | Theme switching | Verify the theme name and appearance change. | Reported Pass | Map to `TC-SET-03`; confirm all three themes and gameplay behavior. |

Before treating these as fully evidenced passes:

- Confirm the exact tested artifact version or commit.
- Link available screenshots, recordings, test notes, or other supporting evidence.
- Map each check to its approved STP test-case ID and requirement.
- Record any failures or defects found during further investigation.
- Request missing evidence from the Windows tester if available.

These results support only the behaviors explicitly listed. They do not establish complete Windows compatibility or full requirements conformance.

### 4.1 Additional key-remapping observations

The existing execution log records these supplemental observations:

- Changing Move Up from Up Arrow to U allowed U to move the snake upward.
- Assigning U to Move Down while Move Up was already U was rejected with an error.
- W/A/S/D continued to work after remapping to U.
- The former Up Arrow binding did not move the snake upward after Move Up was changed to U.
- Remapping movement actions to I/K/J/L worked for each assigned direction.
- W/A/S/D continued to work after remapping to I/K/J/L.
- The former primary keys were not separately verified after the I/K/J/L remapping.

The I/K/J/L sequence exercises part of `TC-SET-01`, but the old-key behavior, full acceptance criteria, test environment, and supporting evidence must be reconciled before the complete test case is marked Passed.

### 4.2 F5 observation

An observation that F5 did not pause gameplay was recorded while the snake was moving.

`TC-UI-03` specifies Esc for pausing and resuming. No defect should be raised solely because F5 does not pause the game unless another approved requirement or test case specifies that behavior.

The F5 observation is not included in the seven reported Windows tests.

## 5. Functional and Integration Verification

The following items are broad QA tracking categories. They are not substitutes for the approved STP test-case IDs.

| ID | Test area | Verification activity | Priority | Status | Evidence / defect reference |
|---|---|---|---|---|---|
| QA-01 | Application state flow | Verify applicable state transitions and invalid transitions. | Confirm from STP | Not Started | |
| QA-02 | Game engine | Execute approved engine behavior and boundary test cases. | Confirm from STP | Not Started | |
| QA-03 | Game modes | Verify each required mode against its specification. | Confirm from STP | Not Started | |
| QA-04 | Gameplay | Verify specified gameplay rules and expected outcomes. | Confirm from STP | Not Started | |
| QA-05 | Scoring | Verify score calculation, updates, and relevant boundary cases. | Confirm from STP | Not Started | |
| QA-06 | Player-name validation | Verify permitted characters, empty input, invalid characters, and the 15-character limit. | Confirm from STP | Not Started | Historical validator unit-test evidence exists; current-branch and integration results remain unverified. |
| QA-07 | UI validation integration | Verify invalid names are rejected or handled as specified by approved requirements. | Confirm from STP | Blocked pending integration verification | |
| QA-08 | Persistence integration | Verify player-name sanitization and stored data against approved requirements. | Confirm from STP | Blocked pending implementation review | |
| QA-09 | Leaderboard | Verify persistence and leaderboard behavior against approved test cases. | Confirm from STP | Not Started | |
| QA-10 | Application settings | Verify settings behavior against the approved test cases. | Confirm from STP | Not Started | |

## 6. Overall QA Status

Final QA sign-off: **Pending**.

Remaining tests, evidence gaps, and integration checks must be resolved or explicitly documented before final sign-off.

The detailed execution records are maintained in `docs/qa/TEST_EXECUTION_LOG.md`.

## 7. Windows 11 Offline Network Collector Evidence

**Evidence status:** Collaborator-reported result; JSON reviewed, with limitations.

**Evidence reference:** PR #50 evidence note. The raw collector JSON was kept outside Git because it contains local machine and path details. This checklist does not independently preserve the raw JSON.

| Field | Recorded observation |
|---|---|
| Platform | Windows 11 Home Single Language |
| Process monitoring | The SnakeXtreme process remained running in 60 of 60 samples. |
| TCP observations | Zero process-owned TCP connection records across the collected snapshots. |
| UDP observations | Zero process-owned UDP endpoint records across the collected snapshots. |
| Outbound firewall rule | ActiveStore checks reported the enabled outbound block rule scoped to the game executable as present at both the start and end of collection. |
| Gameplay under the rule | The collaborator reported that normal gameplay remained usable while the block rule was enabled. |
| Sampling duration | The configured duration was 60 seconds, but sample timestamps spanned approximately 98.35 seconds because of per-loop query overhead. |
| Evidence limitation | The raw JSON is not stored in this repository. The result is based on the collaborator's report and review of that JSON. |

**Interpretation:** These observations provide limited evidence about process monitoring, observed process-owned network endpoints, and the scoped outbound firewall rule during this Windows 11 collection. Zero observed endpoint records do not prove that all network activity is impossible or that every security requirement is satisfied. The longer-than-configured sampling interval must be retained as a limitation.

**Outstanding work:**

- Record or link the exact PR #50 evidence note for future reviewers.
- Verify the collector evidence against the approved requirements and applicable STP test cases.
- Historical action: execute and record the corresponding Windows 10 test. Dinesh has since reported a Windows 10 gameplay result; consult the [Windows 10 evidence register](WINDOWS10-EVIDENCE-REGISTER.md) for current evidence status. Supporting-log review remains pending.
- Repeat applicable checks against the identified final integrated candidate and record its exact SHA and artifact reference.
- Complete outstanding platform, usability, performance, and regression testing.

This collector result does not replace the seven historical Windows x64 manual checks, establish that they used the final integrated candidate, or authorize final QA sign-off.

**Final QA sign-off remains Pending.**

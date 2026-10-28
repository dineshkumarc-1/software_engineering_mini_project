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

## 2. Execution Status Definitions

- **Not Started:** No execution attempt has been made.
- **In Progress:** Testing has begun but is incomplete.
- **Passed:** The expected behavior was observed and sufficient evidence was recorded.
- **Reported Pass:** A passing result was reported, but supporting evidence or artifact identification is incomplete.
- **Failed:** Actual behavior did not meet the expected result; record a defect.
- **Blocked:** Testing cannot proceed because of an identified dependency.
- **Not Applicable:** The test is not applicable, with a reason documented.

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

## 4. Windows x64 Manual-Test Results

Seven Windows x64 manual checks were reported as passing on 28 October 2026. The exact tested artifact commit/version and supporting evidence have not yet been recorded in the available execution log.

For that reason, these results are classified as **Reported Pass**, pending evidence reconciliation.

| ID | Test area | Verification activity | Reported result | Follow-up |
|---|---|---|---|---|
| WIN-01 | Audio settings — music 0, SFX 100 | Verify music is silent while the eating sound remains audible. | Reported Pass | Record artifact version and supporting evidence. |
| WIN-02 | Audio settings — music 100, SFX 0 | Verify music is audible while the eating sound is silent. | Reported Pass | Record artifact version and supporting evidence. |
| WIN-03 | Settings Save/Cancel | Verify Cancel restores the previous setting. | Reported Pass | Record artifact version and supporting evidence. |
| WIN-04 | Pause/Resume | Verify Esc pauses and resumes gameplay. | Reported Pass | Record artifact version and supporting evidence. |
| WIN-05 | Settings persistence | Verify saved settings persist after relaunch. | Reported Pass | Record artifact version and supporting evidence. |
| WIN-06 | Key remapping | Verify W and Up Arrow both move the snake upward. | Reported Pass | Record artifact version and supporting evidence. |
| WIN-07 | Theme switching | Verify the theme name and appearance change. | Reported Pass | Record artifact version and supporting evidence. |

Before treating these as fully evidenced passes:

- Record the exact tested artifact version or commit.
- Link available screenshots, recordings, test notes, or other supporting evidence.
- Map each check to its corresponding STP test-case ID and requirement.
- Record any failures or defects found during further investigation.
- Verify F5 behavior if it is required by the approved test plan; it is not included in the seven reported results above.

These results support only the behaviors explicitly listed. They do not establish complete Windows compatibility or full requirements conformance.

## 5. Functional and Integration Verification

Confirm exact test identifiers, priorities, and expected results against STP v1.0. These are tracking items, not a replacement for the approved test plan.

| ID | Test area | Verification activity | Priority | Status | Evidence / defect reference |
|---|---|---|---|---|---|
| QA-01 | Application state flow | Verify applicable state transitions and invalid transitions. | Confirm from STP | Not Started | |
| QA-02 | Game engine | Execute approved engine behavior and boundary test cases. | Confirm from STP | Not Started | |
| QA-03 | Game modes | Verify each required mode against its specification. | Confirm from STP | Not Started | |
| QA-04 | Gameplay | Verify specified gameplay rules and expected outcomes. | Confirm from STP | Not Started | |
| QA-05 | Scoring | Verify score calculation, updates, and relevant boundary cases. | Confirm from STP | Not Started | |
| QA-06 | Player-name validation | Verify permitted characters, empty input, invalid characters, and the 15-character limit. | Confirm from STP | Not Started | Validator unit-test evidence exists; end-to-end enforcement remains unverified. |
| QA-07 | UI validation integration | Verify invalid names are rejected or handled as specified by the approved requirements. | Confirm from STP | Blocked pending integration verification | |
| QA-08 | Persistence integration | Verify player-name sanitization and stored data against the approved requirements. | Confirm from STP | Blocked pending implementation review | |
| QA-09 | Leaderboard | Verify persistence and leaderboard behavior against approved test cases. | Confirm from STP | Not Started | |
| QA-10 | Application layering | Run approved architecture or dependency checks. | Confirm from STP | Not Started | `layering_core_is_sfml_free` passed for its specific checked property. |

The Windows manual results in Section 4 must not automatically change these items to Passed. Update each item only after its expected behavior, scope, and evidence are mapped to the approved test plan.

## 6. Non-Functional Verification

| ID | Test area | Verification activity | Status | Evidence / defect reference |
|---|---|---|---|---|
| NFR-01 | Performance | Measure FPS against the exact SRS requirement on the specified reference environment. | Not Started | |
| NFR-02 | Input latency | Measure latency using the STP's specified method and acceptance threshold. | Not Started | |
| NFR-03 | Persistence constraints | Verify the specified persistence file-size limit and related requirements. | Not Started | |
| NFR-04 | Platform compatibility | Execute required platform tests, including Windows testing if required by the plan. | In Progress — evidence reconciliation pending | Seven Windows manual checks were reported as passing; full platform coverage remains unverified. |
| NFR-05 | Security and input handling | Execute applicable input-validation and fault-injection tests in the approved plan. | Not Started | |
| NFR-06 | Static analysis and coverage | Record results against the exact project exit criteria. | Not Started | |

## 7. Requirement Traceability

For each applicable requirement in SRS v1.0, SRS Addendum v1.1, and SAD v1.0, record:

- Requirement or component ID.
- Exact expected behavior or design constraint.
- Linked STP test-case ID.
- Implementation version or commit tested.
- Test environment and execution date.
- Actual result and supporting evidence.
- Deviation or defect reference, if applicable.

Do not conclude **"No deviation identified"** until the relevant scope has been checked. State the exact scope verified.

## 8. Defect Tracking

For every failed test or confirmed deviation, record:

| Field | Details |
|---|---|
| Defect ID / GitHub issue | |
| Requirement and test-case ID | |
| Build or commit tested | |
| Test environment | |
| Preconditions | |
| Reproduction steps | |
| Expected result | |
| Actual result | |
| Impact | |
| Reason, if known | |
| Severity and priority | |
| Evidence | |
| Associated approved change, if applicable | |
| Current status | |

Retest fixed defects and record the commit or build containing the fix. Missing evidence alone does not establish a confirmed defect.

## 9. Release Exit Criteria

Use the approved STP v1.0 as the authority for release criteria.

- [ ] 100% of planned test cases executed.
- [ ] 100% of High-priority test cases passed.
- [ ] Zero open Critical defects.
- [ ] Zero open High defects.
- [ ] Required coverage and static-analysis criteria satisfied.
- [ ] Required platform, integration, performance, and acceptance tests completed.
- [ ] Requirements and design deviations documented.
- [ ] Reported Windows manual results reconciled with artifact version and supporting evidence.
- [ ] Test Summary Report completed with execution metrics.
- [ ] Final QA decision recorded and supported by evidence.

## 10. Final QA Decision

**Current status:** In progress — final QA sign-off pending.

The preliminary headless runs passed their respective registered test suites, and seven Windows x64 manual checks have been reported as passing.

However, complete execution of all planned STP test cases, the High-priority pass rate, defect exit criteria, and full requirements and architecture conformance have not yet been demonstrated.

Final sign-off must be based on the actual planned test-case inventory, traceable execution results, defect status, and all applicable release exit criteria.
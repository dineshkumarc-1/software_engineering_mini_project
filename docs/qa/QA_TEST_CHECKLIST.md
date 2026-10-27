# Sprint 2 QA Test Checklist

**Project:** SnakeXtreme  
**QA Issue:** #25 — QA Sign-off, Test Summary Report & Documentation  
**Sprint:** Sprint 2 (26–30 October 2026)  
**QA Owner:** Ayushi Srivastava  
**Status:** In progress

## 1. Purpose

Track planned QA activities, execution evidence, defects, and release criteria throughout Sprint 2.

This checklist is a working document. Test cases must be checked against STP v1.0 before the final execution count is reported. Do not mark a test as passed without recording the actual result and evidence.

## 2. Execution Status Definitions

- **Not Started:** No execution attempt has been made.
- **In Progress:** Testing has begun but is incomplete.
- **Passed:** Expected behavior was observed and evidence was recorded.
- **Failed:** Actual behavior did not meet the expected result; record a defect.
- **Blocked:** Testing cannot proceed because of an identified dependency.
- **Not Applicable:** The test is not applicable, with a reason documented.

## 3. Preliminary Automated Test Execution

These results are from the previously recorded headless CTest run. They are historical evidence, not confirmation that the latest `main` branch passes.

| ID | Registered CTest test | Recorded result | Evidence |
|---|---|---|---|
| AUTO-01 | app_state_flow | Passed | Preliminary test execution log |
| AUTO-02 | score_service | Passed | Preliminary test execution log |
| AUTO-03 | input_validator_tests | Passed | Preliminary test execution log |
| AUTO-04 | game_session | Passed | Preliminary test execution log |
| AUTO-05 | game_engine_tests | Passed | Preliminary test execution log |
| AUTO-06 | game_mode_tests | Passed | Preliminary test execution log |
| AUTO-07 | gameplay_tests | Passed | Preliminary test execution log |
| AUTO-08 | layering_core_is_sfml_free | Passed | Preliminary test execution log |

**Recorded total:** 8 registered tests; 8 passed; 0 failed.

**Important:** These eight CTest entries must not be assumed to equal all planned STP test cases.

## 4. Functional and Integration Verification

Confirm the exact test identifiers and expected results against STP v1.0. The following are tracking items, not a replacement for the approved test plan.

| ID | Test area | Verification activity | Priority | Status | Evidence / defect reference |
|---|---|---|---|---|---|
| QA-01 | Application state flow | Verify applicable state transitions and invalid transitions. | Confirm from STP | Not Started | |
| QA-02 | Game engine | Execute the approved engine behavior and boundary test cases. | Confirm from STP | Not Started | |
| QA-03 | Game modes | Verify each required mode against its specification. | Confirm from STP | Not Started | |
| QA-04 | Gameplay | Verify the specified gameplay rules and expected outcomes. | Confirm from STP | Not Started | |
| QA-05 | Scoring | Verify score calculation, updates, and relevant boundary cases. | Confirm from STP | Not Started | |
| QA-06 | Player-name validation | Verify permitted characters, empty input, invalid characters, and the 15-character limit. | Confirm from STP | Not Started | |
| QA-07 | UI validation integration | Verify invalid names are rejected or handled as specified by the approved requirements. | Confirm from STP | Blocked pending integration | |
| QA-08 | Persistence integration | Verify player-name sanitization and stored data against the approved requirements. | Confirm from STP | Blocked pending implementation review | |
| QA-09 | Leaderboard | Verify persistence and leaderboard behavior against the approved test cases. | Confirm from STP | Not Started | |
| QA-10 | Application layering | Run the approved architecture or dependency checks. | Confirm from STP | Not Started | |

## 5. Non-Functional Verification

| ID | Test area | Verification activity | Status | Evidence / defect reference |
|---|---|---|---|---|
| NFR-01 | Performance | Measure FPS against the exact SRS requirement on the specified reference environment. | Not Started | |
| NFR-02 | Input latency | Measure latency using the STP's specified method and acceptance threshold. | Not Started | |
| NFR-03 | Persistence constraints | Verify the specified persistence file-size limit and related requirements. | Not Started | |
| NFR-04 | Platform compatibility | Execute required platform tests, including Windows testing if required by the plan. | Not Started | |
| NFR-05 | Security and input handling | Execute the applicable input-validation and fault-injection tests in the approved plan. | Not Started | |
| NFR-06 | Static analysis and coverage | Record the results against the exact project exit criteria. | Not Started | |

## 6. Requirement Traceability

For each requirement in SRS v1.0, SRS Addendum v1.1, and SAD v1.0, record:

- Requirement or component ID.
- Exact expected behavior or design constraint.
- Linked STP test-case ID.
- Implementation version or commit tested.
- Actual result and evidence.
- Deviation or defect reference, if applicable.

Do not conclude "No deviation identified" until the relevant scope has been checked.

## 7. Defect Tracking

For every failed test, record:

| Field | Details |
|---|---|
| Defect ID / GitHub issue | |
| Requirement and test-case ID | |
| Build or commit tested | |
| Preconditions | |
| Reproduction steps | |
| Expected result | |
| Actual result | |
| Severity and priority | |
| Evidence | |
| Current status | |

Retest fixed defects and record the commit or build containing the fix.

## 8. Release Exit Criteria

Use the approved STP v1.0 as the authority for release criteria.

- [ ] 100% of planned test cases executed.
- [ ] 100% of High-priority test cases passed.
- [ ] Zero open Critical defects.
- [ ] Zero open High defects.
- [ ] Required coverage and static-analysis criteria satisfied.
- [ ] Required platform, integration, performance, and acceptance tests completed.
- [ ] Requirements and design deviations documented.
- [ ] Test Summary Report completed with execution metrics.
- [ ] Final QA decision recorded and supported by evidence.

## 9. Final QA Decision

**Current status:** In progress — final sign-off pending.

Final sign-off must be based on the actual planned test-case inventory, execution results, defect status, and all applicable release exit criteria.

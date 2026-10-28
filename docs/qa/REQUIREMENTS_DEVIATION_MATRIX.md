# Requirements and Design Deviation Matrix

**Project:** SnakeXtreme  
**QA Issue:** #25 — QA Sign-off, Test Summary Report & Documentation  
**Sprint:** Sprint 2 (26–30 October 2026)  
**Prepared by:** Ayushi Srivastava  
**Document status:** Preliminary — verification in progress

## 1. Purpose

This document records the comparison between the submitted requirements and design documents and the implementation evidence reviewed during QA.

The references in scope are:

- Software Requirements Specification (SRS) v1.0
- SRS Addendum v1.1
- Software Architecture Design (SAD) v1.0
- Software Test Plan (STP) v1.0

Automated headless test results and Windows manual-test results are recorded separately because they were obtained in different environments and provide different types of evidence.

A status of **Pending verification** means sufficient evidence has not yet been collected to conclude whether the implementation conforms to the relevant requirement. It does not, by itself, establish a defect or deviation.

## 2. Status Definitions

| Status | Meaning |
|---|---|
| No deviation identified | Available evidence supports conformance for the specific scope checked. |
| Deviation confirmed | Evidence demonstrates a difference between the approved document and implementation. |
| Pending verification | Evidence is insufficient to establish conformance or a deviation. |
| Not applicable | The reference does not apply to the component or test under review. |

## 3. SRS v1.0 — Functional Requirements

| Reference | Requirement or scope | Evidence reviewed | Deviation / finding | Status |
|---|---|---|---|---|
| FR-001 | Core functional requirement | Preliminary headless test results are recorded. Complete requirement-to-test mapping has not been recorded. | Conformance to the full requirement has not been established. | Pending verification |
| FR-002 | Core functional requirement | Preliminary headless test results are recorded. Complete requirement-to-test mapping has not been recorded. | Conformance to the full requirement has not been established. | Pending verification |
| FR-003 | Core functional requirement | Preliminary headless test results are recorded. Complete requirement-to-test mapping has not been recorded. | Conformance to the full requirement has not been established. | Pending verification |
| FR-004 | Core functional requirement | Preliminary headless test results are recorded. Complete requirement-to-test mapping has not been recorded. | Conformance to the full requirement has not been established. | Pending verification |
| FR-005 | Core functional requirement | Preliminary headless test results are recorded. Complete requirement-to-test mapping has not been recorded. | Conformance to the full requirement has not been established. | Pending verification |
| FR-006 | Core functional requirement | Preliminary headless test results are recorded. Complete requirement-to-test mapping has not been recorded. | Conformance to the full requirement has not been established. | Pending verification |
| FR-007 | Core functional requirement | Preliminary headless test results are recorded. Complete requirement-to-test mapping has not been recorded. | Conformance to the full requirement has not been established. | Pending verification |

**QA note:** These rows identify the functional requirement range. Before final sign-off, replace generic descriptions with the exact requirement titles and expected behavior from SRS v1.0, then map each requirement to specific test cases and evidence.

The reported Windows manual tests provide evidence for selected settings and gameplay behaviors, but the exact mapping from those tests to FR-001 through FR-007 has not yet been established.

## 4. SRS v1.0 — Non-Functional Requirements

| Reference | Requirement or scope | Evidence reviewed | Deviation / finding | Status |
|---|---|---|---|---|
| NFR-001 | Frames-per-second performance requirement | No completed reference-hardware performance measurement is recorded in the preliminary test log. | Performance conformance has not been demonstrated. | Pending verification |
| NFR-002 | Input-latency requirement | No completed input-latency measurement is recorded in the preliminary test log. | Latency conformance has not been demonstrated. | Pending verification |
| Other applicable NFRs | Review each non-functional requirement in SRS v1.0. | Requirement-by-requirement evidence remains incomplete. | No overall conformance conclusion can yet be made. | Pending verification |

## 5. SRS Addendum v1.1

| Reference | Requirement or scope | Evidence reviewed | Deviation / finding | Status |
|---|---|---|---|---|
| Player-name whitelist | Names must follow the permitted character rules. | `InputValidator` provides player-name validation and sanitization functions. Validator unit tests passed in the recorded test run. | The reviewed branch did not show call sites integrating these functions into UI input or persistence. Integration into the final implementation remains to be verified. | Pending verification |
| Player-name length | Player names are limited to 15 characters. | `MaxPlayerNameLength` is defined as 15. Validation and sanitization tests cover the length limit. | Unit-test evidence supports the validator component. End-to-end enforcement remains unverified. | Pending verification |
| Persistence file-size constraint | Persistence file must comply with the 64 KiB limit specified in the addendum. | No completed persistence file-size verification is recorded. | Compliance has not been demonstrated. | Pending verification |
| Defined constants | Implementation must conform to constants and values specified in the addendum. | Complete constant-by-constant comparison has not been recorded. | A complete comparison is required before concluding conformance. | Pending verification |
| Other addendum requirements | Review all remaining applicable requirements in SRS Addendum v1.1. | Requirement-by-requirement verification remains incomplete. | No overall conformance conclusion can yet be made. | Pending verification |

## 6. SAD v1.0 — Architecture and Design

| Design reference | Expected design property | Evidence reviewed | Deviation / finding | Status |
|---|---|---|---|---|
| Core-layer separation | Core logic should remain independent of SFML where required by the architecture. | The registered `layering_core_is_sfml_free` test passed in the recorded headless run. | **No deviation identified** for the specific core-layer separation property checked by this test. This does not establish conformance of the entire architecture. | No deviation identified — limited scope |
| Component responsibilities and interfaces | Implementation should conform to the approved architecture and component interfaces. | A preliminary source review covered validation, scoring, gameplay, engine, and presentation areas. | Full comparison against SAD v1.0 is incomplete. | Pending verification |
| Input-validation integration | Validation should be connected to the relevant input and persistence flows as required by the approved design and security requirements. | The reviewed branch contained validator declarations and definitions, but no external call sites were found in the search performed. | Further investigation is required. The absence of call sites in the reviewed branch is a finding, not yet a confirmed deviation from the final implementation. | Pending verification |
| Persistence architecture | Persistence behavior should conform to the approved design. | No complete persistence implementation and architecture comparison has been recorded. | Conformance has not been demonstrated. | Pending verification |

## 7. STP v1.0 — Test Plan and Execution

| Test-plan reference | Expected QA activity | Evidence reviewed | Deviation / finding | Status |
|---|---|---|---|---|
| Planned test execution | Execute 100% of planned test cases. | The execution log contains separate headless CTest runs and seven reported Windows manual checks. | These results do not yet demonstrate that every planned STP test case has been executed. Full execution tracking remains outstanding. | Pending verification |
| High-priority tests | Achieve a 100% pass rate for High-priority test cases. | A complete priority-tagged execution matrix has not been recorded. | The required pass rate has not been demonstrated. | Pending verification |
| Defect exit criteria | Confirm there are no open Critical or High defects, as required by the release exit criteria. | No complete defect-status audit is included in the preliminary test log. | Exit-criteria compliance remains unverified. | Pending verification |
| Performance testing | Execute the applicable performance tests on the required environment. | No completed reference-hardware FPS or input-latency results are recorded. | Performance testing evidence remains outstanding. | Pending verification |
| Platform and integration testing | Execute applicable runtime, integration, and acceptance tests. | Seven Windows x64 manual checks have been reported as passing. The exact artifact commit/version and supporting evidence have not yet been recorded. | The reported checks provide preliminary evidence for selected behaviors. Remaining platform, integration, and acceptance tests must be tracked and executed as applicable. | Pending verification |
| Coverage and static analysis | Verify applicable coverage and static-analysis exit criteria. | No complete coverage or static-analysis report is recorded in the preliminary evidence. | Compliance remains unverified. | Pending verification |

## 8. Windows x64 Manual-Test Evidence

The following results were reported for manual testing of a Windows x64 artifact on 28 October 2026. They are recorded as **reported results**, not as independently verified evidence, because the exact artifact commit/version and supporting test evidence have not yet been recorded in the available execution log.

| Test | Scope checked | Reported result | Evidence limitation |
|---|---|---|---|
| Music 0 / SFX 100 | Music was silent while the eating sound remained audible. | Pass reported | Exact artifact version and supporting evidence not recorded. |
| Music 100 / SFX 0 | Music was audible while the eating sound was silent. | Pass reported | Exact artifact version and supporting evidence not recorded. |
| Save / Cancel | Cancel restored the previous setting. | Pass reported | Exact artifact version and supporting evidence not recorded. |
| Pause / Resume | Esc paused and resumed gameplay. | Pass reported | Exact artifact version and supporting evidence not recorded. |
| Relaunch persistence | Saved settings persisted after relaunch. | Pass reported | Exact artifact version and supporting evidence not recorded. |
| Key remapping | W and Up Arrow both moved the snake upward. | Pass reported | Exact artifact version and supporting evidence not recorded. |
| Theme switching | Theme name and appearance changed. | Pass reported | Exact artifact version and supporting evidence not recorded. |

**Interpretation:** These seven reported passes support the specific behaviors listed in the table, subject to confirmation of the artifact and evidence. They do not establish that all Windows behavior, all requirements, or all planned test cases conform.

Before relying on these results for final sign-off:

1. Record the exact tested artifact version or commit.
2. Add links or paths to available screenshots, recordings, test notes, or other supporting evidence.
3. Map each check to the corresponding STP test-case ID and requirement, where applicable.
4. Investigate any failures or inconsistencies and link confirmed defects or approved changes.
5. Verify F5 behavior if it is required by the applicable test plan; it is not included in the seven results above.

## 9. Confirmed Findings Versus Open Questions

### 9.1 Findings supported by available evidence

1. The recorded headless CMake configuration and build completed successfully.
2. The execution log records an earlier headless run with eight registered CTest tests passing and a later reproducible headless run with seven tests passing. These are separate runs and must not be combined into one test count.
3. The player-name validator component provides validation and sanitization functions with a 15-character limit.
4. The recorded validator unit tests passed.
5. The `layering_core_is_sfml_free` test passed for the specific property it checks.
6. The reviewed branch did not show call sites for `isValidPlayerName()` or `sanitizePlayerName()` outside the validator component.
7. Seven Windows x64 manual checks covering audio settings, Save/Cancel, pause/resume, settings persistence, key remapping, and theme switching have been reported as passing. Artifact identification and supporting evidence remain outstanding.

### 9.2 Items requiring further verification

1. Complete mapping of FR-001 through FR-007 to their exact SRS statements and test cases.
2. Verification of every applicable NFR and SRS Addendum requirement.
3. End-to-end player-name validation at UI input and before persistence.
4. Persistence behavior, including the specified file-size constraint.
5. Full comparison of implementation components against SAD v1.0.
6. Execution tracking for every planned STP v1.0 test case.
7. High-priority test pass rate and outstanding defect status.
8. Required performance, platform, coverage, static-analysis, and acceptance evidence.
9. Exact Windows artifact commit/version and supporting evidence for the seven reported manual checks.
10. F5 behavior, if required by the applicable test plan.
11. Confirmation that reported manual results correspond to the implementation version under review.

## 10. Overall QA Assessment

**Current decision: Final QA sign-off pending.**

The preliminary headless test results are successful for the registered tests in their respective runs. Seven Windows x64 manual checks have also been reported as passing for the specific behaviors listed in Section 8.

However, the available evidence does not yet establish that all planned STP test cases have been executed, that every High-priority test has passed, that all requirements and architecture constraints have been verified, or that the release defect criteria have been met.

No broad claim of complete conformance is made in this document. Each open item must be resolved with traceable evidence before final sign-off.

A confirmed deviation must identify the specific requirement or design reference, observed behavior, evidence, impact, reason if known, and associated defect or approved change. Do not classify an item as a confirmed deviation solely because evidence is missing.

Where evidence supports conformance, use **“No deviation identified”** and explicitly state the scope verified.

## 11. Evidence and Maintenance

The preliminary execution record is maintained in:

`docs/qa/TEST_EXECUTION_LOG.md`

Relevant headless evidence files include:

- `docs/qa/evidence/ctest-output-aa1cc1e.txt`
- `docs/qa/evidence/headless-environment-aa1cc1e.txt`

Update this matrix when additional tests, source reviews, integration checks, or defect investigations produce new evidence.

For every confirmed deviation, record the specific requirement or design reference, observed behavior, evidence, impact, reason if known, and the associated defect or approved change. Where no deviation is found, state **“No deviation identified”** and specify the scope actually verified.
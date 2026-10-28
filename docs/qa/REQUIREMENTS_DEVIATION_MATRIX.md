# Requirements and Design Deviation Matrix

**Project:** SnakeXtreme
**QA Issue:** #25 — QA Sign-off, Test Summary Report & Documentation
**Sprint:** Sprint 2 (26–30 October 2026)
**Prepared by:** Ayushi Srivastava
**Document status:** Preliminary — verification in progress

## 1. Purpose

This document records the preliminary comparison between the submitted requirements and design documents and the implementation evidence reviewed during QA.

The references in scope are:

- Software Requirements Specification (SRS) v1.0
- SRS Addendum v1.1
- Software Architecture Design (SAD) v1.0
- Software Test Plan (STP) v1.0

A status of **Pending verification** means sufficient evidence has not yet been collected to conclude whether the implementation conforms to the relevant requirement. It does not, by itself, establish a defect or deviation.

## 2. Status Definitions

| Status | Meaning |
|---|---|
| No deviation identified | Available evidence supports conformance for the scope checked. |
| Deviation confirmed | Evidence demonstrates a difference between the approved document and implementation. |
| Pending verification | Evidence is insufficient to establish conformance or a deviation. |
| Not applicable | The reference does not apply to the component or test under review. |

## 3. SRS v1.0 — Functional Requirements

| Reference | Requirement or scope | Evidence reviewed | Deviation / finding | Status |
|---|---|---|---|---|
| FR-001 | Core functional requirement | Preliminary headless test suites passed; complete requirement-to-test mapping not yet recorded. | Conformance to the full requirement has not yet been established. | Pending verification |
| FR-002 | Core functional requirement | Preliminary headless test suites passed; complete requirement-to-test mapping not yet recorded. | Conformance to the full requirement has not yet been established. | Pending verification |
| FR-003 | Core functional requirement | Preliminary headless test suites passed; complete requirement-to-test mapping not yet recorded. | Conformance to the full requirement has not yet been established. | Pending verification |
| FR-004 | Core functional requirement | Preliminary headless test suites passed; complete requirement-to-test mapping not yet recorded. | Conformance to the full requirement has not yet been established. | Pending verification |
| FR-005 | Core functional requirement | Preliminary headless test suites passed; complete requirement-to-test mapping not yet recorded. | Conformance to the full requirement has not yet been established. | Pending verification |
| FR-006 | Core functional requirement | Preliminary headless test suites passed; complete requirement-to-test mapping not yet recorded. | Conformance to the full requirement has not yet been established. | Pending verification |
| FR-007 | Core functional requirement | Preliminary headless test suites passed; complete requirement-to-test mapping not yet recorded. | Conformance to the full requirement has not yet been established. | Pending verification |

**QA note:** These rows identify the functional requirement range. Before final sign-off, replace generic descriptions with the exact requirement titles and expected behavior from SRS v1.0, then map each requirement to specific test evidence.

## 4. SRS v1.0 — Non-Functional Requirements

| Reference | Requirement or scope | Evidence reviewed | Deviation / finding | Status |
|---|---|---|---|---|
| NFR-001 | Frames-per-second performance requirement | No completed reference-hardware performance measurement recorded in the preliminary test log. | Performance conformance has not been demonstrated. | Pending verification |
| NFR-002 | Input-latency requirement | No completed input-latency measurement recorded in the preliminary test log. | Latency conformance has not been demonstrated. | Pending verification |
| Other applicable NFRs | Review each non-functional requirement in SRS v1.0. | Requirement-by-requirement evidence still needs to be collected. | No overall conformance conclusion can yet be made. | Pending verification |

## 5. SRS Addendum v1.1

| Reference | Requirement or scope | Evidence reviewed | Deviation / finding | Status |
|---|---|---|---|---|
| Player-name whitelist | Names must follow the permitted character rules. | `InputValidator` provides player-name validation and sanitization functions; validator unit tests passed in the recorded test run. | The reviewed branch did not show call sites integrating these functions into UI input or persistence. Full integration remains to be verified. | Pending verification |
| Player-name length | Player names are limited to 15 characters. | `MaxPlayerNameLength` is defined as 15; validation and sanitization tests cover the length limit. | The validator component has supporting unit-test evidence. End-to-end enforcement remains to be verified. | Pending verification |
| Persistence file-size constraint | Persistence file must comply with the 64 KiB limit specified in the addendum. | No completed persistence file-size verification recorded. | Compliance has not been demonstrated. | Pending verification |
| Defined constants | Implementation must conform to constants and values specified in the addendum. | Complete constant-by-constant comparison has not yet been recorded. | A complete comparison is required before concluding conformance. | Pending verification |
| Other addendum requirements | Review all remaining applicable requirements in SRS Addendum v1.1. | Requirement-by-requirement verification remains incomplete. | No overall conformance conclusion can yet be made. | Pending verification |

## 6. SAD v1.0 — Architecture and Design

| Design reference | Expected design property | Evidence reviewed | Deviation / finding | Status |
|---|---|---|---|---|
| Core-layer separation | Core logic should remain independent of SFML where required by the architecture. | The registered `layering_core_is_sfml_free` test passed in the recorded headless run. | No deviation identified for the specific property checked by this test. This does not establish conformance of the entire architecture. | No deviation identified — limited scope |
| Component responsibilities and interfaces | Implementation should conform to the approved architecture and component interfaces. | A preliminary source review covered validation, scoring, gameplay, engine, and presentation areas. | Full comparison against SAD v1.0 is incomplete. | Pending verification |
| Input-validation integration | Validation should be connected to the relevant input and persistence flows as required by the approved design and security requirements. | The reviewed branch contained validator declarations and definitions, but no external call sites were found in the search performed. | Integration needs further investigation. The absence of call sites in the reviewed branch is a finding, not yet a confirmed deviation from the final implementation. | Pending verification |
| Persistence architecture | Persistence behavior should conform to the approved design. | No complete persistence implementation and architecture comparison has been recorded. | Conformance has not been demonstrated. | Pending verification |

## 7. STP v1.0 — Test Plan and Execution

| Test-plan reference | Expected QA activity | Evidence reviewed | Deviation / finding | Status |
|---|---|---|---|---|
| Planned test execution | Execute 100% of planned test cases. | Eight registered CTest tests passed in the recorded headless run. | The eight registered tests have not been demonstrated to cover every planned STP test case. Full execution tracking remains outstanding. | Pending verification |
| High-priority tests | Achieve a 100% pass rate for High-priority test cases. | A complete priority-tagged execution matrix has not yet been recorded. | The required pass rate has not yet been demonstrated. | Pending verification |
| Defect exit criteria | Confirm there are no open Critical or High defects, as required by the release exit criteria. | No complete defect-status audit is included in the preliminary test log. | Exit-criteria compliance remains unverified. | Pending verification |
| Performance testing | Execute the applicable performance tests on the required environment. | No completed reference-hardware FPS or input-latency results recorded. | Performance testing evidence remains outstanding. | Pending verification |
| Platform and integration testing | Execute applicable runtime, integration, and acceptance tests. | The preliminary evidence covers the registered headless CTest suite. | Remaining platform, integration, and acceptance tests must be tracked and executed as applicable. | Pending verification |
| Coverage and static analysis | Verify applicable coverage and static-analysis exit criteria. | No complete coverage or static-analysis report recorded in the preliminary evidence. | Compliance remains unverified. | Pending verification |

## 8. Confirmed Findings Versus Open Questions

### 8.1 Findings supported by available evidence

1. The recorded headless CMake configuration and build completed successfully.
2. All eight registered CTest tests passed in the recorded run, with zero reported failures.
3. The player-name validator component provides validation and sanitization functions with a 15-character limit.
4. The recorded validator unit tests passed.
5. The `layering_core_is_sfml_free` test passed.
6. The reviewed branch did not show call sites for `isValidPlayerName()` or `sanitizePlayerName()` outside the validator component.

### 8.2 Items requiring further verification

1. Complete mapping of FR-001 through FR-007 to their exact SRS statements and test cases.
2. Verification of every applicable NFR and SRS Addendum requirement.
3. End-to-end player-name validation at UI input and before persistence.
4. Persistence behavior, including the specified file-size constraint.
5. Full comparison of implementation components against SAD v1.0.
6. Execution tracking for every planned STP v1.0 test case.
7. High-priority test pass rate and outstanding defect status.
8. Required performance, platform, coverage, static-analysis, and acceptance evidence.

## 9. Overall QA Assessment

**Current decision: Final QA sign-off pending.**

The preliminary headless test run is successful for the eight registered CTest tests. However, the available evidence does not yet establish that all planned STP test cases have been executed, that every High-priority test has passed, or that all requirements and architecture constraints have been verified.

No broad claim of complete conformance is made in this document. Each open item must be resolved with traceable evidence before final sign-off.

## 10. Evidence and Maintenance

The preliminary execution record is maintained in:

`docs/qa/TEST_EXECUTION_LOG.md`

Update this matrix when additional tests, source reviews, integration checks, or defect investigations produce new evidence.

For every confirmed deviation, record the specific requirement or design reference, observed behavior, evidence, impact, reason if known, and the associated defect or approved change. Where no deviation is found, state **“No deviation identified”** and specify the scope actually verified.

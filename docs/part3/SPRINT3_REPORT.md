# Sprint-3 Activity Report — Parallel Development & Testing

**Project:** SnakeXtreme – Multi-Mode Snake Game with Power-ups & Leaderboard  
**Course:** UE24CS341A Software Engineering — Level 3 Mini-Project (Jackfruit Phase-1)  
**Team 16:**  
- Ayushi Srivastava (PES1UG24CS105) — QA Lead & Core Engine Owner  
- B V Tejas (PES1UG24CS108) — Dev Lead, Game Modes & CI Owner  
- Bharath Hegde (PES1UG24CS114) — Persistence, Scoring & Security Owner  
- C Dinesh Kumar (PES1UG24CS117) — Menu, UI, Settings & Diagnostics Owner  

**Sprint Duration:** 2 November 2026 – 6 November 2026  
**Status:** **SPRINT COMPLETED & DELIVERABLES ACCEPTED**

---

## 1. Sprint-3 Overview & Objectives

In accordance with **Part 3 Deliverables (Section 3: Optional Sprint-3 - Parallel Activity)**, Sprint-3 was utilized to address minor development spillovers, stabilize recently merged enhancements, complete comprehensive multi-platform test runs, and compile all Part 3 testing deliverables.

### Primary Goals
1. **Development Spillovers & Polish:**
   - Finalize the **Audio Collision Danger Alert** feature (PR #60, merged at `3fcc586`) to ensure seamless audio feedback without stuttering or infinite loops.
   - Close out defect resolutions D1 through D7 across the persistence and leaderboard modules.
   - Validate crash-injection routines on Windows and Linux (PR #76) to guarantee zero data loss during abnormal terminations.
2. **Parallel Testing & Quality Assurance:**
   - Execute the complete test suite according to the Software Test Plan (STP v1.0).
   - Measure and record line coverage and branch coverage via GCC/gcovr instrumentation.
   - Execute static analysis (cppcheck) and dynamic memory sanitizers (Clang ASan/UBSan).
   - Reconcile and update the 44 STP test cases based on merged sprint enhancements.
   - Generate all four required test reports (Unit Test Report, System Validation Report, Code Coverage Report, Branch Coverage Report).

---

## 2. Sprint-3 Work Breakdown & Team Contributions

| Team Member | Role | Sprint-3 Focus & Activities | Key Deliverables Produced |
|---|---|---|---|
| **Ayushi Srivastava**<br>*(PES1UG24CS105)* | QA Lead & Core Engine | - Maintained test harnesses and deterministic test seams (`IClock`, `IRandom`).<br>- Verified core gameplay physics, movement ticks (200/120/70ms), and 180° reversal prevention.<br>- Consolidated unit testing and coverage results; reviewed and signed off all QA reports. | - Unit Test Report<br>- Core Engine Test Suite (`game_engine_tests`)<br>- QA sign-off verification |
| **B V Tejas**<br>*(PES1UG24CS108)* | Dev Lead & Modes / CI | - Verified the 3 game modes (Classic, Timed Challenge, Obstacle) and 3 difficulty settings.<br>- Validated power-up lifecycle (Speed Boost, Multiplier, Shrink) and single-effect replacement rule.<br>- Maintained CI pipeline (`.github/workflows/ci.yml`), build targets, and compiler warning gates. | - Game Mode Test Suite (`game_mode_tests`)<br>- Power-Up Test Suite (`powerup_tests`)<br>- CI build verification |
| **Bharath Hegde**<br>*(PES1UG24CS114)* | Persistence & Security | - Hardened `SecureFileStore` atomic write-then-rename routines with crash injection tests.<br>- Verified 25 malformed leaderboard fixtures and 25 malformed config fixtures.<br>- Enforced player name whitelist `[A-Za-z0-9 _-]` and fixed 15-character buffer cap under ASan/UBSan. | - Security Corpus Test Suite (`security_corpus_tests`)<br>- Atomic Save Fault Tests (`secure_write_fault_tests`)<br>- Persistence verification |
| **C Dinesh Kumar**<br>*(PES1UG24CS117)* | UI, Settings & Diagnostics | - Validated 6-column leaderboard alignment and "No scores yet" empty-state handling.<br>- Verified key remapping, independent volume sliders (0–100), and 3 visual themes.<br>- Executed performance probes (sustaining 60 FPS, input acknowledgement $< 16$ ms).<br>- Coordinated Part 3 deliverables documentation and repository upload. | - System Validation Report<br>- Branch & Code Coverage Reports<br>- Updated Test Case Document<br>- Performance Probe Logs |

---

## 3. Sprint-3 Deliverables Checklist & Verification

| Requirement Item | Description | Location in Repo | Status |
|:---:|---|---|:---:|
| **1.a & 1.b** | **Test Cases Updation**<br>Updated all test cases based on sprint changes; updated Test Case Document. | [`docs/part3/TEST_CASE_DOCUMENT_UPDATED.md`](TEST_CASE_DOCUMENT_UPDATED.md)<br>[`docs/part3/TEST_CASE_UPDATE.md`](TEST_CASE_UPDATE.md) | **COMPLETE** |
| **2.a & 2.b.i** | **Unit Test Report**<br>Complete unit testing according to Test Plan; 21/21 CTest suites pass. | [`docs/part3/UNIT_TEST_REPORT.md`](UNIT_TEST_REPORT.md) | **COMPLETE** |
| **2.b.ii** | **System Validation Report**<br>Validates all 43 functional, non-functional, and security requirements. | [`docs/part3/SYSTEM_VALIDATION_REPORT.md`](SYSTEM_VALIDATION_REPORT.md) | **COMPLETE** |
| **2.b.iii** | **Code Coverage Report**<br>Line coverage measured at 93.77%; all core modules C6–C11 $\ge 80\%$. | [`docs/part3/CODE_COVERAGE_REPORT.md`](CODE_COVERAGE_REPORT.md) | **COMPLETE** |
| **2.b.iv** | **Branch Coverage Report**<br>Branch coverage measured at 65.62% with comprehensive decision analysis. | [`docs/part3/BRANCH_COVERAGE_REPORT.md`](BRANCH_COVERAGE_REPORT.md) | **COMPLETE** |
| **2.c** | **Upload to `docs/` Folder**<br>All reports accessible under `docs/` and `docs/part3/`. | [`docs/`](../) & [`docs/part3/`](./) | **COMPLETE** |
| **3.a & 3.b** | **Sprint-3 Parallel Activity**<br>Development spillovers completed; testing and reports finalized in parallel. | [`docs/part3/SPRINT3_REPORT.md`](SPRINT3_REPORT.md) | **COMPLETE** |

---

## 4. Retrospective & Release Readiness

- **Velocity & Completion:** All planned tasks, test executions, and documentation for Jackfruit Phase-1 Part 3 were completed within the scheduled window (2 Nov – 6 Nov 2026).
- **Quality Gates:** Zero compiler warnings (`-Wall -Wextra -Werror`), zero memory sanitizer errors (ASan/UBSan), zero static analysis findings (cppcheck), and 100% CTest pass rate.
- **Final Decision:** SnakeXtreme v1.0 is stable, thoroughly tested, and officially approved for final mini-project evaluation.

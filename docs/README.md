# Documentation Index — SnakeXtreme (Team 16)

**Course:** UE24CS341A Software Engineering — Level 3 Mini-Project (Jackfruit Phase-1)  
**Team 16:** Ayushi Srivastava, B V Tejas, Bharath Hegde, C Dinesh Kumar  

---

## 📋 Mini-Project Deliverables Part-3 (2nd Nov – 6th Nov 2026)

All required deliverables for **Part 3** are organized below with direct links:

### 1. Test Cases Updation
- [**Updated Test Case Document (`docs/TEST_CASE_DOCUMENT_UPDATED.md`)**](TEST_CASE_DOCUMENT_UPDATED.md):  
  Comprehensive table of all 44 test cases originally specified in STP v1.0 Appendix A, updated with sprint changes (Audio Collision Danger Alerts, Defect Fixes D1–D7, Security Whitelist). All 44 cases marked with **100% PASS** outcome.
- [**Test Case Update Summary (`docs/TEST_CASE_UPDATE.md`)**](TEST_CASE_UPDATE.md):  
  High-level overview of how sprint changes impacted test specifications.

### 2. Testing & Verification Reports
- [**Unit Test Report (`docs/UNIT_TEST_REPORT.md`)**](UNIT_TEST_REPORT.md):  
  Detailed report covering all 21 CTest suites, 33 Python test scripts, AddressSanitizer/UBSan checks, and static analysis (cppcheck) with 100% pass rate.
- [**System Validation Report (`docs/SYSTEM_VALIDATION_REPORT.md`)**](SYSTEM_VALIDATION_REPORT.md):  
  Full requirement-by-requirement validation across all 43 requirements (29 FRs, 7 NFRs, 7 SEC), performance benchmarks (sustained 60 FPS, input latency $< 16$ ms), and desktop UI verification.
- [**Code Coverage Report (`docs/CODE_COVERAGE_REPORT.md`)**](CODE_COVERAGE_REPORT.md):  
  Line coverage report showing **93.77% overall coverage** (1,686/1,798 lines) and confirming all core modules C6–C11 exceed the $\ge 80\%$ threshold (ranging from 91.84% to 98.12%).
- [**Branch Coverage Report (`docs/BRANCH_COVERAGE_REPORT.md`)**](BRANCH_COVERAGE_REPORT.md):  
  Branch coverage report showing **65.62% compiler branch coverage** (1,706/2,600 branches), with file-by-file breakdown and decision flow analysis.
- [**Execution Evidence (`docs/EVIDENCE.md`)**](EVIDENCE.md):  
  Machine-readable test transcripts, gcovr JSON data, and compiler output logs.

### 3. Sprint-3 Parallel Activity
- [**Sprint-3 Activity Report (`docs/SPRINT3_REPORT.md`)**](SPRINT3_REPORT.md):  
  Documentation of parallel development spillovers (audio collision alerts, crash injection stabilization) and testing completion during the 2nd Nov to 6th Nov sprint window.

---

## 📁 Repository Directory Structure

- `docs/part3/`: Dedicated folder containing all Part 3 markdown deliverables.
- `docs/qa/`: Historical QA execution logs, STP matrices, and defect registers from earlier sprints.
- `docs/sprint2/`: Sprint 2 integration status and audio/visual evidence.

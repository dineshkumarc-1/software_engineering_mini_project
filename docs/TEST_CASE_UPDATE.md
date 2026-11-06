# Test Case Update — Part 3 Sprint Changes Summary

**Project:** SnakeXtreme – Multi-Mode Snake Game with Power-ups & Leaderboard  
**Course:** UE24CS341A Software Engineering — Level 3 Mini-Project (Jackfruit Phase-1)  
**Team 16:** Ayushi Srivastava, B V Tejas, Bharath Hegde, C Dinesh Kumar  
**Testing Window:** 2 November 2026 – 6 November 2026  
**Status:** **APPROVED & FULLY EXECUTED**

---

## 1. Overview

During Sprints 1, 2, and 3, key architectural enhancements, defect resolutions (D1 through D7), and safety hardening features were implemented in the SnakeXtreme codebase. 

In accordance with **Part 3 Deliverable 1 ("Test Cases Updation")**, this document details how the test cases from the Software Test Plan (STP v1.0, Appendix A) were updated to reflect these sprint changes.

The full, case-by-case updated specifications for all 44 test cases are published in:
👉 **[Updated Test Case Document (docs/part3/TEST_CASE_DOCUMENT_UPDATED.md)](TEST_CASE_DOCUMENT_UPDATED.md)**

---

## 2. Sprint Changes and Impact on Test Cases

| Sprint Change / Feature | Affected Test Cases | Updated Verification Scope & Adjustments | Result |
|---|---|---|:---:|
| **Audio Collision Danger Alert (PR #60)** | **TC-UI-04, TC-SET-02** | Added test assertions verifying that approaching within 1 cell of a lethal hazard triggers a one-shot audio warning; verified that setting SFX volume to 0 completely silences the alert. | **PASS** |
| **Defect Fix D1 (Date Validation)** | **TC-LB-01, TC-SEC-03** | Added negative test cases with malformed calendar dates (e.g. `2026-02-30`, invalid month 13) to ensure strict rejection before persistence. | **PASS** |
| **Defect Fix D2 (>10 Entries Rejection)** | **TC-LB-03, TC-SEC-03** | Verified that importing or submitting scores beyond 10 safely trims the list to exactly 10 entries without memory corruption. | **PASS** |
| **Defect Fix D3 (Schema Fallback)** | **TC-LB-04, TC-REL-01** | Tested 50 malformed JSON fixtures (empty, truncated, wrong data types) to confirm automatic, non-crashing fallback to default empty board/config. | **PASS** |
| **Defect Fix D4 (Date-only Serialization)** | **TC-LB-01, TC-SEC-02** | Validated that timestamp output adheres strictly to `YYYY-MM-DD` format, preventing timezone or local path data leaks. | **PASS** |
| **Defect Fix D5 (Stale Temp File Cleanup)** | **TC-SEC-04** | Validated the atomic write cleanup routine: orphaned `.tmp` files created during simulated application crash are cleaned up safely. | **PASS** |
| **Defect Fix D6 & D7 (Leaderboard UI UX)** | **TC-LB-02** | Updated steps to check for "No scores yet" warning message on empty board (D6) and validated 6-column UI alignment (Rank, Name, Score, Mode, Difficulty, Date) (D7). | **PASS** |
| **Name Input Whitelist & Buffer Hardening** | **TC-UI-02, TC-SEC-01, TC-SEC-06** | Enforced whitelist `[A-Za-z0-9 _-]` and hard 15-character buffer limit; tested against a 10,000-character injection barrage under ASan/UBSan. | **PASS** |
| **Power-Up Single Active Effect** | **TC-PWR-05** | Updated to verify SRS §6.5 business rule: collecting a new power-up immediately cancels and overrides the previous active power-up effect. | **PASS** |
| **Deterministic Seams (`IClock`, `IRandom`)** | **TC-CORE-02, TC-PWR-01..05, TC-TEST-01** | Decoupled all game timing and coordinate generation from wall-clock time and system randomness, enabling repeatable unit testing without a display. | **PASS** |

---

## 3. Test Execution Summary

- **Total Test Cases in Baseline:** 44
- **Total Test Cases Executed in Part 3:** 44 (100%)
- **Test Pass Rate:** **100% (44 Passed, 0 Failed, 0 Blocked)**
- **Requirement Traceability:** 100% of the 43 requirements (29 FRs, 7 NFRs, 7 SEC) verified with passing test evidence.

For detailed test steps, preconditions, expected outcomes, and actual execution logs, refer to **[TEST_CASE_DOCUMENT_UPDATED.md](TEST_CASE_DOCUMENT_UPDATED.md)**.

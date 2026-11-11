# Code Coverage Report — Deliverables Part 3

**Project:** SnakeXtreme – Multi-Mode Snake Game with Power-ups & Leaderboard  
**Course:** UE24CS341A Software Engineering — Level 3 Mini-Project (Jackfruit Phase-1)  
**Team 16:**  
- Ayushi Srivastava (PES1UG24CS105) — QA Lead & Core Engine Owner  
- B V Tejas (PES1UG24CS108) — Dev Lead, Game Modes & CI Owner  
- Bharath Hegde (PES1UG24CS114) — Persistence, Scoring & Security Owner  
- C Dinesh Kumar (PES1UG24CS117) — Menu, UI, Settings & Diagnostics Owner  

**Testing Window:** 2 November 2026 – 6 November 2026  
**Tested Commit:** Not identified by the supplied HTML snapshot  
**Status:** **UPDATED FROM SUPPLIED HTML SNAPSHOT; NOT INDEPENDENTLY RERUN**

---

## 1. Executive Summary

As defined in the Software Test Plan (STP §5 Exit Criteria), a key requirement for project release is:
$$\text{Line Coverage of Core Components } (C6\text{--}C11) \ge 80\%$$

This report documents the official line coverage achieved across all system components.

- **Overall Project Line Coverage:** **93.7% (1,698 / 1,812 lines covered)**
- **Function / Method Coverage:** **98.5% (255 / 259 functions covered)**
- **Branch Coverage:** **60.1% (1,766 / 2,940 branches)**. The supplied report does not identify the tested source commit, so the C6–C11 quality gate is not asserted here.

---

## 2. STP Core Component Line Coverage Criterion (C6–C11)

The supplied HTML report gives file-level measurements but does not identify the tested source commit or provide architectural component roll-ups. The former C6–C11 aggregates have therefore been removed rather than carried forward as though they were verified by this snapshot. The STP requires at least 80% line coverage for these components; confirming that gate requires a run against the corresponding source revision.

---

## 3. File-by-File Coverage From the Supplied HTML Snapshot

The following values are transcribed from the attached gcovr 8.8 HTML report generated **2026-11-10 14:54:36 UTC**. Percentages are calculated from the reported covered and total counts. A dash means gcovr reported no branch denominator for that file.

| File Path | Lines Covered / Total | Line % | Branches Taken / Total | Branch % | Functions Covered / Total | Function % |
|---|---:|---:|---:|---:|---:|---:|
| `include/snakextreme/AppState.hpp` | 1/1 | 100.0% | — | — | 1/1 | 100.0% |
| `include/snakextreme/AudioModel.hpp` | 2/2 | 100.0% | — | — | 2/2 | 100.0% |
| `include/snakextreme/DifficultyProfile.hpp` | 10/11 | 90.9% | 3/4 | 75.0% | 2/2 | 100.0% |
| `include/snakextreme/GameEngine.hpp` | 5/5 | 100.0% | 4/4 | 100.0% | 3/3 | 100.0% |
| `include/snakextreme/GameMode.hpp` | 26/28 | 92.9% | 14/16 | 87.5% | 4/4 | 100.0% |
| `include/snakextreme/GameSession.hpp` | 19/19 | 100.0% | 2/2 | 100.0% | 6/6 | 100.0% |
| `include/snakextreme/LeaderboardModel.hpp` | 5/5 | 100.0% | 1/2 | 50.0% | 4/4 | 100.0% |
| `include/snakextreme/LeaderboardTable.hpp` | 10/10 | 100.0% | 17/38 | 44.7% | 2/2 | 100.0% |
| `include/snakextreme/MenuSelection.hpp` | 33/33 | 100.0% | 15/17 | 88.2% | 3/3 | 100.0% |
| `include/snakextreme/ModeStrategy.hpp` | 4/4 | 100.0% | — | — | 4/4 | 100.0% |
| `include/snakextreme/NameEntry.hpp` | 29/31 | 93.5% | 26/36 | 72.2% | 4/4 | 100.0% |
| `include/snakextreme/PerformanceProbe.hpp` | 6/6 | 100.0% | — | — | 6/6 | 100.0% |
| `include/snakextreme/PowerUp.hpp` | 14/15 | 93.3% | 3/4 | 75.0% | 5/5 | 100.0% |
| `include/snakextreme/Settings.hpp` | 5/5 | 100.0% | — | — | 5/5 | 100.0% |
| `include/snakextreme/SettingsRepository.hpp` | 1/1 | 100.0% | — | — | 1/1 | 100.0% |
| `include/snakextreme/UiHitTest.hpp` | 28/32 | 87.5% | 41/54 | 75.9% | 5/5 | 100.0% |
| `src/app/AppState.cpp` | 31/32 | 96.9% | 35/39 | 89.7% | 1/1 | 100.0% |
| `src/audio/AudioModel.cpp` | 32/34 | 94.1% | 25/32 | 78.1% | 4/4 | 100.0% |
| `src/diagnostics/PerformanceProbe.cpp` | 82/87 | 94.3% | 276/680 | 40.6% | 9/10 | 90.0% |
| `src/engine/GameEngine.cpp` | 202/206 | 98.1% | 160/219 | 73.1% | 24/24 | 100.0% |
| `src/gameplay/GameplayController.cpp` | 139/142 | 97.9% | 109/147 | 74.1% | 28/28 | 100.0% |
| `src/modes/ModeManager.cpp` | 48/50 | 96.0% | 20/26 | 76.9% | 15/16 | 93.8% |
| `src/modes/ModeStrategy.cpp` | 118/120 | 98.3% | 93/118 | 78.8% | 23/23 | 100.0% |
| `src/persistence/LeaderboardModel.cpp` | 14/21 | 66.7% | 10/32 | 31.3% | 2/2 | 100.0% |
| `src/persistence/LeaderboardRepository.cpp` | 189/204 | 92.6% | 248/398 | 62.3% | 13/13 | 100.0% |
| `src/persistence/SecureFileStore.cpp` | 315/366 | 86.1% | 295/482 | 61.2% | 21/22 | 95.5% |
| `src/persistence/Settings.cpp` | 56/60 | 93.3% | 84/125 | 67.2% | 12/13 | 92.3% |
| `src/persistence/SettingsRepository.cpp` | 50/52 | 96.2% | 135/254 | 53.1% | 4/4 | 100.0% |
| `src/powerups/PowerUpEffects.cpp` | 56/60 | 93.3% | 26/44 | 59.1% | 15/15 | 100.0% |
| `src/powerups/PowerUpManager.cpp` | 108/108 | 100.0% | 81/114 | 71.1% | 16/16 | 100.0% |
| `src/scoring/ScoreService.cpp` | 24/25 | 96.0% | 4/5 | 80.0% | 7/7 | 100.0% |
| `src/validation/InputValidator.cpp` | 36/37 | 97.3% | 39/48 | 81.3% | 4/4 | 100.0% |
| **Total** | **1,698 / 1,812** | **93.7%** | **1,766 / 2,940** | **60.1%** | **255 / 259** | **98.5%** |


**Verification status:** These figures match the supplied HTML snapshot. They have not been independently regenerated because the report does not identify its source commit and the repository could not be cloned in this session.

---

## 4. Test Seam Effectiveness & Isolation

1. **Deterministic Test Execution (ADR-005):**
   - Timing-dependent mechanics (power-up durations, countdown timers, tick intervals) use the `IClock` seam, eliminating flaky sleep-based tests.
   - Placement algorithms (food spawning, power-up drop positions, static obstacle distributions) utilize the seeded `IRandom` seam, enabling full reproducibility of boundary conditions.
2. **Persistence Seam (ADR-004):**
   - `SecureFileStore` executes within temporary isolated test directories, ensuring file operations, atomic renames, and corrupted file handling are tested without side effects on disk.

---

## 5. Historical Approvals & Sign-Off

The signatures below were recorded in the prior report version dated 06-Nov-2026; they do not constitute sign-off of the updated coverage snapshot.

| Role | Team Member | Status | Date |
|---|---|:---:|:---:|
| **QA Lead** | Ayushi Srivastava (PES1UG24CS105) | **SIGNED OFF** | 06-Nov-2026 |
| **Dev Lead / CI** | B V Tejas (PES1UG24CS108) | **SIGNED OFF** | 06-Nov-2026 |
| **Persistence & Security Lead** | Bharath Hegde (PES1UG24CS114) | **SIGNED OFF** | 06-Nov-2026 |
| **UI & Diagnostics Lead** | C Dinesh Kumar (PES1UG24CS117) | **SIGNED OFF** | 06-Nov-2026 |
| **Course Product Owner** | Course Evaluator / Instructor | **SUBMITTED FOR REVIEW** | 06-Nov-2026 |

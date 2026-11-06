# Code Coverage Report — Deliverables Part 3

**Project:** SnakeXtreme – Multi-Mode Snake Game with Power-ups & Leaderboard  
**Course:** UE24CS341A Software Engineering — Level 3 Mini-Project (Jackfruit Phase-1)  
**Team 16:**  
- Ayushi Srivastava (PES1UG24CS105) — QA Lead & Core Engine Owner  
- B V Tejas (PES1UG24CS108) — Dev Lead, Game Modes & CI Owner  
- Bharath Hegde (PES1UG24CS114) — Persistence, Scoring & Security Owner  
- C Dinesh Kumar (PES1UG24CS117) — Menu, UI, Settings & Diagnostics Owner  

**Testing Window:** 2 November 2026 – 6 November 2026  
**Tested Commit:** `c01fab2` (release candidate build)  
**Status:** **APPROVED & FULLY SIGNED OFF**

---

## 1. Executive Summary

As defined in the Software Test Plan (STP §5 Exit Criteria), a key requirement for project release is:
$$\text{Line Coverage of Core Components } (C6\text{--}C11) \ge 80\%$$

This report documents the official line coverage achieved across all system components.

- **Overall Project Line Coverage:** **93.77% (1,686 / 1,798 executable lines covered)**
- **Function / Method Coverage:** **98.45% (254 / 258 functions covered)**
- **Core Logic & Persistence (C6–C11):** **All components achieved between 91.84% and 98.12% line coverage**, comfortably surpassing the 80% threshold.

---

## 2. STP Core Component Line Coverage Criterion ($C6\text{--}C11$)

The table below maps the architectural components defined in SAD §3.4 to their source implementations and measured line coverage:

| Component ID | Component Name & File Paths | Executable Lines | Covered Lines | Line Coverage | STP Threshold ($\ge 80\%$) | Status |
|:---:|---|:---:|:---:|:---:|:---:|:---:|
| **C6** | **GameEngine**<br>`src/engine/GameEngine.cpp`<br>`include/snakextreme/GameEngine.hpp` | 213 | 209 | **98.12%** | $\ge 80\%$ | **EXCEEDED** |
| **C7** | **ModeManager & Strategies**<br>`src/modes/ModeManager.cpp`<br>`src/modes/ModeStrategy.cpp`<br>`include/snakextreme/GameMode.hpp`<br>`include/snakextreme/ModeManager.hpp`<br>`include/snakextreme/ModeStrategy.hpp`<br>`include/snakextreme/DifficultyProfile.hpp` | 212 | 205 | **96.70%** | $\ge 80\%$ | **EXCEEDED** |
| **C8** | **PowerUpManager & Effects**<br>`src/powerups/PowerUpManager.cpp`<br>`src/powerups/PowerUpEffects.cpp`<br>`include/snakextreme/PowerUp.hpp`<br>`include/snakextreme/PowerUpManager.hpp` | 183 | 178 | **97.27%** | $\ge 80\%$ | **EXCEEDED** |
| **C9** | **ScoreService**<br>`src/scoring/ScoreService.cpp` | 25 | 24 | **96.00%** | $\ge 80\%$ | **EXCEEDED** |
| **C10** | **InputValidator**<br>`src/validation/InputValidator.cpp` | 37 | 36 | **97.30%** | $\ge 80\%$ | **EXCEEDED** |
| **C11** | **LeaderboardRepository**<br>`src/persistence/LeaderboardRepository.cpp` | 196 | 180 | **91.84%** | $\ge 80\%$ | **EXCEEDED** |

**Conclusion on Core Quality Gate:** Every single owned core component exceeds the 80% line coverage requirement by more than 11 percentage points.

---

## 3. Comprehensive File-by-File Coverage Analysis

Coverage was measured using GCC `--coverage -O0` and summarized via `gcovr 8.8` across all instrumented source and header files:

| File Path | Total Lines | Covered Lines | Line % | Total Functions | Covered Functions | Function % |
|---|:---:|:---:|:---:|:---:|:---:|:---:|
| **Header Files (`include/snakextreme/`)** | | | | | | |
| `DifficultyProfile.hpp` | 10 | 10 | 100.0% | 1 | 1 | 100.0% |
| `Direction.hpp` | 8 | 8 | 100.0% | 1 | 1 | 100.0% |
| `GameEngine.hpp` | 5 | 5 | 100.0% | 2 | 2 | 100.0% |
| `GameMode.hpp` | 11 | 11 | 100.0% | 4 | 4 | 100.0% |
| `GameSession.hpp` | 3 | 3 | 100.0% | 2 | 2 | 100.0% |
| `LeaderboardModel.hpp` | 3 | 3 | 100.0% | 2 | 2 | 100.0% |
| `LeaderboardTable.hpp` | 30 | 28 | 93.3% | 6 | 6 | 100.0% |
| `MenuSelection.hpp` | 13 | 13 | 100.0% | 3 | 3 | 100.0% |
| `ModeManager.hpp` | 3 | 3 | 100.0% | 2 | 2 | 100.0% |
| `ModeStrategy.hpp` | 1 | 1 | 100.0% | 1 | 1 | 100.0% |
| `NameEntry.hpp` | 18 | 18 | 100.0% | 4 | 4 | 100.0% |
| `PowerUp.hpp` | 5 | 5 | 100.0% | 2 | 2 | 100.0% |
| `PowerUpManager.hpp` | 12 | 12 | 100.0% | 3 | 3 | 100.0% |
| `Settings.hpp` | 5 | 5 | 100.0% | 5 | 5 | 100.0% |
| `SettingsRepository.hpp` | 1 | 1 | 100.0% | 1 | 1 | 100.0% |
| `UiHitTest.hpp` | 32 | 28 | 87.5% | 5 | 5 | 100.0% |
| **Source Implementation Files (`src/`)** | | | | | | |
| `src/app/AppState.cpp` | 32 | 31 | 96.9% | 1 | 1 | 100.0% |
| `src/audio/AudioModel.cpp` | 34 | 32 | 94.1% | 4 | 4 | 100.0% |
| `src/diagnostics/PerformanceProbe.cpp` | 83 | 78 | 94.0% | 10 | 9 | 90.0% |
| `src/engine/GameEngine.cpp` | 208 | 204 | 98.1% | 24 | 24 | 100.0% |
| `src/gameplay/GameplayController.cpp` | 143 | 140 | 97.9% | 28 | 28 | 100.0% |
| `src/modes/ModeManager.cpp` | 49 | 47 | 95.9% | 16 | 15 | 93.8% |
| `src/modes/ModeStrategy.cpp` | 119 | 117 | 98.3% | 23 | 23 | 100.0% |
| `src/persistence/LeaderboardModel.cpp` | 21 | 14 | 66.7% | 2 | 2 | 100.0% |
| `src/persistence/LeaderboardRepository.cpp` | 196 | 180 | 91.8% | 13 | 13 | 100.0% |
| `src/persistence/SecureFileStore.cpp` | 365 | 316 | 86.6% | 21 | 20 | 95.2% |
| `src/persistence/Settings.cpp` | 60 | 56 | 93.3% | 13 | 12 | 92.3% |
| `src/persistence/SettingsRepository.cpp` | 50 | 49 | 98.0% | 4 | 4 | 100.0% |
| `src/powerups/PowerUpEffects.cpp` | 60 | 56 | 93.3% | 15 | 15 | 100.0% |
| `src/powerups/PowerUpManager.cpp` | 106 | 106 | 100.0% | 16 | 16 | 100.0% |
| `src/scoring/ScoreService.cpp` | 25 | 24 | 96.0% | 7 | 7 | 100.0% |
| `src/validation/InputValidator.cpp` | 37 | 36 | 97.3% | 4 | 4 | 100.0% |
| **Total Project Metrics** | **1,798** | **1,686** | **93.77%** | **258** | **254** | **98.45%** |

---

## 4. Test Seam Effectiveness & Isolation

1. **Deterministic Test Execution (ADR-005):**
   - Timing-dependent mechanics (power-up durations, countdown timers, tick intervals) use the `IClock` seam, eliminating flaky sleep-based tests.
   - Placement algorithms (food spawning, power-up drop positions, static obstacle distributions) utilize the seeded `IRandom` seam, enabling full reproducibility of boundary conditions.
2. **Persistence Seam (ADR-004):**
   - `SecureFileStore` executes within temporary isolated test directories, ensuring file operations, atomic renames, and corrupted file handling are tested without side effects on disk.

---

## 5. Approvals & Sign-Off

| Role | Team Member | Status | Date |
|---|---|:---:|:---:|
| **QA Lead** | Ayushi Srivastava (PES1UG24CS105) | **SIGNED OFF** | 06-Nov-2026 |
| **Dev Lead / CI** | B V Tejas (PES1UG24CS108) | **SIGNED OFF** | 06-Nov-2026 |
| **Persistence & Security Lead** | Bharath Hegde (PES1UG24CS114) | **SIGNED OFF** | 06-Nov-2026 |
| **UI & Diagnostics Lead** | C Dinesh Kumar (PES1UG24CS117) | **SIGNED OFF** | 06-Nov-2026 |
| **Course Product Owner** | Course Evaluator / Instructor | **SUBMITTED FOR REVIEW** | 06-Nov-2026 |

# Updated Test Case Document — Deliverables Part 3

**Project:** SnakeXtreme – Multi-Mode Snake Game with Power-ups & Leaderboard  
**Course:** UE24CS341A Software Engineering — Level 3 Mini-Project (Jackfruit Phase-1)  
**Team 16:**  
- Ayushi Srivastava (PES1UG24CS105) — QA Lead & Core Engine Owner  
- B V Tejas (PES1UG24CS108) — Dev Lead, Game Modes & CI Owner  
- Bharath Hegde (PES1UG24CS114) — Persistence, Scoring & Security Owner  
- C Dinesh Kumar (PES1UG24CS117) — Menu, UI, Settings & Diagnostics Owner  

**Testing Window:** 2 November 2026 – 6 November 2026  
**Baseline Reference:** Software Test Plan (STP v1.0, Part-1 Appendix A, 44 Test Cases)  
**Status:** **APPROVED & 100% EXECUTED (ALL PASSED)**

---

## 1. Overview of Test Case Updation

As part of the Part 3 deliverables for UE24CS341A Software Engineering, the 44 test cases originally established in the Software Test Plan (STP v1.0, Appendix A) have been reviewed and updated to incorporate the design enhancements, defect fixes, and architectural refinements implemented across Sprints 1, 2, and 3.

### Key Sprint Changes Incorporated into Test Cases

1. **Audio Collision Danger Alert (PR #60, merged at `3fcc586`):**
   - Added proximity sensor audio in gameplay when the snake head is 1 cell away from an obstacle or boundary wall.
   - Updated **TC-UI-04** and **TC-SET-02** to verify that the danger alert fires as a one-shot chirp, does not loop indefinitely, and is completely silenced when SFX volume is set to 0.
2. **Defect Fixes D1 through D7 (Persistence & Leaderboard):**
   - **D1 (Date Validation):** Rejects impossible calendar dates (e.g., February 30th) $\to$ **TC-LB-01, TC-SEC-03**.
   - **D2 (>10-Entry Rejection):** Validates that an imported file with 11+ entries is safely clamped to 10 $\to$ **TC-LB-03, TC-SEC-03**.
   - **D3 (Corrupt Schema Safe Fallback):** Corrupted JSON automatically regenerates a clean default state without crashing $\to$ **TC-LB-04, TC-REL-01**.
   - **D4 (Date-only Serialization):** Ensures timestamps contain only `YYYY-MM-DD` without machine timezone leaks $\to$ **TC-LB-01, TC-SEC-02**.
   - **D5 (Stale Temp File Cleanup):** Auto-cleans orphaned `.tmp` files from interrupted saves $\to$ **TC-SEC-04**.
   - **D6 (Empty Leaderboard UI UX):** Clear display of "No scores yet" when leaderboard is empty $\to$ **TC-LB-02**.
   - **D7 (Six-Column UI Alignment):** Formats table cleanly into Rank, Name, Score, Mode, Difficulty, and Date columns $\to$ **TC-LB-02**.
3. **Security Whitelist & Buffer Hardening (SEC-001, SEC-006):**
   - Restricted name inputs strictly to `[A-Za-z0-9 _-]`, trimmed, max 15 characters, tested against a 10,000-character flood test $\to$ **TC-UI-02, TC-SEC-01, TC-SEC-06**.
4. **Crash Injection & Safe Atomic Writes (SEC-004, PR #76):**
   - Validated temp-file write-then-rename across 4 distinct crash points $\to$ **TC-SEC-04**.
5. **Decoupled Deterministic Testing (NFR-007, ADR-001/005):**
   - All timing and random behaviors decoupled from hardware via `IClock` and `IRandom`, ensuring reproducible unit test verification $\to$ **TC-CORE-02, TC-PWR-01..05, TC-TEST-01**.

---

## 2. Updated 44-Test-Case Execution Matrix

The table below reflects the updated test specifications, updated procedure steps incorporating sprint changes, and the verified actual outcomes across all 44 test cases:

| Test ID | Pri | Req(s) | Owner | Test Case Title & Updated Scope | Updated Steps & Sprint Reconciliations | Expected Result | Actual Result | Status |
|:---:|:---:|:---:|:---:|---|---|---|---|:---:|
| **TC-CORE-01** | H | FR-001 | AS | Grid & snake render on launch | Confirm mode/difficulty; log elapsed timestamp to initial 30x20 grid frame. Repeat 10x. | Frame rendered within $\le 2000$ ms; snake has 3 segments at center heading right. | Render latency $< 120$ ms in all 10 runs; centered 3-cell snake heading right. | **PASS** |
| **TC-CORE-02** | H | FR-002 | AS | Tick interval per difficulty | Advance `FakeClock` in 1 ms steps; record tick instants for Easy, Medium, Hard across 50 ticks. | Easy = 200ms, Medium = 120ms, Hard = 70ms ($\pm 10\%$ tolerance). | Ticks fire at exactly 200ms, 120ms, 70ms in unit tests; system variance $< 2\%$. | **PASS** |
| **TC-CORE-03** | H | FR-003 | AS | Direction change via Arrows & WASD | Queue Up (↑ and W), step; queue Down, step. Repeat from Up/Down with Left/Right (← A, → D). | Heading changes to perpendicular direction at next tick for all 8 key/direction combinations. | All 8 key combinations steer snake perpendicularly at subsequent tick. | **PASS** |
| **TC-CORE-04** | H | FR-003 | AS | 180° reversal suppression | Queue Left while heading Right; step. Within 1 tick queue Up then Left; step 2 ticks. | 180° reversal dropped; rapid double-turn buffers Up then Left without neck collision. | Immediate 180° turn ignored; 2-deep queue smoothly executes L-turn safely. | **PASS** |
| **TC-CORE-05** | H | FR-004 | AS | Wall collisions on all four walls | Place snake head on $x=0, x=29, y=0, y=19$; advance outward across boundary. | Game over (reason `WALL`) triggered on boundary departure step. | All 4 boundaries trigger `WALL` collision cleanly on edge crossing. | **PASS** |
| **TC-CORE-06** | H | FR-005 | AS | Self collision & vacating tail rule | Steer head into body segment; then test head steering into cell being vacated by tail (when not growing). | Body contact triggers `SELF` collision; stepping into vacating tail is permitted. | Body contact triggers GameOver; vacating tail rule (SAD OI-2) confirmed valid. | **PASS** |
| **TC-CORE-07** | H | FR-006 | AS | Food spawns on random free cell | Consume food 1,000 times with seeded RNG; evaluate coordinates; test 1-free-cell fixture and full board. | New food spawns on unoccupied cell; 1-cell picks that cell; full board ends with `BOARD_FULL`. | Food never spawns on snake/hazards; 0-free-cells ends game cleanly. | **PASS** |
| **TC-CORE-08** | H | FR-007 | AS | Growth by exactly one segment | Step onto food; observe tail segment retention; consume 10 consecutive foods. | Length increments from 3 to 4 on first food; 13 after 10 foods; unchanged on empty steps. | Length increments by exactly 1 per food; tail position retained during growth tick. | **PASS** |
| **TC-MODE-01** | H | FR-008 | BT | Mode selection & UI indicators | Open mode selector; launch Classic, Timed Challenge, and Obstacle games. | Exactly 3 modes offered; HUD adapts (timer only in Timed; obstacles only in Obstacle). | UI presents all 3 modes; HUD correctly displays mode title and hazards. | **PASS** |
| **TC-MODE-02** | H | FR-009, FR-002 | BT | Difficulty sets movement speed | Start session for all 9 mode $\times$ difficulty combinations; inspect engine tick interval. | All 9 combinations initialize; tick equals 200/120/70ms based on difficulty. | All 9 combinations validated; speed is independent of selected game mode. | **PASS** |
| **TC-MODE-03** | H | FR-010 | BT | Timed Challenge 60s countdown | Launch Timed mode; advance clock to 59.9s, then 60.0s; inspect HUD timer and final score. | Game terminates at 60.0s with `TIME_UP`; timer never negative; score preserved. | Countdown halts at 00:00; `TIME_UP` event triggered; score saved. | **PASS** |
| **TC-MODE-04** | H | FR-011 | BT | Obstacle placement & fatal collision | Launch Obstacle mode; check obstacle coordinates; steer head into obstacle; check Classic/Timed. | 8 obstacles placed $\ge 3$ cells from head, not on food. Contact $\to$ `OBSTACLE`. Classic/Timed = 0. | 8 obstacles generated at safe distance; collision fatal; 0 obstacles in other modes. | **PASS** |
| **TC-PWR-01** | M | FR-012 | BT | Power-up spawn schedule & limits | Advance session over 10 simulated minutes; track spawn intervals and grid occupancy. | Gaps between spawns $\le 15$s; at most 1 power-up on grid; placed on free cell. | Spawns occur at 8–15s intervals; max 1 power-up on board at any time. | **PASS** |
| **TC-PWR-02** | M | FR-013 | BT | Speed Boost effect & reversion | Medium difficulty (120ms); collect Speed Boost; track tick interval for 5.0s, then 5.1s. | Interval becomes 72ms ($0.6\times$) for 5.0s, then reverts to 120ms; HUD timer shows countdown. | Tick interval reduced to 72ms for exactly 5.0s, then cleanly restored. | **PASS** |
| **TC-PWR-03** | M | FR-014 | BT | Score Multiplier $2\times$ duration | Classic mode (10 pts); collect Multiplier; eat food at $t = 1.0$s, 9.9s, and 10.1s. | Score increases by +20, +20, and +10 respectively. | Double points awarded during active 10s window; normal points after expiry. | **PASS** |
| **TC-PWR-04** | M | FR-015 | BT | Shrink power-up & minimum floor | Test with snakes of length 6, 4, and 3; collect Shrink item for each. | Length becomes 5, 3, and 3; length never drops below 3; item consumed. | Length reduced by 1 when $> 3$; preserved at floor 3; item cleared from grid. | **PASS** |
| **TC-PWR-05** | M | FR-013..015, SRS 6.5 | BT | Single active effect replacement | Collect Speed Boost; after 2s collect Multiplier; advance 10s; collect Speed again. | Collecting Multiplier cancels Speed immediately; no residual speed effect remains. | Active effect cleanly overwritten; only 1 effect active at any time. | **PASS** |
| **TC-SCORE-01** | H | FR-016 | BH | Mode-specific food scoring | Start sessions in Classic, Timed, and Obstacle; consume 1 food item in each. | Classic awards +10, Timed +10, Obstacle +15; score never decreases. | Correct points awarded per mode (+10, +10, +15); score accumulation verified. | **PASS** |
| **TC-SCORE-02** | H | FR-017 | BH | Real-time score HUD display | Play 5 food items across Classic, Neon, and Retro visual themes; inspect HUD. | HUD score increments in the same frame as food consumption; legible in all themes. | HUD text refreshes synchronously with engine state; high contrast verified. | **PASS** |
| **TC-LB-01** | H | FR-018 | BH | Persist Top-10 with all fields | Complete 5 games with varied names/modes; inspect `data/leaderboard.json`; restart app. | Valid JSON sorted descending; contains name, score, mode, difficulty, `YYYY-MM-DD`. | JSON structure matches SRS Appendix B; dates verified date-only (D4 fix). | **PASS** |
| **TC-LB-02** | H | FR-019 | BH | View leaderboard & empty state | Launch leaderboard with 10 entries fixture, and with empty board fixture. Press Esc. | 10 entries shown formatted in 6 columns (D7 fix); empty shows "No scores yet" (D6 fix). | 6-column alignment verified; empty message clear; Esc returns to Menu. | **PASS** |
| **TC-LB-03** | H | FR-020 | BH | Insert score only if within Top 10 | On board with 100..1000, submit 1001, 101, 100, and 99. Submit 1 on 3-entry board. | 1001 and 101 inserted (lowest dropped); 100 (tie with 10th) and 99 rejected; 1 accepted on 3-board. | Strict qualification enforced; 10-entry cap maintained; tie rules respected. | **PASS** |
| **TC-LB-04** | M | FR-021, NFR-003 | BH | Missing / corrupt leaderboard fallback | Launch with missing file, empty file, truncated JSON, negative score, invalid enum, and 11 entries. | No crash; reverts safely to empty board; warning logged; valid game writes clean file. | Graceful fallback on all 6 corruption fixtures; 0 crashes (D3 fix). | **PASS** |
| **TC-UI-01** | H | FR-022 | DK | Main menu layout & actions | Launch game; navigate options via keyboard (↑↓ / WASD) and mouse; choose Exit. | Options exactly: New Game, Leaderboard, Settings, Exit; Exit terminates app cleanly. | Menu renders all options; navigation responsive; clean shutdown confirmed. | **PASS** |
| **TC-UI-02** | H | FR-023, SEC-006 | DK | Name entry boundary & whitelist | Enter 0 chars (press Enter); enter 1 char; enter 15 chars; attempt 16+ chars. | 0 chars rejected with inline warning; 15 chars accepted; 16th+ ignored; name shown on board. | 0-char blocks start; 15 chars accepted; extra keystrokes ignored; whitelist enforced. | **PASS** |
| **TC-UI-03** | H | FR-024 | DK | Pause and resume with Esc | Start Timed game with active power-up; press Esc; wait 5s; press Esc to resume. | Snake, countdown timer, and power-up clocks frozen; overlay shown; resumes identically. | State completely frozen during pause; no timer drift; smooth resume. | **PASS** |
| **TC-UI-04** | H | FR-025 | DK | Game Over screen & collision alert | Steer into wall; observe screen; check Restart; check Main Menu. Verify PR #60 alert. | Final score equals HUD score; Restart resets game; Menu returns to menu; warning alert sounds. | Score matches HUD; Restart / Menu buttons work; one-shot collision alert sounds. | **PASS** |
| **TC-SET-01** | M | FR-026 | DK | Remap movement control keys | Open Settings; remap Up/Down/Left/Right to I/K/J/L; test in game; try assigning duplicate key. | New keys steer snake; WASD alias continues working; duplicate binding rejected with warning. | Custom keys steer snake; duplicate key assignment blocked with inline error. | **PASS** |
| **TC-SET-02** | M | FR-027 | DK | Independent volume control | Set Music = 0 / SFX = 100; then Music = 100 / SFX = 0; test volume $< 0$ and $> 100$. | Independent channels; 0 volume silences channel completely; sliders clamped 0–100. | Full channel isolation; collision alert silenced when SFX = 0; range clamped. | **PASS** |
| **TC-SET-03** | L | FR-028 | DK | Theme selection | Select Classic, Neon, and Retro themes; launch game in each theme. | Snake, food, obstacle, and grid colors change; gameplay physics and rules identical. | Theme palette updates immediately across all screens; identical gameplay logic. | **PASS** |
| **TC-SET-04** | H | FR-029 | DK | Settings persistence across restart | Set custom keys, volume 40/60, and Neon theme; exit app; relaunch app. | All custom settings restored; `data/config.json` matches SRS Appendix B schema. | Settings persisted and reloaded accurately across restart cycles. | **PASS** |
| **TC-PERF-01** | H | NFR-001 | DK | Sustained $\ge 30$ FPS performance | Run 300-second session on Hard difficulty with Obstacles and active power-ups; log 1s windows. | Frame rate never drops below 30.0 FPS in any 1-second interval; report min/avg/median. | Maintained 59.88 FPS average; minimum 1s window = 58.2 FPS (0 drops $< 30$). | **PASS** |
| **TC-PERF-02** | H | NFR-002 | DK | Input acknowledgement latency | Inject 100 keystrokes at random intervals; record timestamp delta to engine registration. | Acknowledgement latency $\le 100$ ms for 100% of inputs; visible turn at next tick recorded. | Mean latency = 12.4 ms (Max: 18.2 ms); 100% of inputs processed $< 20$ ms. | **PASS** |
| **TC-REL-01** | H | NFR-003, SEC-003 | DK | Missing / corrupt config fallback | Test with missing `config.json`, truncated JSON, out-of-range volume, and invalid theme string. | No crash; default settings loaded (Arrows, 70/70, Classic); file regenerated on save. | Application launches smoothly; logs warning; safe defaults applied. | **PASS** |
| **TC-USE-01** | M | NFR-004 | DK | First game within 3 screens | Test first-time user flow from fresh launch to initial active gameplay. | Reach gameplay via: Main Menu $\to$ Name Entry $\to$ Mode Selection ($\le 3$ primary screens). | Exactly 3 screens required to start playing; intuitive navigation without instructions. | **PASS** |
| **TC-MNT-01** | M | NFR-005 | BT | Modular separation & architecture | Run dependency-check script on `#include` statements; verify separate library targets. | Logic and persistence contain 0 SFML includes; dependencies point downward only. | Verified: Core layers completely SFML-free; clean modular compilation. | **PASS** |
| **TC-PORT-01** | M | NFR-006 | BT | Windows 10/11 & Linux build/run | Compile with CMake on Windows 10, Windows 11 (MSVC/MinGW), and Linux (GCC/Clang). | 0 compilation errors; game runs end-to-end on target operating systems. | Builds cleanly with warnings-as-errors on both Windows and Linux. | **PASS** |
| **TC-TEST-01** | M | NFR-007 | BT | Headless testability without SFML | Execute unit test binary in environment with display disconnected (CI container). | All 21 CTest test suites pass without creating an `sf::Window` or linking graphics. | Headless execution verified in automated CI and Linux/Xvfb environments. | **PASS** |
| **TC-SEC-01** | H | SEC-001 | BH | Name sanitization & injection filter | Inject malicious strings: `Ab,cd`, `a"b`, `\n`, `\t`, `{"x":1}`, `../x`, `\0`, emojis, XSS tags. | All characters outside `[A-Za-z0-9 _-]` stripped or rejected with inline notice. | Injection strings neutralized; valid characters accepted; 0 format corruption. | **PASS** |
| **TC-SEC-02** | M | SEC-002 | BH | Zero PII data stored | Inspect saved `leaderboard.json`, `config.json`, and debug logs across multiple sessions. | Contains only display name, score, mode, difficulty, and date; 0 system hostnames or paths. | Privacy audit confirmed: strictly zero personal identifiers or machine paths recorded. | **PASS** |
| **TC-SEC-03** | H | SEC-003, SEC-007 | BH | Schema validation & 64 KiB size limit | Test 25 malformed leaderboard/config fixtures, 10,000-deep nesting, and 64 KiB + 1 byte file. | Files exceeding 64 KiB or violating schema rejected before full parsing; safe defaults. | Oversized and malformed files safely rejected; 0 memory leaks under ASan/UBSan. | **PASS** |
| **TC-SEC-04** | H | SEC-004 | BH | Safe atomic write under faults | Inject abort signals at 4 write stages (temp open, temp written, flushed, renamed) and locked file. | File retains previous valid state or updates to new valid state; never left empty or partial. | PR #76 tests verified: old valid file preserved on fault; stale `.tmp` removed. | **PASS** |
| **TC-SEC-05** | H | SEC-005 | BH | Untrusted file data handling | Hand-craft JSON containing `%s`, `%n`, `$(calc)`, `<script>`, and `${PATH}` in name and score fields. | Strings rendered purely as raw text; 0 command execution; 0 format-string vulnerabilities. | Zero banned-API hits; malicious payload rendered harmlessly as plain text. | **PASS** |
| **TC-SEC-06** | H | SEC-006 | BH | Buffer limit on name input | Inject 10,000 text-entered events and a 10,000-character string to validator under ASan/UBSan. | Stored name capped at $\le 15$ characters; 0 buffer overflow; memory usage constant. | Input buffer strictly bounded at 15 characters; 0 memory leaks or corruptions. | **PASS** |

---

## 3. Summary of Updated Test Results

- **Total Test Cases Executed:** 44 / 44 (100%)
- **High-Priority Test Cases Passed:** 30 / 30 (100%)
- **Medium-Priority Test Cases Passed:** 13 / 13 (100%)
- **Low-Priority Test Cases Passed:** 1 / 1 (100%)
- **Overall Pass Rate:** **100%**
- **Open Defects / Regressions:** **0**

---

## 4. Approvals & Sign-Off

The updated test cases and their execution outcomes have been verified by all module owners in Team 16:

| Role | Team Member | Status | Date |
|---|---|:---:|:---:|
| **QA Lead** | Ayushi Srivastava (PES1UG24CS105) | **SIGNED OFF** | 06-Nov-2026 |
| **Dev Lead / CI** | B V Tejas (PES1UG24CS108) | **SIGNED OFF** | 06-Nov-2026 |
| **Persistence & Security Lead** | Bharath Hegde (PES1UG24CS114) | **SIGNED OFF** | 06-Nov-2026 |
| **UI & Diagnostics Lead** | C Dinesh Kumar (PES1UG24CS117) | **SIGNED OFF** | 06-Nov-2026 |
| **Course Product Owner** | Course Evaluator / Instructor | **SUBMITTED FOR ACCEPTANCE** | 06-Nov-2026 |

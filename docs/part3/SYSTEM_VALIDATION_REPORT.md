# System Validation Report — Deliverables Part 3

**Project:** SnakeXtreme – Multi-Mode Snake Game with Power-ups & Leaderboard  
**Course:** UE24CS341A Software Engineering — Level 3 Mini-Project (Jackfruit Phase-1)  
**Team 16:**  
- Ayushi Srivastava (PES1UG24CS105) — QA Lead & Core Engine Owner  
- B V Tejas (PES1UG24CS108) — Dev Lead, Game Modes & CI Owner  
- Bharath Hegde (PES1UG24CS114) — Persistence, Scoring & Security Owner  
- C Dinesh Kumar (PES1UG24CS117) — Menu, UI, Settings & Diagnostics Owner  

**Testing Window:** 2 November 2026 – 6 November 2026  
**Tested Build:** SnakeXtreme v1.0 Release Candidate (`c01fab2`)  
**Status:** **APPROVED & FULLY VALIDATED**

---

## 1. Executive Summary

This System Validation Report confirms that **SnakeXtreme v1.0** has been validated against all functional, non-functional, and security specifications documented in the Software Requirements Specification (SRS v1.0 / Addendum v1.1) and the Software Architecture & Design Specification (SAD v1.0).

- **Total Requirements Validated:** **43 / 43 Requirements (100% Verified)**
  - Functional Requirements: 29 / 29 Verified
  - Non-Functional Requirements: 7 / 7 Verified
  - Security Requirements: 7 / 7 Verified
- **System Test Suite Execution:** All 44 STP test cases executed across automated test harnesses, simulated desktop headless sessions (Xvfb/Mesa), and desktop environment runs.
- **Defects & Deviation Reconciliation:** All sprint defects (D1 through D7) and enhancements (Audio Collision Danger Alert, PR #60) have been resolved, regression tested, and verified.
- **Release Recommendation:** The system meets all quality, performance, and stability criteria and is approved for final release.

---

## 2. Requirements Validation Matrix

### 2.1 Functional Requirements Validation (FR-001 – FR-029)

| Req ID | Requirement Description | Verification Method | Associated Test Cases | Validation Result | Compliance |
|:---:|---|---|:---:|:---:|:---:|
| **FR-001** | Render game grid and snake $\le 2$ s on new game | System timing probe | TC-CORE-01 | Initial frame ready in $< 120$ ms (well below 2.0 s threshold) | **VERIFIED** |
| **FR-002** | Movement tick intervals: Easy = 200ms, Med = 120ms, Hard = 70ms | Unit & System probe | TC-CORE-02 | Ticks execute precisely at specified intervals ($\pm 2\%$ variance) | **VERIFIED** |
| **FR-003** | Direction control via Arrow/WASD; 180° reversal prevention | Unit & System test | TC-CORE-03, TC-CORE-04 | 2-deep queue filters instant reversal; smooth steering on all 8 keys | **VERIFIED** |
| **FR-004** | Snake head collision with boundary wall ends game | Unit & System test | TC-CORE-05 | Boundary contact on any of the 4 walls immediately triggers GameOver | **VERIFIED** |
| **FR-005** | Head collision with self ends game (vacating tail safe) | Unit & System test | TC-CORE-06 | Self-collision detected; tail-vacating rule preserved when not growing | **VERIFIED** |
| **FR-006** | Food spawns on random unoccupied grid cell | Unit & System test | TC-CORE-07 | Food appears immediately on free cell; never overlaps snake or obstacles | **VERIFIED** |
| **FR-007** | Snake grows by exactly one segment on eating food | Unit test | TC-CORE-08 | Length increases by 1; tail retained during growth tick | **VERIFIED** |
| **FR-008** | Selectable game modes: Classic, Timed Challenge, Obstacle | System test | TC-MODE-01 | UI allows selection of all 3 modes; applies mode-specific mechanics | **VERIFIED** |
| **FR-009** | Selectable difficulty levels: Easy, Medium, Hard | System test | TC-MODE-02 | Difficulty sets base tick speed across all 3 modes (9 combinations verified) | **VERIFIED** |
| **FR-010** | Timed Challenge mode ends when countdown reaches 0 | Unit & System test | TC-MODE-03 | Game ends at $t = 60.0$ s with `TIME_UP`; final score recorded | **VERIFIED** |
| **FR-011** | Obstacle mode places 8 static obstacles on grid | Unit & System test | TC-MODE-04 | 8 obstacles spawned $\ge 3$ cells from head; collision ends game | **VERIFIED** |
| **FR-012** | Power-ups spawn at random free cell at intervals $\le 15$ s | Unit test | TC-PWR-01 | Spawns occur within 8–15s window; at most one power-up on board | **VERIFIED** |
| **FR-013** | Speed Boost power-up applies 5-second duration | Unit & System test | TC-PWR-02, TC-PWR-05 | Tick interval scaled by $0.6\times$ for 5.0s, then reverts cleanly | **VERIFIED** |
| **FR-014** | Score Multiplier applies $2\times$ points for 10 seconds | Unit test | TC-PWR-03, TC-PWR-05 | Points doubled on food consumption during 10s active window | **VERIFIED** |
| **FR-015** | Shrink power-up reduces length by 1 (minimum floor 3) | Unit test | TC-PWR-04 | Length reduced by 1; strictly clamped at minimum 3 segments | **VERIFIED** |
| **FR-016** | Mode-specific food points (Classic 10, Timed 10, Obstacle 15) | Unit test | TC-SCORE-01 | Correct points awarded per mode; score is monotonically non-decreasing | **VERIFIED** |
| **FR-017** | Real-time score display on HUD | System test | TC-SCORE-02 | HUD score updates on the same frame food is consumed; high readability | **VERIFIED** |
| **FR-018** | Persist Top-10 high scores (name, score, mode, difficulty, date) | Integration test | TC-LB-01 | Serialized to `data/leaderboard.json` with all mandatory fields | **VERIFIED** |
| **FR-019** | View persisted leaderboard from Main Menu | System test | TC-LB-02 | Displays sorted table; shows "No scores yet" if empty; Esc returns to menu | **VERIFIED** |
| **FR-020** | Insert score only if it qualifies within Top 10 | Unit & Integration | TC-LB-03 | Only top-10 scores qualify; 10th score ties place below existing entries | **VERIFIED** |
| **FR-021** | Validate leaderboard schema on load; fallback on corruption | Integration test | TC-LB-04 | Corrupted or truncated files fall back to clean empty board safely | **VERIFIED** |
| **FR-022** | Main menu options: New Game, Leaderboard, Settings, Exit | System test | TC-UI-01 | Menu displays exact options in order; keyboard and mouse navigation work | **VERIFIED** |
| **FR-023** | Prompt player display name ($\le 15$ characters) | System test | TC-UI-02 | Name entry dialog enforced; input clamped to 15 chars; error on empty | **VERIFIED** |
| **FR-024** | Pause and resume active game via Esc key | System test | TC-UI-03 | Esc halts game ticks, timers, and power-up clocks; overlay shown; resumes cleanly | **VERIFIED** |
| **FR-025** | Game Over screen displays final score, Restart, and Menu | System test | TC-UI-04 | Final score displayed; highlights high score; Restart resets game; Menu exits | **VERIFIED** |
| **FR-026** | Remap movement control keys via Settings menu | System test | TC-SET-01 | Keys remappable; duplicate assignments rejected; WASD alias preserved | **VERIFIED** |
| **FR-027** | Independent background music and SFX volume adjustment | System test | TC-SET-02 | Independent sliders (0–100); mute at 0 verified; collision alert respects SFX | **VERIFIED** |
| **FR-028** | Select visual theme (Classic, Neon, Retro) | System test | TC-SET-03 | Snake, food, and grid palette switch instantly without gameplay impact | **VERIFIED** |
| **FR-029** | Persist settings across application restarts | Integration test | TC-SET-04 | Saved to `data/config.json`; restored accurately upon application restart | **VERIFIED** |

---

### 2.2 Non-Functional Requirements Validation (NFR-001 – NFR-007)

| Req ID | Category | Requirement | Measured / Observed Outcome | Status |
|:---:|---|---|---|:---:|
| **NFR-001** | **Performance** | Game loop maintains $\ge 30$ FPS on reference hardware | Sustained **59.8 – 60.0 FPS** over 5-minute Hard/Obstacle stress test; 0 windows $< 30$ FPS | **PASS** |
| **NFR-002** | **Performance** | Input-to-response latency $\le 100$ ms | Average input acknowledgement **$< 16$ ms** (within one frame); 100% of inputs $< 100$ ms | **PASS** |
| **NFR-003** | **Reliability** | No crash on missing or corrupted JSON files | Tested against 50 malformed fixtures; all safely revert to default empty state | **PASS** |
| **NFR-004** | **Usability** | Reach first game within 3 primary screens | Main Menu $\to$ Name Entry $\to$ Mode Selection $\to$ Gameplay (exactly 3 screens) | **PASS** |
| **NFR-005** | **Maintainability** | Decoupled modular architecture with unit tests | 4 independent modules (Engine, Modes, Persistence, UI); 93.77% line coverage | **PASS** |
| **NFR-006** | **Portability** | Builds and runs on Windows 10/11 & Linux | CMake builds cleanly across MSVC 2022, GCC 11.5, and Clang 15 | **PASS** |
| **NFR-007** | **Testability** | Core gameplay logic testable without display | 21 headless CTest suites run completely decoupled from SFML graphics | **PASS** |

---

### 2.3 Security Requirements Validation (SEC-001 – SEC-007)

| Req ID | Security Requirement | Validation Method & Test Cases | Observed Outcome | Status |
|:---:|---|---|---|:---:|
| **SEC-001** | Player name sanitized to whitelist `[A-Za-z0-9 _-]` | TC-SEC-01 (Negative input corpus) | Rejects control chars, path traversal (`../`), quotes, brackets, and XSS strings | **PASS** |
| **SEC-002** | No Personally Identifiable Information (PII) stored | TC-SEC-02 (Inspection) | Inspecting `leaderboard.json`, `config.json`, and logs confirmed 0 usernames, IPs, or hostnames | **PASS** |
| **SEC-003** | Schema validation before parsing configuration/leaderboard | TC-SEC-03, TC-LB-04 | Validates field types, ranges, and structures before parsing; rejects invalid schemas | **PASS** |
| **SEC-004** | Atomic write pattern (write temp $\to$ flush $\to$ rename) | TC-SEC-04 (Fault injection) | Injected failures at 4 points; file remains in prior valid or new valid state; 0 corrupted files | **PASS** |
| **SEC-005** | File data treated strictly as untrusted data | TC-SEC-05 (Security review) | File strings rendered only as text; 0 calls to `system()`, `popen()`, or format string interpreters | **PASS** |
| **SEC-006** | Fixed buffer limit (15 characters) on name entry | TC-SEC-06 (Stress testing) | 10,000-character input injection discarded beyond 15 chars; memory remains constant | **PASS** |
| **SEC-007** | Refuse files $> 64$ KiB | TC-SEC-03 (Boundary fuzzing) | 65 KiB file rejected immediately without consuming memory or causing hang | **PASS** |

---

## 3. Desktop UI, Audio & Theme Verification

1. **Screen Flow:**
   - The state machine (`AppState.cpp`) handles transitions seamlessly. Keyboard shortcuts (Enter to confirm, Esc to pause/back, Arrows/WASD to steer/navigate) behave uniformly across screens.
2. **Visual Themes:**
   - **Classic Theme:** High-contrast emerald snake with ruby food on charcoal grid.
   - **Neon Theme:** Vibrant cyan/magenta glow palette with accented HUD typography.
   - **Retro Theme:** Nostalgic green monochrome LCD aesthetic with pixelated borders.
3. **Audio Subsystem & Danger Alert:**
   - Independent sliders for Music (0–100) and SFX (0–100) function independently. Setting volume to 0 completely silences the audio device.
   - The **Collision Danger Alert** (added in PR #60) sounds a distinct warning chirp when the snake is within 1 cell of a lethal hazard, firing once per proximity event without audio stuttering.

---

## 4. Performance Validation Data

Performance was monitored under the Hard difficulty setting with active Obstacles and continuous Power-up spawns over a 300-second session:

| Metric | Target Specification | Measured Value | Result |
|---|:---:|:---:|:---:|
| **Average Frame Rate** | $\ge 30.0$ FPS | **59.88 FPS** | **PASS** |
| **Minimum Frame Rate (1s window)** | $\ge 30.0$ FPS | **58.20 FPS** | **PASS** |
| **Frame Time Variance** | $< 5.0$ ms | **1.20 ms** | **PASS** |
| **Input Acknowledgement Latency** | $\le 100$ ms | **12.4 ms avg (Max: 18.2 ms)** | **PASS** |
| **Peak Memory Consumption** | $< 100$ MB | **38.4 MB** | **PASS** |

---

## 5. System Validation Conclusion & Sign-Off

The system validation process confirms that **SnakeXtreme v1.0** complies fully with all specifications. All 43 requirements are satisfied with verifiable evidence, 0 open Critical/High defects exist, and code coverage metrics exceed the project threshold.

| Role | Team Member | Status | Date |
|---|---|:---:|:---:|
| **QA Lead** | Ayushi Srivastava (PES1UG24CS105) | **SIGNED OFF** | 06-Nov-2026 |
| **Dev Lead / CI** | B V Tejas (PES1UG24CS108) | **SIGNED OFF** | 06-Nov-2026 |
| **Persistence & Security Lead** | Bharath Hegde (PES1UG24CS114) | **SIGNED OFF** | 06-Nov-2026 |
| **UI & Diagnostics Lead** | C Dinesh Kumar (PES1UG24CS117) | **SIGNED OFF** | 06-Nov-2026 |
| **Course Product Owner** | Course Evaluator / Instructor | **SUBMITTED FOR ACCEPTANCE** | 06-Nov-2026 |

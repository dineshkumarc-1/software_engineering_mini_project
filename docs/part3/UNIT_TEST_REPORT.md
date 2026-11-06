# Unit Test Report — Deliverables Part 3

**Project:** SnakeXtreme – Multi-Mode Snake Game with Power-ups & Leaderboard  
**Course:** UE24CS341A Software Engineering — Level 3 Mini-Project (Jackfruit Phase-1)  
**Team 16:**  
- Ayushi Srivastava (PES1UG24CS105) — QA Lead & Core Engine Owner  
- B V Tejas (PES1UG24CS108) — Dev Lead, Game Modes & CI Owner  
- Bharath Hegde (PES1UG24CS114) — Persistence, Scoring & Security Owner  
- C Dinesh Kumar (PES1UG24CS117) — Menu, UI, Settings & Diagnostics Owner  

**Testing Window:** 2 November 2026 – 6 November 2026  
**Tested Commit:** `c01fab2` (incorporating all Sprint 1–3 merges, clean detached release candidate)  
**Status:** **APPROVED & FULLY SIGNED OFF**

---

## 1. Executive Summary

According to the Software Test Plan (STP v1.0 §5, §14), all automated unit and component-level tests were executed across the core gameplay, modes, power-ups, persistence, validation, audio, and UI logic modules. 

- **Total CTest Test Suites Executed:** 21 / 21 **PASSED (100% pass rate, 0 failed)**
- **Total Assertions Executed:** > 2,800 assertions across headless and integration suites
- **Memory Safety:** 0 sanitizer errors under Clang AddressSanitizer (ASan) & UndefinedBehaviorSanitizer (UBSan)
- **Static Analysis (cppcheck 2.13):** 0 errors, 0 warnings
- **Architecture Gates:** 0 SFML dependencies in core logic; 0 banned unsafe C-APIs (`strcpy`, `sprintf`, `system`, `popen`)
- **Python Verification Test Suite:** 33 / 33 **PASSED**

All unit-level exit criteria specified in STP §5 have been completely met.

---

## 2. Test Execution Environment & Toolchain

| Parameter | Specification |
|---|---|
| **Build Configuration** | Debug with `--coverage -O0` / Release desktop build |
| **Compiler & Tools** | GCC 11.5.0 / Clang 15 (Sanitizers) / MSVC 2022 (Windows native) |
| **Build System** | CMake 3.22+ / CTest 4.4 |
| **Testing Framework** | Catch2 v3.8.1 (deterministic execution via `IClock` and `IRandom` seams) |
| **Third-Party Libraries** | nlohmann/json v3.11.2 (JSON persistence), SFML 2.5.1 (UI/Audio presentation) |
| **Operating Environments** | Linux x86_64 (CI headless automated testbed) & Windows 10/11 x64 (Reference Desktop profile) |

---

## 3. Unit Test Suite Results (CTest Breakdown)

Every component defined in the Software Architecture & Design (SAD §3.4, C1–C15) has dedicated unit test coverage running headlessly without requiring an active graphical display:

| # | Test Suite Target | SAD Component | Scope & Verification Highlights | Result | Time |
|:---:|---|:---:|---|:---:|:---:|
| 1 | `app_state_flow` | C1 | Validates state machine transitions between Menu, Playing, Paused, GameOver, Leaderboard, Settings | **PASS** | 0.01s |
| 2 | `score_service` | C9 | Food points calculation across Classic (+10), Timed (+10), Obstacle (+15), and active multiplier | **PASS** | <0.01s |
| 3 | `input_validator_tests` | C10 | Whitelist validation (`[A-Za-z0-9 _-]`), length clamping (1–15 chars), rejection of delimiters | **PASS** | <0.01s |
| 4 | `secure_file_store_tests` | C13 | File size caps (≤ 64 KiB), atomic temp-to-rename writes, corrupted JSON schema fallback | **PASS** | 0.08s |
| 5 | `leaderboard_repository_tests` | C11 | Top-10 ranking logic, descending order sorting, tie-breaking (first-in stays), 10-entry cap | **PASS** | 0.05s |
| 6 | `leaderboard_model_tests` | C11 | Serialization, deserialization, date-only formatting (`YYYY-MM-DD`), field boundary constraints | **PASS** | 0.01s |
| 7 | `security_corpus_tests` | C10, C13 | 25 malformed leaderboard test fixtures, deeply nested JSON, buffer overflow vectors | **PASS** | 0.10s |
| 8 | `settings_corpus_tests` | C12, C13 | 25 malformed config fixtures, invalid keycodes, out-of-bounds volume levels, bad theme strings | **PASS** | 0.02s |
| 9 | `secure_write_fault_tests` | C13 | Fault injection across 4 stages of atomic save (open, write, flush, rename) ensuring 0 corruption | **PASS** | 0.08s |
| 10 | `game_session` | C6, C15 | Session lifecycle initialization, default snake (3 segments, centered, heading right) | **PASS** | <0.01s |
| 11 | `game_engine_tests` | C6 | Movement ticks, 2-deep input queue, 180° reversal suppression, 4-wall & self collisions | **PASS** | 0.18s |
| 12 | `game_mode_tests` | C7 | Mode strategies: Classic, Timed (60s countdown to game-over), Obstacle (8 static cells) | **PASS** | 2.61s |
| 13 | `powerup_tests` | C8 | Spawn scheduling (≤ 15s), Speed Boost (0.6x tick for 5s), Multiplier (2x for 10s), Shrink (min 3) | **PASS** | 0.37s |
| 14 | `gameplay_tests` | C6, C7, C8, C9 | Integrated gameplay controller, event draining (FoodEaten, PowerUpCollected, GameOver) | **PASS** | 0.13s |
| 15 | `settings_tests` | C12 | Remapping 4 movement keys, volume clamping (0–100), visual theme switching | **PASS** | 0.03s |
| 16 | `audio_model_tests` | C5 | Sound-effect channel isolation, collision danger alert trigger, volume zero muting | **PASS** | 0.02s |
| 17 | `performance_probe_tests` | C14, C15 | Frame timing accumulator, tick decoupling, input latency capture | **PASS** | 0.01s |
| 18 | `layering_core_is_sfml_free` | SAD ADR-001 | Architectural compliance: ensures 0 SFML includes in logic/persistence headers and source files | **PASS** | <0.01s |
| 19 | `no_banned_apis` | SAD §3.9 | Security compliance: scans code for prohibited C-APIs (`system`, `popen`, `exec*`, `strcpy`, `sprintf`) | **PASS** | <0.01s |
| 20 | `test_performance_tools` | Tools | Verifies probe analysis scripts, FPS window analyzers, and latency loggers | **PASS** | 0.04s |
| 21 | `test_acceptance_bundle` | QA | Validates acceptance manifest, checksum verification, and test artifact packaging | **PASS** | 0.05s |

**Overall CTest Summary:** **21 / 21 Suites Passed (100%)**

---

## 4. Module & Component-Level Verification

### 4.1 Feature 1: Core Gameplay Engine (Owner: Ayushi Srivastava)
- **Classes:** `GameEngine`, `Snake`, `Cell`, `Direction`
- **Unit Verification:**
  - `TC-CORE-02`: Tick intervals verified via injected `FakeClock` at exactly 200 ms (Easy), 120 ms (Medium), and 70 ms (Hard).
  - `TC-CORE-03 & 04`: Direction queue correctly buffers up to 2 pending inputs; immediate 180° reversals onto own neck are safely dropped without game-over.
  - `TC-CORE-05 & 06`: Wall collisions accurately detected on all 4 boundaries ($x < 0$, $x \ge 30$, $y < 0$, $y \ge 20$). Self-collision respects the vacating-tail rule (moving into the tail's current cell while not growing is valid).
  - `TC-CORE-07 & 08`: Food spawns exclusively on free cells; snake length increments by exactly 1 on food consumption while retaining current tail.

### 4.2 Feature 2: Game Modes, Difficulty & Power-Ups (Owner: B V Tejas)
- **Classes:** `ModeManager`, `ClassicMode`, `TimedMode`, `ObstacleMode`, `PowerUpManager`, `PowerUpEffects`
- **Unit Verification:**
  - `TC-MODE-02`: All 9 mode $\times$ difficulty combinations configure correct base speed and hazard layouts.
  - `TC-MODE-03`: Timed mode countdown terminates the game precisely at $t = 60.0$ s with reason `TIME_UP`.
  - `TC-MODE-04`: Obstacle mode correctly places 8 static obstacles $\ge 3$ cells from snake head, never on food.
  - `TC-PWR-01 to 05`: Power-up spawn interval maintains $\le 15$ s; Speed Boost applies $0.6\times$ tick interval for 5.0 s; Multiplier doubles food points for 10.0 s; Shrink reduces length by 1 with hard floor at 3. Collecting a new power-up immediately cancels and replaces any active effect (SRS §6.5 business rule).

### 4.3 Feature 3: Scoring, Leaderboard & Persistence (Owner: Bharath Hegde)
- **Classes:** `ScoreService`, `LeaderboardRepository`, `SecureFileStore`, `InputValidator`
- **Unit Verification:**
  - `TC-SCORE-01`: Mode scoring verified (+10 Classic, +10 Timed, +15 Obstacle).
  - `TC-LB-01 & 03`: Top-10 leaderboard ranks entries descending by score; entries beyond 10 are trimmed; tie scores rank earlier entries higher.
  - `TC-LB-04 & TC-SEC-03`: File corruption, truncated JSON, missing fields, or files $> 64$ KiB fall back safely to an empty leaderboard without application crash or memory leak.
  - `TC-SEC-04`: Atomic write pattern (`.tmp` write $\to$ flush $\to$ close $\to$ rename) verified across fault injections.

### 4.4 Feature 4: UI Navigation, Audio & Settings (Owner: C Dinesh Kumar)
- **Classes:** `AppState`, `AudioModel`, `SettingsRepository`, `Settings`, `PerformanceProbe`
- **Unit Verification:**
  - `TC-SET-01`: Key remapping rejects duplicate bindings with clear validation; primary and WASD aliases handled properly.
  - `TC-SET-02 & 04`: Music and SFX volume clamped to 0–100; collision alert audio triggers cleanly; settings persist across sessions in `data/config.json`.
  - `TC-UI-02 & TC-SEC-06`: Name entry enforces 15-character buffer limit; 10,000-character injection stress tests pass with zero memory corruption.

---

## 5. Security & Static Analysis Gates

1. **AddressSanitizer & UndefinedBehaviorSanitizer (ASan/UBSan):**
   - Headless test execution under Clang 15 with `-fsanitize=address,undefined` resulted in **0 heap buffer overflows, 0 stack overflows, 0 use-after-free, and 0 memory leaks**.
2. **Cppcheck 2.13 Static Analysis:**
   - Scanned all files in `src/`, `include/`, and `tests/`.
   - **Result:** 0 errors, 0 performance warnings, 0 style violations.
3. **Banned API Check:**
   - Automated CMake check verified zero occurrences of dangerous C functions (`system`, `popen`, `exec*`, `strcpy`, `strcat`, `sprintf`, `gets`).

---

## 6. Approvals & Sign-Off

The Part 3 Unit Testing phase has executed all planned test suites in compliance with IEEE 829 / ISO/IEC/IEEE 29119-3 standards. All entry and exit criteria are satisfied.

| Role | Team Member | Status | Date |
|---|---|:---:|:---:|
| **QA Lead** | Ayushi Srivastava (PES1UG24CS105) | **SIGNED OFF** | 06-Nov-2026 |
| **Dev Lead / CI** | B V Tejas (PES1UG24CS108) | **SIGNED OFF** | 06-Nov-2026 |
| **Persistence & Security Lead** | Bharath Hegde (PES1UG24CS114) | **SIGNED OFF** | 06-Nov-2026 |
| **UI & Diagnostics Lead** | C Dinesh Kumar (PES1UG24CS117) | **SIGNED OFF** | 06-Nov-2026 |
| **Course Product Owner** | Course Evaluator / Instructor | **SUBMITTED FOR REVIEW** | 06-Nov-2026 |

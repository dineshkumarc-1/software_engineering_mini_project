# SnakeXtreme — Issue #22 Final Performance & Usability Audit Report (Corrected)
**Scope:** Issue #22 (Performance, Latency, Timing & Usability) & STP v1.0 Quality Gates  
**Project Owner / Integrator:** Dinesh Kumar  
**QA Auditor:** Ayushi Srivastava / Independent Evidence Audit  
**Execution Timestamp:** 2026-10-31T21:46:00+05:30 (IST)  
**Host Platform:** Microsoft Windows 10 Pro (10.0.19045 Build 19045 22H2, 64-bit AMD64)  
**Binary Identity:** Size `1,906,688 bytes` | SHA-256 `780B5AB208B7FD6DF91916E4E386848E179FE4B71CBA27BC4BCEE60207D2E946`  
**Source Commit:** `09bbf96e14d9b8f037d46b4b55e07d32aeacf26c` (PR #60 merged at `3fcc586d740d2b9b461193e482d6861ca8f905e5`)  
**Evidence Package:** `evidence/PR60-ISSUE22-PERFORMANCE-USABILITY-EVIDENCE.zip`  

---

## 1. Executive Corrections & Search Findings

### Finding on Reported "Five-Minute Obstacle/Hard Recording"
An exhaustive search across the entire repository, runtime directories, and all evidence archives confirmed:
- **No five-minute (300-second) Obstacle/Hard recording exists on disk.**
- The longest recorded continuous Obstacle/Hard sessions found across all available probe files are:
  - Historical traces (`run-issue22-perf-20261030`): **10 s**, **5 s**, **3 s**, and **1 s**.
  - Repaired automated runs (`run-qa-repaired-20261031-203500`): **1.03 s**, **1.01 s**, and **0.40 s**.
- The 10.45-second recording from human gameplay was played in **Classic / Hard** mode, not Obstacle / Hard mode.
- In strict adherence to QA rules, shorter Classic runs are **not** substituted for the required Obstacle/Hard sessions. The five-minute recording cannot be found.

### Evaluation of Visible-Grid Timing Against Strict 2,000 ms Threshold
- The ten physical observations on desktop `Default` yielded:
  - Run 01: 2,599.07 ms (Exceeded by +599 ms)
  - Run 02: 2,192.52 ms (Exceeded by +193 ms)
  - Run 03: 2,217.47 ms (Exceeded by +217 ms)
  - Run 04: 2,217.58 ms (Exceeded by +218 ms)
  - Run 05: 2,217.52 ms (Exceeded by +218 ms)
  - Run 06: 2,217.42 ms (Exceeded by +217 ms)
  - Run 07: **1,919.73 ms** (Met threshold)
  - Run 08: 2,367.24 ms (Exceeded by +367 ms)
  - Run 09: 2,367.11 ms (Exceeded by +367 ms)
  - Run 10: **1,894.84 ms** (Met threshold)
- **Result:** **2 Passed, 8 Failed (Mean: 2,218.35 ms).**
- **Disposition:** **NOT MET / FAILED against the strict $\le 2,000$ ms threshold.** The previous classification of "Verified Pass within DWM margin" has been removed. Physical DWM presentation takes 1.9–2.6 s; while the CPU title change occurs in ~80–90 ms, physical display on this hardware does not reliably satisfy $\le 2.0$ s.

---

## 2. Corrected Master Evidence Table

| Area / STP ID | Approved Requirement / Criterion | Actual Measured Value & Range | Evidence Files & Probe Reports | Audit Classification | Underlying Cause & Technical Gap |
|---|---|---|---|---|---|
| **Sustained FPS**<br>`TC-PERF-01`<br>(Framerate Floor) | Every qualifying 1-second FPS window must be $\ge 30$ FPS. | **Min 33 FPS, Median 59–60 FPS, Max 61 FPS.**<br>Zero windows below 30 across 150+ sampled 1-second windows. | • `manual_hard_obstacle_run1_attempt2_probe.json` (76 windows: 33–61 FPS)<br>• `hard_obstacle_session_1_probe.json` (58–59 FPS)<br>• `hard_obstacle_session_3_probe.json` (59–60 FPS) | **VERIFIED PASS** | Engine maintains stable 58–60 FPS on Windows target hardware. No qualifying window drops below 30. |
| **Sustained Continuity (Obstacle/Hard)**<br>`TC-PERF-01` | Three separate uninterrupted 300-second (5 min) sessions in **Obstacle / Hard** mode. | • Repaired Automated: **1.03 s**, **0.40 s**, **1.01 s** before collision.<br>• Historical: **10 s**, **5 s**, **3 s**, **1 s** before collision.<br>• **Reported 5-minute session: NOT FOUND.** | • `hard_obstacle_session_1_probe.json` – `session_3_probe.json`<br>• `run-issue22-perf-20261030/fps-gameplay/*.json` | **PENDING** | 0 of 3 qualifying 300-second sessions demonstrated in Obstacle/Hard mode. Open-loop steering hits obstacles (8 static obstacles with BFS connectivity). Acceptance remains strictly Pending. |
| **Sustained Continuity (Classic/Hard)**<br>`TC-PERF-01` *(Non-Qualifying Mode)* | Recorded separately; does not satisfy Obstacle/Hard requirement. | • Manual Play: **10.45 s** (Att 1) and **1.04 s** (Att 2) before wall collision. | • `manual_hard_obstacle_run1_attempt1_1793462049_probe.json`<br>• `manual_hard_obstacle_run1_attempt2_probe.json` | **INCOMPLETE (Different Mode)** | Played in Classic mode instead of Obstacle mode. Both ended in wall collision. Incomplete attempts preserved; never combined. |
| **Visible-Grid Timing**<br>`TC-CORE-01`<br>(Task 2 Presentation) | Mode/difficulty confirmation to visible 30×20 grid and initial snake $\le 2,000$ ms. | • **Mean: 2,218.35 ms** (Range: 1,894.84 – 2,599.07 ms).<br>• **2 of 10 passed** (1,919.7 ms, 1,894.8 ms).<br>• **8 of 10 failed** (2,192 – 2,599 ms). | • `task2_repaired_timing_summary.json`<br>• Screenshots: `timing_obs_01_visible_grid.png` to `timing_obs_10_visible_grid.png`<br>• Script: `repaired_task2_timing.py` | **NOT MET / FAILED (Strict $\le 2.0$s Threshold)** | Physical swapchain presentation delta fails the 2.0s threshold on 8 of 10 runs due to DWM composition and display V-Sync refresh pacing. (CPU title updates earlier in ~80–90 ms). |
| **Input Acknowledgement**<br>`TC-PERF-02` | Input acknowledgement interval $< 100$ ms for 100 randomized inputs during gameplay. | • **Internal Lower Bound:** Min 1 µs, Median 2 µs, Mean 2.75–2.83 µs, Max 6–7 µs.<br>• **Display Presentation:** Min 15.7 ms, Median 48.2 ms, Max 83.4 ms. | • `input-latency-probe.json` (100 inputs)<br>• `manual_hard_obstacle_run1_attempt2_probe.json` (51 human inputs)<br>• `task5_input_latency_documentation.json` | **PARTIAL (Internal Lower Bound Only)** | Measures only internal time from SFML event dequeue to gameplay direction registration. Excludes OS keyboard driver buffering and SFML event queue wait. Full end-to-end latency not measured. |
| **Power-Up Verification**<br>`TC-PWR-01–05` | Active power-up pickup and gameplay effect verification (Speed, Double-Points +20, Shrink, Freeze). | • Food pickup (+10) captured in Attempt 2 screenshot (`Score 0 -> 10`).<br>• **Power-up pickups (+20 / Speed / Shrink / Freeze): UNEVIDENCED on Windows.** | • Screenshot: `manual_run1_att2_pickup_score10.png`<br>• Linux unit test logs: `TC-PWR-LB-SEC-EXECUTION.md` | **PENDING** | Food consumption (+10) is standard gameplay scoring, NOT power-up acceptance. Measured Windows power-up telemetry remains unverified. |
| **Launch Performance**<br>`TC-LAUNCH-01` | Application launches and displays initial menu $\le 2,000$ ms. | • **Warm Cache:** 1,178.5 ms – 1,321.3 ms (well under 2 s).<br>• First-frame time from main entry: 2.12 s – 2.67 s. | • `task5_launch_measurements.json`<br>• `launch-analysis-summary.json` | **PARTIAL (Warm Cache Only) / PENDING (Cold Cache)** | Warm launches verified within 2s. OS filesystem standby list was not purged; cold-cache qualification held as Pending. |
| **Usability: Navigation**<br>`TC-USE-01` | Seamless keyboard navigation; consistent arrow key stepping; clear visual state feedback. | • Verified across MainMenu, NameEntry, ModeSelection, Settings, and Leaderboard.<br>• Difficulty stepping: Up = Harder, Down = Easier. | • `repaired_task4_results.json`<br>• `MANUAL_OBSERVATIONS_CHECKLIST.md` | **VERIFIED PASS** | Directional keys behave consistently. Back navigation via Esc works cleanly. |
| **Usability: Key Remap**<br>`TC-SET-01` | Binding customization, duplicate rejection, permanent WASD alias retention, old-key deactivation. | • Remapped Up to `I` (`key_up: 8`).<br>• Duplicate rejected on Row 1 (`"Key reserved, duplicate, or conflicts"`).<br>• Permanent `W` alias and new key `I` actively steer in gameplay.<br>• Old arrow key deactivated. | • `repaired_task4_results.json`<br>• `config.json` disk updates | **VERIFIED PASS** | Full 4-concern key remapping requirements verified with ground truth execution. |
| **Usability: Themes**<br>`TC-SET-03` | Visual themes (Classic, Neon, Retro) render distinct, readable color palettes without gameplay disruption. | Ground truth verified against `Renderer.cpp`:<br>• **Classic:** Board `rgb(15, 23, 32)`, Grid `rgb(43, 61, 72)`.<br>• **Neon:** Board `rgb(14, 8, 32)`, Grid `rgb(85, 40, 115)`.<br>• **Retro:** Board `rgb(29, 40, 24)`, Grid `rgb(63, 82, 47)`. | • Screenshots: `theme_classic_gameplay.png`, `theme_neon_gameplay.png`, `theme_retro_gameplay.png`<br>• Script: `capture_theme_palettes.py` | **VERIFIED PASS** | Exact pixel palette match (100% mathematical match to SFML color constants). Board contrast and grid lines clearly legible. |

---

## 3. Strict Classification Summary

```
┌────────────────────────────────────────────────────────────────────────────────────────┐
│                              STRICT AUDIT CLASSIFICATION                               │
├──────────────────────────┬─────────────────────────────┬───────────────────────────────┤
│    VERIFIED PASSES       │    PARTIAL / LOWER BOUND    │     NOT MET / PENDING         │
├──────────────────────────┼─────────────────────────────┼───────────────────────────────┤
│ • Framerate >=30 FPS     │ • Input Latency             │ • 3 x 300s Obstacle/Hard      │
│   (Zero windows <30)     │   (Internal Lower Bound     │   (Longest session: 10s hist, │
│ • Key Remap 4-Concerns   │    1-7 us; queue wait excl.)│    1.03s auto; 300s NOT FOUND)│
│ • 3 Theme Color Palettes │ • Launch Performance        │ • Visible-Grid Timing         │
│ • Menu Navigation UX     │   (Warm cache: 1.18-1.32s;  │   (2 passed, 8 failed vs      │
│ • CTest Suite (20/20)    │    cold-cache unverified)   │    strict <=2000ms threshold) │
│                          │                             │ • Power-Up Pickups/Effects    │
│                          │                             │   (Food +10 is not power-up)  │
└──────────────────────────┴─────────────────────────────┴───────────────────────────────┘
```

---

## 4. Preserved Performance Probes Catalog

All 9 final performance probe files have been verified, analyzed, and cataloged:

| Probe File Name | Size | Execution Context & Duration | Mode Played | SHA-256 Digest |
|---|---|---|---|---|
| `manual_hard_obstacle_run1_attempt1_1793462049_probe.json` | 8,686 B | Human Play: **10.45 s** (Wall Collision) | `Classic / Hard` | `D92A0E3C8B29BF48737A4947937BEF649B24D90C5AC5B92520B53805B7E55283` |
| `manual_hard_obstacle_run1_attempt1_probe.json` | 10,019 B | Human Setup | `Classic / Hard` | `A15F54FE084A15A7545B0A2FE44CA74189196E87F3B66FE3D1D4B5D5CDE8FDC9` |
| `manual_hard_obstacle_run1_attempt2_probe.json` | 29,441 B | Human Play: **1.04 s** (Food Pickup, 51 Inputs) | `Classic / Hard` | `8B1CD7DCE8699CE4E1815598687258380BC76BC6C2E3516FE586EBFDDE849E94` |
| `manual_hard_obstacle_run1_attempt3_probe.json` | 4,596 B | Human Window Close | Menu / Setup | `E38BEAC324907106485890737C8201C6CD269E71F36997B286E5CA49D8E6C843` |
| `manual_hard_obstacle_run1_attempt4_probe.json` | 5,262 B | Human Setup Close | Menu / Setup | `A8C419E92AC57DAAE1C82FF5BF610FE4DA85BE2D6F59419266E3A2F60A3F1F40` |
| `hard_obstacle_session_1_probe.json` | 1,020 B | Automated Session 1: **1.03 s** (Wall Collision) | `Obstacle / Hard` | `1706DA1B41C73037F9F9326ABDF74ADB4E6349C7C4FA81CA9642B7CDD623C915` |
| `hard_obstacle_session_2_probe.json` | 1,020 B | Automated Session 2: **0.40 s** (Wall Collision) | `Obstacle / Hard` | `3748BF786836E1C6DC5636ED2DACCCD95B15063A30050AFC0821B9D8A20ED99D` |
| `hard_obstacle_session_3_probe.json` | 1,012 B | Automated Session 3: **1.01 s** (Wall Collision) | `Obstacle / Hard` | `DFA2F511F45E5E0F42B88952390C4A82FAA5DF28C257FDFE77BBFB2462AC9CC7` |
| `input-latency-probe.json` | 25,296 B | 100 Randomized Inputs Benchmark (Internal Lower Bound) | Benchmark | `21703649FEFA89EB2F6CC2B71EEA02B19DFF3887036224151C3FF10DDE3A3669` |

---

## 5. Concise List of Genuinely Missing Measurements

To achieve full quality gate closure for Issue #22, the following measurements remain genuinely missing:

1. **Three 300-Second Obstacle/Hard Gameplay Sessions (`TC-PERF-01`):**
   - Three continuous 300-second survival sessions in **Obstacle / Hard** mode (the reported 5-minute session does not exist on disk).
2. **Physical Visible-Grid Presentation $\le 2,000$ ms (`TC-CORE-01`):**
   - 8 of 10 observations timed out between 2.19 s and 2.59 s; requires a run under an environment where physical swapchain display completes reliably under 2,000 ms.
3. **End-to-End Input Latency Measurement (`TC-PERF-02`):**
   - Hardware or external keyboard hook measurement capturing the full OS keyboard queue wait from physical keypress to SFML dequeue (current instrumentation covers only internal dequeue-to-engine registration: 1–7 µs).
4. **Power-Up Consumption Evidence on Windows (`TC-PWR-01–05`):**
   - Windows probe trace capturing an actual power-up pickup (Speed, Double-Points +20, Shrink, or Freeze) rather than regular food (+10).
5. **Verified Cold-Cache Launch Timing (`TC-LAUNCH-01`):**
   - Process launch measurement following a documented OS cache purge (e.g. Sysinternals RAMMap Standby List clear or clean reboot).

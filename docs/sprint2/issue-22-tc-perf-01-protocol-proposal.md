# TC-PERF-01: proposed alternative protocol (approval required)

Author: B V TEJAS, for #22. **Status: proposal only.** It is **not** an approved replacement, and nothing here changes a test result. Until the owner (Dinesh) approves an option in writing on #22, TC-PERF-01 stays **Pending acceptance measurement**. All existing partial runs stay **Incomplete**, and none of them is marked Passed.

## 1. What is required today

- **NFR-001 (SRS 6.1):** "The game loop shall maintain a minimum of 30 frames per second on the target hardware profile." The SRS verification row reads: "Game loop sustains ≥30 FPS over a 5-minute play session".
- **TC-PERF-01 (STP):**
  - Preconditions: reference hardware profile (Windows 10/11 x64, dual-core ≥ 2.0 GHz, 4 GB RAM, integrated GPU with OpenGL 3.3); **Hard + Obstacle; power-ups active; 3 runs × 5 min**.
  - Step: run the FPS probe for 5 minutes and log every 1-s window.
  - Expected: no 1-s window below 30 FPS in any run; min/avg/median reported.
  - Mode: **"Auto · DK"**.
- **Current interpretation** (STP matrix, #22): three *continuous* 300 s Hard/Obstacle gameplay sessions with power-ups, measured by the probe's `hard_obstacle_uniform_gameplay_fps` and `longest_same_session_eligible_seconds`.

## 2. Why the workload as interpreted is impractical

- **A person cannot reliably survive 300 s on Hard/Obstacle.** At Hard the snake moves every 70 ms, which is about 4,300 moves in 5 minutes. The board is 30×20 with 8 obstacles, and the snake keeps growing.
  - The reviewed Windows traces have continuous Hard/Obstacle segments of only **3 s, 1 s and 5 s**, and two later probes had **zero** complete qualifying windows (see `WINDOWS10-EVIDENCE-REGISTER.md`).
  - Dinesh reported one five-minute Obstacle/Hard session, but its raw probe has not been uploaded and analysed.
- **The STP marks the test "Auto", but no automation exists.** Nothing in the project can play 300 s of Hard/Obstacle without a person.
- **"Power-ups active" cannot be checked from the probe today.** The probe report hard-codes `"powerups_implemented": false`, and its FPS windows do not record whether a power-up was on the board or an effect was running. That data no longer matches the game, which has power-ups.

## 3. What the 300-second runs protect against

Any replacement has to cover these three things, or state openly that it doesn't.

| # | Purpose | Why it matters |
| --- | --- | --- |
| R1 | **Enough samples:** 3 × 300 = 900 one-second windows at the target configuration | One short segment can miss an occasional slow frame |
| R2 | **Sustained effects:** a long run without a restart | Thermal throttling, memory or resource growth, a longer snake (more to draw), timer drift |
| R3 | **Worst-case workload:** Hard tick rate, obstacles and power-ups (the speed boost shortens the tick to 42 ms) | The test is meant to show the worst case on the reference PC |

## 4. Options

### Option A: pooled sessions plus one survivable continuous run (no product change)

| Part | Workload | Pass criterion | Covers |
| --- | --- | --- | --- |
| A1, pooled | Hard + Obstacle, power-ups collected whenever possible, as many sessions as needed (restarts allowed). Same EXE, reference PC, probe on | **≥ 900** complete, uniform Gameplay/Obstacle/Hard windows in total, from at least 3 sessions; **every** window ≥ 30 FPS; 0 dropped or invalid samples; min/avg/median and each session's length reported | R1 and R3 (sample size at the real worst-case tick rate) |
| A2, continuous | **Obstacle + Easy** (200 ms tick, which a person can survive), power-ups collected, **3 × 300 s without a restart** | Every complete window ≥ 30 FPS for each 300 s run | R2 (sustained run, and a *longer* snake, so more drawing per frame) |
| A3, link check | Compare A1's Hard/Obstacle FPS distribution with A2's Easy/Obstacle distribution on the same PC | Medians agree closely (the threshold, e.g. ±5 %, must be agreed before running) | Tests the assumption that frame cost depends on what is drawn, not on the tick rate. **If A3 fails, A is not valid** |

- **What A measures:** per-second frame rate under the real Hard/Obstacle workload with enough samples, plus a sustained 5-minute run with the same drawing workload.
- **What A does not measure:**
  - an *uninterrupted* 300 s at the Hard tick rate;
  - a sustained Hard-rate speed boost;
  - anything a 5-minute Hard run would reveal that a 5-minute Easy run would not.
- **Report it as:** "NFR-001 verified under the approved alternative protocol (A), with these limits." **Never** report it as "TC-PERF-01 Passed as written".
- **Small additions needed before A is run** (measurement-only, new SHA, reviewed separately):
  - the probe records power-up activity per FPS window (effect active / item on the board), and the hard-coded `powerups_implemented` field is fixed;
  - `analyze_performance.py` adds pooled counts per mode/difficulty and the longest continuous run per context.

  Without these, power-up activity can only be shown by screen recording.

### Option B: closed-loop steering driver, the literal workload (recommended)

This is the same method Dinesh proposed in #72's closure report. It is the recommended **replacement for how the test is run**. TC-PERF-01's workload and thresholds stay unchanged.

**What it is.** An opt-in driver (for example `--perf-autopilot`, accepted only together with `--perf-probe`) steers the snake in an ordinary Hard/Obstacle game. It reads the live game state in the same process and sends directions through **the same input path as the keyboard**. Normal play (no options) is unchanged.

**Requirements that make it verifiable:**

1. **Normal rules only.** All wall, body and obstacle collisions stay on. No pause, restart, invulnerability, speed change or power-up suppression. The driver may only choose among directions the game would accept from a keyboard.
2. **Deterministic.** The policy and its seed are fixed and recorded. The policy is: a shortest safe path to food or a power-up that keeps the tail reachable; otherwise, follow the tail.
3. **Every request is logged.** The probe log records each direction the driver sends (source `autopilot`) and whether it was accepted or rejected, just like a key press.
4. **Power-ups are visible in the data.** The probe records, for each FPS window, whether a power-up was on the board and which effect was active. The hard-coded `powerups_implemented: false` is fixed.
5. **No stitching.** Each run is one 300 s gameplay interval, counted from the first displayed Gameplay frame. A run that dies, restarts, pauses or changes context is **Incomplete**, and a new full run starts. Attempts are never joined.
6. **Unchanged pass rule.** 3 runs; no complete 1-s window below 30 FPS; zero-frame stall windows count; min/avg/median reported; same EXE SHA and reference PC for all 3 runs.

**What B verifies:**

- R1: 900 one-second windows of real Hard/Obstacle gameplay.
- R2: three sustained 5-minute runs, with the snake growing as it does in play.
- R3: the Hard tick rate, obstacles and power-up effects, including the 42 ms speed-boost tick.

**What B does not verify:**

- How a *human* plays: the driver's movement pattern and snake length are not a player's.
- The cost of drawing a very long snake, if the driver keeps the snake shorter than a skilled player would. Report the maximum snake length reached in each run.

**Cost:**

- About 1–2 days of code and tests, a new SHA and a new Windows build.
- The driver can still trap itself; such runs are reported as Incomplete.
- The driver code must be reviewed so it cannot reach normal play.

### Option C: "no game over" diagnostic (not recommended)

- **What it is:** an opt-in switch that ignores collisions during the probe.
- **Why not:** it keeps the frame load but changes the game's rules, so the measured run is not gameplay. It also has the same code cost as B.

## 5. Recommendation and decision needed

1. **Recommended replacement: Option B** (the closed-loop driver, as also proposed in #72). It keeps the STP's workload, the 300 s continuity and the 30 FPS rule, and it matches the STP's "Auto" mode. Only the source of the steering changes.
2. **Fallback, if there is no time for the driver: Option A,** as an explicitly approved deviation with its limits recorded. That needs:
   - the probe/analyzer additions first;
   - the A3 link check to pass;
   - a deviation entry, recorded by its owner.

   Report A only as "NFR-001 verified under the approved alternative protocol (A)", never as "TC-PERF-01 Passed".
3. In both cases, keep these unchanged: the reference hardware, the 30 FPS limit on **every** window, min/avg/median reporting, and one EXE SHA per campaign. TC-PERF-02 is not affected.

**Decision requested** from the project owner (Dinesh) **and** the QA owner responsible for the STP baseline, on #22, by replying with one of:

- *"Keep the original protocol"*;
- *"Approve B as the execution method"*;
- *"Approve A as the replacement, with its stated limits"*;
- *"Approve A now, B later"*.

Record the approver names, the date and the approved wording. Until then nothing here is a replacement, the driver is not built or run for acceptance, and no partial run is relabelled.

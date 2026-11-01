# Issue #22: valid timing methods for TC-CORE-01 (visible grid) and TC-PERF-02 (queue-inclusive input)

Author: B V TEJAS. **Status: proposed methods and review findings. Nothing here is a result, an acceptance, or an approved protocol.** TC-CORE-01 / FR-001, TC-PERF-02 / NFR-002 and TC-PERF-01 / NFR-001 stay **Pending**.

Scope: this document fixes *how* the two measurements must be taken so that the number means what the requirement says. It does not change product code. It reuses `--perf-probe` from #70 and the stdout marker `SNAKEXTREME_PROBE_FIRST_FRAME`.

## 1. What each requirement asks for

| Case | Requirement | Start point | End point | Source |
| --- | --- | --- | --- | --- |
| TC-CORE-01 | FR-001: confirm to grid within 2000 ms, 10 runs | the user's confirming key | the 30x20 grid and the 3-segment snake **visible** | STP / #72 closure report |
| TC-PERF-02 | NFR-002: input acknowledgement within 100 ms, 100 random-time key presses | the key event **entering the OS** | the engine **registering** the request (accepted or rejected) | `docs/qa/STP_EXECUTION_MATRIX.md` row TC-PERF-02: "verify timestamp origin and queue-delay scope. Visible turn is informational, not acknowledgement threshold" |

## 2. Why the in-app numbers cannot be the acceptance numbers

Both in-app intervals start when SFML hands the event to the game (`pollEvent` returns). Anything that waits **before** that is invisible to them.

- `new_games` (#70) runs from the confirming event's dequeue to the return of `display()` for the first Gameplay frame. It excludes the OS queue wait and the time until the frame is actually shown.
- `inputs[].dequeued_us -> registered_us` is the TC-PERF-02 "internal lower bound" (1-7 us on Windows). It excludes the OS queue wait.

**The queue wait is not hypothetical after #69.** With the fix, `AudioService::start()` runs once after the first frame and blocks the event loop while the OpenAL device opens (about 1.8-1.9 s on Dinesh's first launch after a reboot, about 0.5 s warm). A key pressed during that time sits in the OS queue.

Local demonstration (Linux, Xvfb, null audio, **not a Windows measurement**): a throw-away copy of #69's head with a 2000 ms sleep added to the audio start, keys sent about 0.1 s after the first-frame marker.

| Case | In-app `new_games` (dequeue to display return) | External: confirming key sent to grid pixels seen |
| --- | --- | --- |
| Confirm while audio is opening (3 runs) | 2.2, 3.2, 3.9 ms | 1,896, 1,902, 1,896 ms |
| Confirm after audio is open (3 runs) | 16.7, 16.7, 16.7 ms | 19, 27, 43 ms |

All five queued keys were processed, in order, after the block; the game reached `Playing | Classic / Easy` and no key was lost. The in-app number is wrong by the whole queue wait, so it can never be used on its own for FR-001 on a first launch.

**Rule 1 for TC-CORE-01.** Report two classes separately and never mix them:

- **Steady runs:** the confirm is sent after `audio_started` has been reached (check the probe report, or wait at least 5 s after the first frame on the first launch). These are the ten FR-001 runs.
- **Early-confirm runs:** the confirm is sent within 1 s of the first frame on the first launch after a reboot. Report these as a separate observation. If the owner reads FR-001 as "from the user's key press regardless of start-up", these runs decide whether the menu-before-audio trade-off is acceptable (#22, section 1).

## 3. Method V: confirm to visible grid (TC-CORE-01)

### 3.1 Procedure (external harness, one run)

1. Unlocked interactive desktop, game window in the foreground and unobscured, charger connected, one build per campaign (record commit and EXE SHA-256).
2. Drive the game to Mode Selection and set the mode and difficulty with the keyboard. Wait until the screen is stable.
3. Start a capture loop that grabs **only the game's client rectangle** as fast as the capture API allows, and stores `(perf_counter_ns, detector_value)` for every capture.
4. Immediately before the confirming key is sent, take `t0 = perf_counter_ns()`. Send the key with `SendInput`.
5. `t1` = the timestamp of the **first** capture, after `t0`, whose detector says "gameplay grid visible" (section 3.2). The run's value is `t1 - t0`.
6. Save that first matching capture, and the last capture before it, as PNGs taken by the same capture path, and record the capture count and the longest gap between captures.
7. Cross-check: read the run's `new_games` entry from the probe report. External minus internal is the queue wait plus presentation plus capture delay. A run is suspicious if that difference is below 0 or is not explainable (section 3.3).

### 3.2 The detector must be specific to the gameplay screen

"Pixel divergence from the menu baseline" is **not** a valid detector: any change (a notification, the cursor, a window move, a title bar repaint, a menu animation) triggers it. Use a detector that is true only for gameplay:

- count pixels of the **snake body colour** of the active theme inside the client rectangle (the board's grid is drawn in a second known colour); gameplay is "visible" when the count is at least one cell's worth, for example 1,000 pixels at 900x600 for two body cells;
- take the colour from a reference gameplay screenshot of the same theme, captured by the same capture path; the menu and mode screens contain zero pixels of the body colour (checked on Classic: 0 of 540,000 on both screens, 1,568 on the gameplay screen);
- the first-match capture saved in step 6 must show the grid, the 3 segments and the food, which is the evidence image the closure report asks for.

### 3.3 Validity rules (a run that breaks one is **invalid**, not slow and not failed)

| # | Rule | Why |
| --- | --- | --- |
| V1 | The session is unlocked and the game window is the foreground window; log both. | While the workstation is locked, DWM does not present the game's OpenGL surface, so nothing changes on screen. |
| V2 | A timeout is **not a latency**. If the detector never fires within the timeout, the run is "not observed" and is reported as such. The timeout must be much longer than 2 s (use 10 s) so that 2 s can be distinguished from "never". | Values that cluster just above the timeout are a timeout plus overhead, not a measurement. |
| V3 | The capture path must read the composed OpenGL surface. GDI `BitBlt` from the window DC does not. Use `PIL.ImageGrab`, the Desktop Duplication API, or Windows.Graphics.Capture, and verify with the saved PNG that it shows the game. | The existing `tc_core_01_grid_snake_frame.png` shows a chat window instead of the game. |
| V4 | Run a **positive control** before the campaign: a trivial screen change that is known to be fast (for example Main Menu to Name Entry, whose detector is a pixel region of that screen). The control result is the harness floor. | It shows what the harness itself costs on this PC. |
| V5 | Run a **negative control**: with no key sent, the detector must stay false for 5 s. | It rules out a detector that fires on noise. |
| V6 | Record the capture interval. Resolution is the larger of the capture interval and one display refresh (16.7 ms at 60 Hz). Report it with the result. | The number is only as fine as the sampling. |
| V7 | Values repeated to the tenth of a millisecond across runs (for example four runs at 2,217.4-2,217.6 ms) are a red flag for a clamped or timer-driven value. Investigate before reporting. | Real latencies vary by at least a frame. |
| V8 | Each run keeps its raw capture log (`t, detector_value`) and the first/last-capture PNGs. | Anyone can re-derive the result. |

### 3.4 What this method does and does not measure

- It measures the key entering the OS to the grid being present in the captured, composed image. It includes the OS and SFML queue wait, the game's work, and the compositor's presentation.
- It excludes monitor scan-out and photon delay. State this; it is the same limit the closure report already states.
- Linux prototype (Xvfb, no compositor): 19-43 ms for a steady confirm, 1,896-1,902 ms for a confirm during a 2 s audio block. The Windows values are not known until Dinesh runs them.

### 3.5 Applying this to the existing evidence

The ten TC-CORE-01 observations in #72 (`tc-core01-locked-poll-measurements.json`) all have `visual_pixel_change_detected: false` and `pixel_diff_count: 0`, were taken while the workstation was locked, and break V1, V2 and V7. #72's closure report already treats them as not valid. Two further points:

- `supporting/tc-perf01-historical-report.md` still lists TC-CORE-01 as "NOT MET / FAILED (2 passed, 8 failed)" and attributes the delay to DWM composition and V-Sync. That cause is not shown by any data, and the closure report says the runs timed out. The row, and the "Visible-Grid Timing" line in its summary box, should be marked superseded.
- The same file lists TC-PERF-01 "Sustained FPS" as **VERIFIED PASS** from Classic/Hard and 1-second Obstacle/Hard fragments. The report itself says Classic runs are not substituted, and the closure report keeps TC-PERF-01 Pending. That row should say "informational only".

## 4. Method Q: queue-inclusive input acknowledgement (TC-PERF-02)

### 4.1 Procedure (one campaign, 100 key presses)

1. Unlocked desktop, game window focused, a game that stays alive for the whole campaign (Classic/Easy, or the closed-loop driver is **not** used: it bypasses the OS queue and must not be mixed into TC-PERF-02).
2. Start the game with `--perf-probe`. Read `SNAKEXTREME_PROBE_FIRST_FRAME` from stdout and take `T_marker = perf_counter_ns()` when it arrives.
3. Time alignment: `offset_us = T_marker/1000 - first_frame_us` (from the probe report). It converts the app clock (microseconds since `main`) to the harness clock. It is **late by the marker's pipe/IPC delay**, so every latency below is slightly **under**-stated by that delay; measure the delay by sending an idle key (section 4.3, Q3) and report it.
4. Send 100 direction keys at random times: draw the gap from 30-150 ms with a fixed seed (record the seed), so presses land at random phases of the 16.7 ms frame and of the tick. Immediately before each `SendInput`, take `t_inject_i`. Optionally also record the delivery time from a low-level keyboard hook (`WH_KEYBOARD_LL`) as `t_hook_i`.
5. After the game exits, join the harness log with the probe report **by order** (the probe records every request in order, accepted and rejected, `dropped_inputs` must be 0). The i-th direction in the report must equal the i-th key sent.
6. Per press: `ack_i = registered_us_i + offset_us - t_inject_i/1000`. Report min, median, p95, **maximum**, the count over 100 ms, and accepted/rejected counts.
7. Informational only: `displayed_us_i + offset_us - t_inject_i/1000`, the key to the next displayed frame.

### 4.2 Pass rule (the STP's)

No acknowledgement over 100 ms across the 100 presses, with all 100 accounted for as requested, accepted, rejected and registered. Rejected presses stay in the data. The maximum, not the median, is the headline.

### 4.3 Validity rules

| # | Rule | Why |
| --- | --- | --- |
| Q1 | The probe report must contain 100 inputs in the order sent, with `dropped_inputs = 0`. | Pairing is by order. |
| Q2 | Every `ack_i` must be at least 0 minus the IPC delay. A clearly negative value means the alignment is wrong, and the campaign is invalid. | It checks the clock alignment from the data itself. |
| Q3 | **Idle control:** before the campaign, with the game idle in Gameplay, send 20 keys that are spaced by 500 ms. The medians show the best case (about one frame on Linux). The minimum shows the alignment error. | It separates harness cost from game cost. |
| Q4 | **Same clock check:** on Windows, MSVC's `steady_clock` and Python's `perf_counter` are both backed by `QueryPerformanceCounter`, so `T_marker` and the app clock are comparable. Confirm it once by logging a hook timestamp and checking `t_hook - t_inject` is under 2 ms. | The method assumes it. |
| Q5 | The campaign runs on the build under test, with the audio device already open (after `audio_started`). A key pressed during the audio open is the early-confirm case of section 2 and is reported separately. | Otherwise one 1.9 s outlier is the audio start, not input handling. |
| Q6 | Record the harness's own spawn/SendInput overhead. On Linux with `xdotool` it adds a few milliseconds; on Windows, in-process `SendInput` adds microseconds. | It bounds the over-statement. |

Linux prototype (Xvfb, software GL, 60 presses, marker-aligned): queue-inclusive acknowledgement **min 4.6 ms, median 13.0 ms, max 21.4 ms**, against an in-app dequeue-to-registration of **0-2 us**. The difference is the OS/SFML queue wait, which is what the closure report says is missing. These are Linux numbers; they say the method works, not what Windows will show.

### 4.4 Existing TC-PERF-02 data: two sources disagree

- `tc-perf02-dequeue-only-summary.json` reports `display_latency_us` min 151,800, median 344,300, max 401,800 (that is, 152-402 ms).
- `tc-perf01-historical-report.md` reports "Display Presentation" min 15.7 ms, median 48.2 ms, max 83.4 ms for the same probe file.

They cannot both describe `displayed_us - dequeued_us` for the same 100 inputs. The raw `input-latency-probe.json` is not in #72, so I could not check either. Please upload the raw file, or drop both display figures (they are informational and not the acceptance endpoint).

## 5. Early-confirm and "not responding" risk (for the #69 decision)

`AudioService::start()` blocks the message pump. Windows marks a window "Not responding" after about 5 s without pumping. The measured first-launch block is 1.9 s, so there is about 2.6x margin, but that margin depends on the device. On the Windows retest, report `audio_started - first_frame` for every run. If any run is above 3 s, escalate it: it is the evidence for moving the device open to a background thread.

## 6. What still needs Windows

- Run the harness checks V1-V8 and Q1-Q6 on the unlocked Windows PC with the #69 candidate and record its EXE hash.
- The 10 steady confirm runs, 3 early-confirm runs, 100 acknowledgement presses, and the idle and positive/negative controls.
- Nothing above has been run on Windows by me. Dinesh's existing `measure_visible_grid_campaign.py` can be corrected against V1-V8 rather than replaced.

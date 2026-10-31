# TC-CORE-01 Evidence Correction & Defect Disclosure

**Date:** 2026-10-31T11:50:00+05:30  
**Test Identifier:** TC-CORE-01 (Requirement: FR-001)  
**Correction Classification:** Evidence Defect, Interpretation Addendum & Presentation Measurement Addendum  
**Preservation Rule:** Original measurement JSON (`confirmation-to-grid-results.json`) and original capture artifact (`tc_core_01_grid_snake_frame.png`) are strictly preserved without alteration.

---

## 1. Invalidation of Old Pass Flags

- **Original Record:** In `confirmation-to-grid-results.json`, fields `"all_passed": true` and `"passed": true` were set based on comparing mode confirmation to window title update time against the 2,000 ms threshold.
- **Explicit Correction:** **The old grid-timing pass flags (`"all_passed": true`, `"passed": true`) are INVALID for first-render acceptance.**
- **Reason:** In `AppManager.cpp` (lines 74–79), `refreshTitle()` runs *before* `window.clear()`, `renderer.draw()`, and `window.display()`. Measuring the title change via Win32 `GetWindowTextW` captures CPU-side string dispatch and window message delivery (4.07–15.89 ms). It does not prove that the 30×20 grid, snake segments, or food were rasterized or displayed.

---

## 2. Screenshot Artifact Defect (`tc_core_01_grid_snake_frame.png`)

- **Observed Defect:** The image bundled as `tc_core_01_grid_snake_frame.png` displays the chat / IDE application interface rather than the active SnakeXtreme game grid.
- **Root Cause Analysis:** The test harness invoked standard Win32 GDI `BitBlt` against `user32.GetDC(hwnd)`. Under the Windows Desktop Window Manager (DWM) composition pipeline, modern hardware-accelerated OpenGL / SFML windows render to an independent DirectX/OpenGL swapchain. GDI `BitBlt` against the window DC reads the underlying desktop composition background (in this case, the chat / IDE window positioned behind the game window) rather than the OpenGL surface.
- **Correction:** `tc_core_01_grid_snake_frame.png` is acknowledged as an invalid capture artifact. It does **not** constitute visual proof that the 30×20 grid or initial snake segments were rendered.

---

## 3. Repeated 10 Confirmation-to-Visible-Grid Presentation Measurements

To address the limitations of title-based proxy timing, a dedicated presentation campaign was executed using high-resolution DWM screen-space polling at the client-area bounding box (`measure_visible_grid_campaign.py`):

- **Evidence File:** `confirmation-to-grid/confirmation-to-visible-grid-measurements.json`
- **Visual Artifact:** `confirmation-to-grid/tc_core_01_actual_visible_grid_frame.png`
- **Results Summary (10 Repeated Observations):**
  - **Title Latency (`refreshTitle` dispatch):** Min: **60.20 ms**, Median: **61.91 ms**, Mean: **65.39 ms**, Max: **99.29 ms**
  - **Presentation Latency (DWM Client Area Polling):** Min: **2,001.85 ms**, Median: **2,252.86 ms**, Mean: **2,237.25 ms**, Max: **2,527.62 ms** (timed out at 2.0 s polling boundary)

---

## 4. Measurement Method & Technical Limitations

1. **Measurement Method:**
   - The test harness attaches to the `Default` interactive desktop, spawns `SnakeXtreme.exe`, programmatically enters the player name, navigates to Mode Selection, and triggers mode confirmation (`VK_RETURN`).
   - High-resolution `time.perf_counter()` timestamps are recorded upon sending `VK_RETURN` and upon observing window title updates.
   - The client area bounding box is polled at ~5 ms intervals via `ImageGrab` desktop screen capture to detect pixel divergence from the baseline menu state.

2. **Technical Limitations:**
   - **Workstation Lock State:** The Windows workstation host is currently in the `Windows Default Lock Screen` state. When the session is locked, Windows Desktop Window Manager (DWM) suspends front-buffer swap presentation of OpenGL/SFML backbuffers to the physical display surface. As a result, pixel changes on the visible screen surface do not composite while locked, causing presentation polling to time out.
   - **Exclusion of Photon Latency:** Presentation timing measures DWM swapchain compositing and sampling delay; it excludes physical monitor scan-out and photon emission latency.
   - **Internal vs External Timing:** Title timing precedes buffer clear, draw calls, and buffer flip, measuring internal CPU state dispatch only.

---

## 5. Status Disposition

- **Classification:** **PENDING**
- **Disposition:** First-render acceptance remains strictly **PENDING** an explicit scope decision or verification under an actively unlocked interactive desktop session.

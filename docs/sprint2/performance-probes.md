# Opt-in performance diagnostics (#22)

Owner: C Dinesh Kumar. AI-assisted implementation. **Not target-hardware acceptance.**
This follow-up is stacked on Settings PR #45, UI PR #44, and validator PR #42.

## Build and run

Build the full desktop app normally. With no arguments behavior is unchanged:
no diagnostic file, first-frame marker, or automatic shutdown is enabled.

```
./build/app/SnakeXtreme --perf-probe /absolute/path/to/NEW-run.json
./build/app/SnakeXtreme --perf-probe /absolute/path/to/NEW-run.json --probe-seconds 320
python tools/analyze_performance.py /absolute/path/to/NEW-run.json
python tools/collect_launch.py /absolute/path/to/build/app/SnakeXtreme NEW-launch-directory --runs 3 --source-snapshot TESTED_SHA_OR_HONEST_WORKTREE_LABEL
```

On Windows use the executable path in the downloaded package, retaining its
DLL, fonts and notices. `--help` does not create a window. `--probe-seconds`
accepts 1..3600, requires a report path, and counts from the first displayed
frame, **including menus and pauses**. It is not a gameplay duration control.
Quit normally to flush a report. Forced termination can leave no report.
Exit codes: 0 normal, 1 application failure, 2 invalid options/existing output,
3 report creation/write failure. Existing reports are never overwritten, even
if created after startup's preflight check; output parents must already exist.
Do not use a report filename that is also a player configuration/save file.

## Measurement definitions

- Steady-clock microseconds start at entry to `main`, before AppManager creates
  audio/window/renderer and loads configuration. `first_frame_us` is the first
  return from `window.display()`. This is NOT OS process creation to readiness.
- `collect_launch.py` measures before subprocess creation until the parent
  observes a flushed marker emitted after that first display. IPC and parent
  scheduling add delay: this is an external readiness **upper bound**. Each run
  has an isolated missing-config fixture; real player files are never modified.
  Collection succeeds only when the process exits zero without timeout, the
  marker is observed, and its regular, non-symlink raw report passes the shared
  analyzer validation with a non-null first-frame timestamp. Per-run
  `report_valid`, `report_error`, and `collection_ok` preserve failure evidence.
  Collector exit codes are 0 for all runs collected, 1 for any incomplete run,
  and 2 for invocation/setup failure. A marker alone is never success.
  The collector does not clear OS caches or verify the machine specification.
- FPS uses exact non-overlapping 1,000,000-us windows anchored on first display.
  A frame exactly on a boundary belongs to the next window. Stalls generate
  zero-frame windows; the final partial window is separate and is NOT included
  in complete-window min/mean/median. Mixed screen/mode/difficulty or restarted
  session windows are excluded from eligible gameplay summaries. Runs cannot be
  stitched across pause/menu/restart gaps into a continuous five-minute sample.
- Each directional KeyPressed received during gameplay records dequeue and
  post-engine-queue-registration times and the actual accepted/rejected result.
  Same direction, reversal, full queue and stopped-engine rejections are kept
  separate from accepted turns. **OS/SFML event-queue waiting is excluded**;
  registration time is a lower bound, not end-to-end keyboard latency.
- Engine application is recorded after the controller consumes a queued turn;
  display time is the next return from `display()`. Two queued turns have distinct
  ordinals. Restart clears pending associations, not historical records. Null
  application/display times remain explicitly unobserved. Display return is not
  a GPU scan-out or photon measurement; visual next-tick timing is informational.
- The analyzer flags accepted registration samples **>=100,000 us**, using #22's
  strict `<100 ms` criterion. A low result does not prove the unmeasured OS wait.
- Reports record only fixed screen/mode/difficulty/direction labels and timing;
  no raw text/player names/key codes. Storage caps: 3600 windows, 4096 input
  records; timestamps limited to 24 h. Drops/invalid counters are explicit and
  make a run unsuitable for acceptance. JSON is written only at shutdown, not
  per frame. The analyzer bounds reports to 8 MiB and rejects duplicate keys,
  invalid types, ordering and inconsistent timestamps. It also rejects overlapping,
  duplicate, reversed, pre-first-frame and off-grid FPS windows. Grid alignment
  uses the report's actual `first_frame_us`, not zero. Aligned gaps are allowed
  but break continuity; exact boundary and zero-frame windows remain valid.

Every report/summary retains `acceptance: not_assessed`. Power-ups are not
implemented on this dependency base, so the required power-up-active campaign
cannot be accepted here, regardless of the numbers.

## Required acceptance campaign — still pending

1. Identify actual tested commit, package, Windows 10/11 x64, CPU/RAM/GPU,
   graphics/audio driver, display mode, cache condition and timing method.
2. Cold launch: independently establish the required cold condition and collect
   external launch/readiness traces on the target device. Do not label a warm
   Xvfb trace or time beginning at `main` as a verified `<2 s` cold launch.
3. TC-PERF-01: after power-up integration, capture three **continuous** 300-s
   Hard/Obstacle sessions with active power-ups. Allow menu setup time. Record
   all one-second windows (including stalls), min/mean/median and any FPS<30.
   Death/restart, pause, mixed windows, drops or short runs are not substitutes.
4. TC-PERF-02: record 100 randomized input requests; retain accepted/rejected
   counts, unobserved applications and latency distributions. Independently
   instrument/bound event injection to SFML dequeue if claiming end-to-end
   latency; do not infer this missing interval from the internal lower bound.
5. TC-USE-01: three independent first-time volunteers; record interactions,
   completion times, observed issues and actual feedback without fabricating it.
6. TC-REL-01: missing/corrupt configuration must recover without a crash, then
   complete Windows startup and C13 logging/security integration checks.

## Reproducible tooling checks

```
ctest --preset headless --output-on-failure
python -m unittest discover -s tests -p 'test_*.py' -v
python tools/desktop_navigation_smoke.py /absolute/path/to/SnakeXtreme
python tools/desktop_settings_smoke.py /absolute/path/to/SnakeXtreme
python tools/desktop_probe_smoke.py /absolute/path/to/SnakeXtreme NEW-output-directory
```

Linux desktop scripts require Xvfb/xdotool and refuse occupied displays :97,
:99 and :98 respectively. They clean up only their own processes and use
isolated temporary settings with null audio/software rendering. Null audio is
not a human listening check. The probe smoke sends 100 synthetic seeded-random
requests in a short Classic/Easy burst and verifies their exact recorded order,
then collects three launch traces. This avoids mistaking keys sent during a
Game Over transition for missing gameplay samples. It is **tooling smoke, not
STP acceptance**: it does not replace the sustained Hard/Obstacle campaign,
randomized target-hardware latency study or verified cold-launch measurements.
See `evidence/owner-followup-results.md` for this follow-up's executed results.

See `evidence/owner-tooling-fix-results.md` for the subsequent PR #46 collector
and FPS-grid regression fixes; earlier evidence applies to its own snapshot.

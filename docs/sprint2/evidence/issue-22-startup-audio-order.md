# Issue #22: start-up phases and audio after the first frame (PR #69), Linux regression

Executed 31 Oct 2026 by B V TEJAS. This is AI-assisted **Linux diagnostic and regression evidence only**. It is **not** cold-launch acceptance, **not** Windows evidence and **not** an audible test. The cold-launch criterion (< 2 s) stays **Pending** until it is measured on the target Windows hardware with real audio enabled.

It does not touch Bharath's pinned Windows 11 candidate `3fcc586`, and no existing evidence is changed.

## Builds

| Build | GitHub commit | Tree (identical to the tested local tree) | What it is |
| --- | --- | --- | --- |
| Baseline | `3fcc586` (= `main` `447b84c` product code) | `41b54378…` | Unchanged; device opened before the window |
| Stage 1 | `26d9dba6fd85a87a09aa46da513461b47bbd26b5` | `95b1606414f78e4083e42be74c21137fe6a5771a` | Opt-in start-up phase marks only; start-up order unchanged |
| Stage 2 | `8411808f2682403cbdcd5f43d0fe742b56edd9da` | `a4b1b13e391ee06029a2bd1c65d78bb4ec7eafba` | Audio device opened after the first frame |

The commits after stage 2 on this branch change documentation and evidence only. A build of the PR head has the same product source as stage 2.

**Environment:** Ubuntu 24.04 container, GCC 13.3.0 (Release, `-Werror`, static SFML 2.5.1), Clang 18.1.3 (ASan + UBSan), cppcheck 2.13.0. Desktop runs used Xvfb with software OpenGL (llvmpipe). There is no real audio device here: `ALSOFT_DRIVERS=null` gives OpenAL Soft's null output, and with the default drivers no device is found, so the silent fallback is used.

The Linux executable hashes below identify these Linux builds only. They are not Windows EXEs.

| Build | Linux `SnakeXtreme` size | SHA-256 |
| --- | --- | --- |
| Baseline | 1,352,680 | `632bb5193a8a916b42e5bcf8d012847f2f5842e7471c35964097d208fd8f8906` |
| Stage 1 | 1,361,480 | `da1b54e4007186d60ca34522e4e2f267d21968d688f4dada7f0d62dc1a099a02` |
| Stage 2 | 1,361,832 | `a22f675ce86e9c77d91913d6234db38e0980970044707eea9d60880249675c19` |

## Regression results

| Check | Stage 1 | Stage 2 |
| --- | --- | --- |
| CTest, GCC Release, normal user | 21/21 | 21/21 |
| CTest, Clang ASan + UBSan + leak, normal user | 21/21, 0 reports | 21/21, 0 reports |
| `performance_probe_tests` (2 new cases) | 99 assertions / 13 cases | 99 / 13 |
| cppcheck gate | 0 findings | 0 findings |
| Banned-API / layering | 0 hits in 47 files / 35 SFML-free files | same |
| Python tooling tests (6 new) | 39 pass | 39 pass |
| `audio_backend_smoke`, null output | — | 7/7 |
| `audio_backend_smoke`, no device | — | Fails cleanly with "Audio backend not available" |
| Desktop navigation (Xvfb) | — | 23/23 (Release); 23/23 with the game built under ASan + UBSan |
| Desktop Settings, including the invalid-driver silent fallback | — | 22/22 (Release; first attempt failed at window start-up, rerun passed); 22/22 under ASan + UBSan |
| Leaderboard UI (`leaderboard_ui_check.py`, unchanged) | — | 75/75 |
| Probe smoke | pass | pass (first attempt failed at application start-up, rerun passed) |
| Hosted CI on the PR | 8/8 on `26d9dba` ([run 41225101894](https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/41225101894)) | see the PR checks |

**Harness start-up failures (not hidden).** `desktop_settings_smoke.py`, `desktop_probe_smoke.py` and `desktop_leaderboard_smoke.py` sometimes failed in this container at window or X11 start-up ("application window did not appear", "application did not start", "Failed to open X11 display"). The **unchanged baseline `3fcc586` failed the same way**:

- Settings smoke: failed once on the baseline, while stage 2 passed on rerun.
- Leaderboard smoke: on the baseline, 0 then 15 checks before an X11 open failure; on stage 2, 7 then 0.

Because it failed the same way on the unchanged baseline, I treat it as an Xvfb problem in this container and have not attributed it to the change. The leaderboard behaviour itself was covered by `leaderboard_ui_check.py` (75/75), which this change does not touch.

## Start-up timelines on Linux (`collect_launch.py`, milliseconds per step)

Each value is the time since the previous mark. A step also includes anything else that ran since that mark.

**Stage 1 (current order), null audio, 5 runs:**

| Run | audio objects (device open) | window created | renderer | settings | audio started | first frame | external marker |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 1.2 | **1,871.3** | 0.9 | 17.1 | 1.3 | 77.1 | 1,975.5 |
| 2 | 0.9 | 79.3 | 0.1 | 2.0 | 1.2 | 16.9 | 107.7 |
| 3 | 1.1 | 44.4 | 0.1 | 17.9 | 1.2 | 24.0 | 95.3 |
| 4 | 1.1 | 72.3 | 0.1 | 9.7 | 1.3 | 20.8 | 113.7 |
| 5 | 1.0 | 52.5 | 0.1 | 5.6 | 1.3 | 28.3 | 99.2 |

**Stage 2 (audio after the first frame), null audio, 5 runs:**

| Run | audio objects | window created | renderer | settings | first frame | audio started (after the frame) | external marker |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 0.0 | 42.9 | 0.1 | 2.0 | 14.6 | 2.3 | 65.3 |
| 2 | 0.0 | 39.6 | 0.1 | 2.4 | 18.2 | 2.2 | 64.9 |
| 3 | 0.0 | 38.4 | 0.1 | 1.8 | 14.8 | 2.2 | 59.7 |
| 4 | 0.0 | 41.0 | 0.1 | 0.9 | 15.7 | 2.3 | 62.6 |
| 5 | 0.0 | 50.6 | 0.2 | 2.5 | 16.2 | 2.2 | 74.4 |

**No audio device.** Three runs each with the default drivers end in `audio_unavailable` and keep the silent fallback.

- Stage 1: the step is 1.3–1.4 ms, before the first frame.
- Stage 2: the step is 3.7–3.8 ms, after the first frame.

**What the Linux runs show:**

- The marks are recorded in the intended order.
- In stage 2 the audio step comes after `first_frame`.
- The first launch of a newly built binary was dominated by `window_created` (1.87 s with software OpenGL). Later launches spent 40–80 ms there.

**What they do not show.** With the null driver, opening the audio device takes about 1 ms, so these runs cannot show how much the fix saves on Windows. That needs a Windows run with real audio.

## Still Pending on Windows

1. Build **stage 1** (`26d9dba`) and **stage 2** (`8411808`, or the PR head) on the measured Windows 10 PC. Record each EXE's size and SHA-256.
2. For each build, do one launch after a reboot, then five more:

   ```
   collect_launch.py <EXE> <NEW dir> --runs 5 --source-snapshot <SHA>
   ```

   Use **normal audio**: do not set `ALSOFT_DRIVERS`. For each run, record `startup_step_us`: `audio_objects_constructed`, `window_created`, `first_frame` and `audio_started`.
3. Check by ear that music plays and that food, Game Over and F5 SFX play on stage 2. Check that SFX volume 0 is silent.
4. Decide on the cold-launch criterion only from real-audio runs on the target hardware.

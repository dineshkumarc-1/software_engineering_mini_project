# #22 Windows retest: normal-audio start-up, profiling build vs fix build

For Dinesh, on the **same Windows 10 PC** used for the earlier launch runs. Author: B V TEJAS.

This compares two builds: the **profiling build** (current start-up order, with timing marks) and the **fix build** (audio opened after the first frame).

- **Do not use or rebuild Bharath's pinned Windows 11 checkout** (`3fcc586`).
- **Do not overwrite earlier evidence folders.** Every command below writes to a new directory.

**Nothing here is acceptance.** The cold-launch target and FR-001 stay **Pending** until these results are reviewed.

## 0. Builds under test

| Label | Commit | Where to find the SHA |
| --- | --- | --- |
| **P**, profiling | Head of the profiling PR | The profiling PR's description and #22 |
| **F**, fix | Head of PR #69 | PR #69's description and #22 |

Make one clean checkout per build in a new folder, from a *Developer PowerShell for VS 2022*:

```powershell
git clone https://github.com/dineshkumarc-1/software_engineering_mini_project.git startup-P
cd startup-P
git checkout --detach <P_SHA>
git rev-parse HEAD                    # must print <P_SHA>
git status --porcelain                # must print nothing
cmake -S . -B build -DSNAKE_FETCH_SFML=ON -DBUILD_SHARED_LIBS=OFF
cmake --build build --config Release --parallel
ctest --test-dir build -C Release     # expect 20/20 (secure_write_fault_tests is Linux-only)
(Get-Item .\build\Release\SnakeXtreme.exe).Length
Get-FileHash .\build\Release\SnakeXtreme.exe -Algorithm SHA256
cd ..
```

Repeat in a folder named `startup-F` with `<F_SHA>`. Record both EXE sizes and SHA-256 hashes. Run each build's own `tools\collect_launch.py`; older copies of the tool do not report the start-up phases.

## 1. Make sure the audio is real

```powershell
Remove-Item Env:ALSOFT_DRIVERS -ErrorAction SilentlyContinue
[Environment]::GetEnvironmentVariable('ALSOFT_DRIVERS', 'User')     # must be empty
[Environment]::GetEnvironmentVariable('ALSOFT_DRIVERS', 'Machine')  # must be empty
```

Use the PC's normal speakers or headphones, and note which output device is the default. Every `launch-summary.json` must show `"audio_null_output": false`, and every run must contain the `audio_started` phase. A run with `audio_unavailable` is **not** a normal-audio measurement: stop and report it.

## 2. Launch series (alternate the order across two boots)

The collector does not clear caches. "Cold" here means the first launch after a reboot and about 2 minutes idle; record it that way. Each command needs a **new** output directory.

**Boot 1**

```powershell
python startup-P\tools\collect_launch.py startup-P\build\Release\SnakeXtreme.exe diag-P-boot1-first --runs 1 --source-snapshot <P_SHA>
python startup-P\tools\collect_launch.py startup-P\build\Release\SnakeXtreme.exe diag-P-boot1-warm  --runs 5 --source-snapshot <P_SHA>
python startup-F\tools\collect_launch.py startup-F\build\Release\SnakeXtreme.exe diag-F-boot1-warm  --runs 5 --source-snapshot <F_SHA>
```

**Boot 2** (reboot, wait about 2 minutes)

```powershell
python startup-F\tools\collect_launch.py startup-F\build\Release\SnakeXtreme.exe diag-F-boot2-first --runs 1 --source-snapshot <F_SHA>
python startup-F\tools\collect_launch.py startup-F\build\Release\SnakeXtreme.exe diag-F-boot2-warm  --runs 5 --source-snapshot <F_SHA>
python startup-P\tools\collect_launch.py startup-P\build\Release\SnakeXtreme.exe diag-P-boot2-warm  --runs 5 --source-snapshot <P_SHA>
```

To print each run (external marker, then each step in ms):

```powershell
python -c "import json,sys; d=json.load(open(sys.argv[1])); print('null audio:', d['audio_null_output']); [print(r['run'], r['collection_ok'], round(r['launch_to_observed_first_frame_marker_us']/1000,1), {k: round(v/1000,1) for k,v in (r['startup_step_us'] or {}).items()}) for r in d['runs']]" diag-P-boot1-first\launch-summary.json
```

## 3. What to report

For every run, record:

- build, commit and EXE SHA-256;
- boot, and whether it was the first run after that boot;
- `collection_ok`;
- the external marker in ms;
- the steps `audio_objects_constructed`, `window_created`, `renderer_ready`, `settings_ready`, `first_frame`, and `audio_started`.

Interpreting the results:

- **On P**, `audio_objects_constructed` is the cost of opening the audio device. `window_created` is the cost of the window and OpenGL. The first run after each boot shows which of the two causes the slow first launch.
- **On F**, `audio_objects_constructed` should be close to 0, `audio_started` should come after `first_frame`, and the time to `first_frame` should drop by about P's audio step.
- **If P's first launch spends most of its time in `window_created`**, the slow first launch comes from graphics start-up. PR #69 does not address that.

## 4. Listening check on F (no probe)

Start `startup-F\build\Release\SnakeXtreme.exe` normally. Record the tester, date, EXE hash, output device and volume values.

| # | Check | Expected |
| --- | --- | --- |
| 1 | Menu appears; listen | Background music starts within about a second of the menu appearing and loops |
| 2 | Settings → F5 | Preview sound once |
| 3 | Play, eat food | Food sound once per food |
| 4 | Collide | Game Over two-tone alert, once |
| 5 | Settings: SFX 0, save; repeat 2–3 | No SFX; music unaffected |
| 6 | Settings: Music 0, SFX 70, save | Music silent; SFX audible |
| 7 | Esc during play, then resume | Music pauses, then continues |

Record each check separately. Do not infer any of them from the probe or CI.

## 5. FR-001 / TC-CORE-01 on F

Follow the 10-run procedure in [startup-measurements.md](startup-measurements.md) with the F build. Report every run's `confirm_to_first_grid_us` and the snake check. This is the STP's actual 2-second requirement. The launch series above measures process start-up to the menu, which is a different question.

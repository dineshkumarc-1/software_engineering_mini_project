# Issue #20 — Settings implementation slice

Owner: C Dinesh Kumar. AI-assisted implementation. **Draft; #20 remains open.**

**Current update:** the audio implementation and Linux desktop smoke tests below
supersede the earlier audio-not-implemented/local-runtime-pending notes. Human
Windows listening and C13 integration are still pending.

## Dependencies and contract

Based on UI PR #44 (`5331dd639122f00d4c36929268aa6d684b87c581`), which in
turn depends on validator PR #42. Target the UI branch for review; do not merge
this independently into main before its dependencies are integrated.

Contract: approved SRS/SAD/STP PDF, FR-026..029, SAD §4.3.3/§4.3.5 and
SRS Addendum defaults. `config.json` is relative to the working directory.
Fields: version=1; key_up/down/left/right as SFML 2.x integer codes;
music_volume/sfx_volume integers 0..100; theme Classic/Neon/Retro.
Defaults are arrow keys, 70/70 volumes and Classic; WASD aliases stay enabled.
Only letters (except reserved M) and arrows are offered as primary movement
keys. Duplicate keys and contradictory WASD assignments are rejected inline.
Other keys remain reserved for navigation. Confirm any broader key requirement
before expanding the accepted binding set.

## Implemented

- SFML-free Settings, transactional SettingsEditor and C12 SettingsRepository.
- Keyboard and mouse settings screen; key capture with Esc cancellation,
  independent click/drag volume sliders, theme cycling, restore defaults,
  explicit Save & back and Cancel. Unsaved edits do not affect active settings.
- Gameplay dispatch uses remapped primary keys plus permanent WASD aliases.
  Old primary arrows stop steering after remap; pause/menu controls remain.
- Classic, Neon and Retro change board background, grid and snake colours;
  no engine or difficulty rules are changed. Theme preview is visible in Settings.
- Startup loads settings; invalid/missing files use defaults, emit a bounded
  diagnostic code on stderr, and attempt to regenerate config.json. Failed save
  stays on Settings and does not apply changes or claim success.
- Schema checks: exact types/ranges/enums, all required fields, unique bindings,
  version 1, 64-KiB byte limit, depth limit 8, duplicate root-key rejection.
  Valid unknown fields are ignored. Files read via symlink are rejected.
- Save: validate, create exclusive sibling temporary file, write/flush/close,
  atomic replace (rename on POSIX; MoveFileExW on Windows); cleanup owned temp
  on failure, keep old target. JSON dependency: nlohmann/json v3.11.3, fetched
  by CMake, MIT-licensed. First configure needs network access.

## Validation evidence

Executed 2026-10-27 UTC on Amazon Linux 2023, GCC 11.5.0, CMake 4.4.4,
Catch2 v3.8.1, nlohmann/json v3.11.3 (`9cca280a4d0ccf0c08f47a99aa71d1b0e52f8d03`).
Evidence applies to the source snapshot containing this report; use the PR's
head commit SHA to identify the published snapshot. The compiler/build commands
and raw outputs are recorded below and in `evidence/settings-*.txt`.

```
cmake --preset headless -DSNAKE_WARNINGS_AS_ERRORS=ON
cmake --build --preset headless -j4
ctest --preset headless --output-on-failure
./build/headless/settings_tests --reporter compact

g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Iinclude \
  -isystem /data/sfml-2.5.1/include -fsyntax-only \
  src/presentation/AppManager.cpp src/presentation/Renderer.cpp src/main.cpp
```

Results: 9/9 registered CTest suites pass. Settings has 16 cases / 191 assertions.
Presentation syntax passes against SFML 2.5.1 headers; **not linked or run**.
Settings tests also pass with GCC ASan + UBSan and leak detection:
configure a Debug headless build with `-fsanitize=address,undefined
-fno-omit-frame-pointer` in CMAKE_CXX_FLAGS and sanitizer linker flags, build
`settings_tests`, run with `ASAN_OPTIONS=detect_leaks=1`. On this agent the GCC
runtime libraries needed local symlinks/LD_LIBRARY_PATH; this is an environment
workaround, not an application dependency or source change.

| Approved test | Evidence / remaining gap |
| --- | --- |
| TC-SET-01 | Automated mapping/WASD/old-primary/duplicate checks pass; desktop steering and pointer/key-capture checks pending |
| TC-SET-02 | Independent volume state, clamps and JSON persistence tested; audible channel checks BLOCKED |
| TC-SET-03 | Theme model cycle/reset tested; theme rendering syntax-checked, visual runtime check pending |
| TC-SET-04 | Fresh repository instances restore all fields, boundaries and themes; real application restart on Windows pending |
| TC-REL-01 | Missing, malformed, truncated, wrong-type, out-of-range, unknown theme, oversized/deep/duplicate-key fixtures tested; repository regeneration tested; actual startup/UI/log-file checks pending |

## Not complete / integration notes

- **Audio is not implemented in this baseline**: SFML audio is disabled and no
  AudioManager or music/SFX assets exist. Sliders store independent values but
  do not alter audible channels yet. TC-SET-02 and #20 must remain open.
- C13 SecureFileStore (#15) is still a placeholder on this base. This interim
  adapter must be reconciled with Bharath's shared bounded-read/atomic-write
  API before final acceptance. It does not provide disk-fsync durability,
  stale-temp scavenging, final threat-model review, or snakextreme.log rotation.
- Headless sanitizer success is not full-app security or STP sign-off.
- Windows/macOS CI, full graphical/manual execution, startup timing and target
  hardware performance still require verification; no final claims for #22/#25.

## Manual follow-up

Open Settings by both keyboard and mouse (opening it must not also edit a row).
Remap directions to I/K/J/L; verify WASD aliases and old arrows; reject a duplicate.
Drag each volume to 0 and 100 and verify the other stays unchanged. Preview each
theme; Cancel and reopen (old settings retained); Save and relaunch (values
restored). Restore defaults then Cancel, then repeat and Save. Test unwritable
config target: error stays visible and active settings remain unchanged. Launch
with corrupt config and check default recovery. Exercise all of this on Windows.

## Updated validator integration and Linux CI fix (2026-10-27 UTC)

Synchronized UI integration from PR #44 snapshot
`0abc73f8d44ee7e8fb6dac96ea025f975e3527a0` into this Settings branch.
The shared validator header/source/tests match Bharath PR #42 revision
`517c169433b4139d047acaead806314dd524de03` exactly. Name-entry typing uses
`isAllowedPlayerNameCharacter`; submission calls `isValidPlayerName` before
`sanitizePlayerName`. UI-specific blank/trim implementation removed.

Revalidation on the combined Settings branch: 9/9 CTest suites, 186 navigation/
name-entry checks, 191 Settings assertions, and 20 consecutive gameplay suite
runs pass. Settings ASan+UBSan tests (191 assertions) pass with leak detection;
UI model ASan+UBSan coverage is recorded in the #19 report. Presentation passes
strict syntax checks against SFML 2.5.1. Environment/commands as above.
New raw results: `evidence/validator-settings-*.txt`.

The previous Settings CI run at b0daa54 had 5 passing jobs but Linux GCC failed:
`SettingsRepository.cpp:80: error: ignoring attributes on template argument
'int (*)(FILE*)' [-Werror=ignored-attributes]`. Job:
https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/40452633928/job/122018661569?pr=45

Replaced the fclose function-pointer deleter with a concrete FileCloser functor.
RAII ownership/close behavior is unchanged; no warning flag was weakened.
Local warnings-as-errors build and tests pass; the new GitHub CI run must confirm
this on its compiler before the portability fix is considered CI-verified.
These updates do not resolve audio/manual/Windows/C13 acceptance gaps above.

## AudioService and Linux desktop integration (2026-10-27 UTC)

Base: published Settings head `b380a921eb2a08026e22d12d9fc41eb9794237ff`.
C5 now plays an original generated musical loop and preview/food/game-over SFX
through SFML Audio, using independent music and SFX gains. The two channels are
not mixed into one gain. Volume edits preview immediately; F5 plays a test SFX.
Save persists values, Cancel restores saved levels, pause suspends music/SFX,
and unavailable audio leaves a playable silent fallback with a Settings notice.
The waveform model is SFML-free, deterministic, bounded to a four-second music
loop and <=350-ms SFX, and uses no third-party audio assets. Buffers outlive sound
sources. SFML audio and its runtime/dependencies are now enabled in CMake/CI.

Windows packaging now includes fonts, runtime notices and the matching SFML
bundled OpenAL 1.18.1 DLL, plus its LGPL v2 text and upstream source link. The
new optional `audio_backend_smoke` executable is not an audio-device-dependent
CTest; Linux CI runs it explicitly using OpenAL Soft's null output driver.

### Executed validation (not final system acceptance)

Environment: Amazon Linux 2023; GCC 11.5.0; CMake 4.4.4; SFML 2.5.1; Catch2
v3.8.1; local OpenAL Soft 1.23.1 (`d3875f333fb6abe2f39d82caca329414871ae53b`).
OpenAL was built under `/data/audio-deps` because this agent's distribution did
not package its development headers. This is a local build detail, not a new
project dependency/version mandate. CI uses system OpenAL on Linux and SFML's
bundled 1.18.1 DLL on Windows. Xvfb/xdotool test a real Linux SFML window.

- Full desktop configure/link/build succeeds with project warnings as errors.
- Headless CTest: **10/10**; full-app-build CTest: **10/10**.
- Audio model: **4 cases / 147 assertions**, also ASan+UBSan/leak-detection pass.
- Real SFML/OpenAL backend: **7 checks** pass, using a null output driver.
  Includes separate 0/100 and 100/0 gains, clamping, pause/resume and restored
  gains. Non-boundary backend float gains use a 0.01 tolerance (OpenAL roundoff).
- Real Linux/Xvfb Settings UI: **17 checks** pass, including independent slider
  states, Save/Cancel, I/K/J/L remapping, theme selection, real process relaunch,
  missing/corrupt config regeneration and no-audio-backend silent fallback.
- Existing name-entry, shared validator, Settings and gameplay tests remain in
  the full CTest pass. Prior 20-run gameplay evidence is historical, not rerun
  as part of this audio slice.

Raw evidence: `docs/sprint2/evidence/audio-*.txt`. These results apply to the
source snapshot containing this addendum; use its published PR head SHA.

### Reproduction

```
cmake --preset headless -DSNAKE_WARNINGS_AS_ERRORS=ON
cmake --build --preset headless -j4
ctest --preset headless --output-on-failure
./build/headless/audio_model_tests --reporter compact
cmake --preset app -DSNAKE_WARNINGS_AS_ERRORS=ON
cmake --build --preset app -j4
ctest --preset app --output-on-failure
ALSOFT_DRIVERS=null ./build/app/audio_backend_smoke
python tools/desktop_settings_smoke.py /absolute/path/to/build/app/SnakeXtreme
```

For the agent's non-system OpenAL prefix, add CMAKE_PREFIX_PATH=/data/audio-deps
and LD_LIBRARY_PATH=/data/audio-deps/lib. Optional
`SNAKE_SMOKE_SCREENSHOT=/path/to/settings.png` captures a Pillow screenshot.
The script creates/cleans an isolated Xvfb display :99 and temporary config dir,
refuses to attach to an occupied display, and never edits real player data.

**Remaining:** no human has listened to music/SFX on the required Windows device;
null audio proves backend routing/status, not audible correctness. Windows app
runtime and mouse-drag/manual acceptance still need verification. C13 shared
store/logging integration remains separate. Keep #20 open and this PR draft;
CI on the new head is separate from the previously green run. No release tag,
freeze or complete-product demo is justified by this slice.

# Owner hardening results — #19 / #20 / #22

AI-assisted C Dinesh Kumar owner work on top of PR #46 source
`0ce1981bb45459e7cda1c90a76543bc13f2c6de7`. These results apply to the changed
working-tree inputs identified in `owner-hardening-source-sha256.txt`, not the
parent SHA by itself. Published file blobs must match that source manifest.
Previous `owner-followup-*` evidence remains historical for its own snapshot.

## Executed environment and commands

Amazon Linux 2023 x64, GCC 11.5.0, CMake 4.4.4, Python 3.13.15;
Catch2 3.8.1, JSON 3.11.3, SFML 2.5.1, local OpenAL Soft 1.23.1;
Xvfb/xdotool, software GL and null audio. Dependency setup is local environment
work, not new project dependency versions. No usernames, machine hostname or
absolute player paths are needed in shared results.

- Configure headless with `SNAKE_WARNINGS_AS_ERRORS=ON`; build and `ctest --preset
  headless --output-on-failure`: **11/11 suites pass** (`owner-hardening-ctest.txt`).
- `app_state_tests`: **428 checks pass** (`owner-hardening-name.txt`), including
  40 fixed-buffer flood/backspace/normalize/reset cycles and independent copies.
- Build that navigation harness using `-fsanitize=address,undefined
  -fno-omit-frame-pointer`, run with `ASAN_OPTIONS=detect_leaks=1`: **428 checks
  pass** (`owner-hardening-name-asan.txt`). This is not full-app sanitizer sign-off.
- `python -m unittest discover -s tests -p 'test_*.py' -v`: **19 tests pass**
  (`owner-hardening-python.txt`), comprising 10 report-analysis and 9 acceptance
  preparation tests. Acceptance tests use explicit non-executable fixtures and
  never award manual passes or verify an operator-supplied commit's provenance.
- Full desktop configure/link/build passes with warnings as errors. This local
  app build used `BUILD_TESTING=OFF`; the 11-suite result above is from headless,
  not a second app-build CTest run. Desktop runtime checks below use that app.
- `tools/desktop_navigation_smoke.py`: **23 real-window checks pass**
  (`owner-hardening-navigation.txt`), repeating all nine mode/difficulty flows,
  name boundaries, mouse controls, pause/restart and Settings pointer checks.
- `tools/desktop_settings_smoke.py`: **22 real-window checks pass**
  (`owner-hardening-settings.txt`): previous 17 plus failed-Save error/stay,
  target preservation, owned-temp cleanup, Cancel rollback and fixture recovery.
- `tools/desktop_probe_smoke.py`: **100 synthetic requests and 3 external launch
  traces pass tooling smoke**, with no name sentinel leakage or dropped/invalid
  records (`owner-hardening-probe.txt`). This is not target performance acceptance.

CMake was configured using cached Catch2/JSON/SFML sources and
`CMAKE_PREFIX_PATH=/data/audio-deps`; runtime scripts used
`LD_LIBRARY_PATH=/data/audio-deps/lib`. Sanitizer shared libraries needed a local
runtime search path. Standard machines/CI may not require those environment paths.

## Scope and pending acceptance

Shared validator files are unchanged. Existing staged changes in the original
workspace are untouched; implementation is isolated from teammates' branches.
No files under `docs/qa` change. No competing persistence API is introduced.
The shared C13 adapter/leaderboard integration remains blocked on the reviewed
PR #48 interface/behavior fixes and subsequent integration verification.

New-head CI must be observed separately. Human Windows/audio/F5 checks must be
reconciled with the actual tested artifact; no reported result is silently
upgraded to final acceptance. Cold target launch, full end-to-end input latency,
3x300-second power-up-active performance campaign and independent volunteer
usability remain pending. `prepare_acceptance.py` creates only an UNEXECUTED
Pending template, never QA sign-off. Issues remain In Progress/open; PR draft;
no merge, release tag or freeze.

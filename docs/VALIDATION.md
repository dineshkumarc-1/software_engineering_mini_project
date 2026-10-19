# Local validation — issue #4

Environment: Amazon Linux 2023, GCC, CMake 3.22.2, SFML 2.5.1 built from its tagged upstream source, Release build. GUI smoke tests ran under Xvfb (virtual X11 display), not Windows.

## Executed results

| Check | Result |
| --- | --- |
| Full SFML configure and build | PASS |
| CTest with application build | PASS: 1/1 test executable |
| SFML-free configure/build with `SNAKE_BUILD_APP=OFF` | PASS |
| Headless state-flow checks in Release | PASS: 28 checks |
| Native window client area | PASS: 900×600 |
| Initial menu → preview | PASS |
| Esc pause/resume | PASS |
| Enter resumes paused preview | PASS |
| M returns to menu from preview and pause | PASS |
| Esc exits from menu | PASS: exit code 0 |
| WM_DELETE_WINDOW from menu | PASS: exit code 0 |
| WM_DELETE_WINDOW from preview | PASS: exit code 0 |
| WM_DELETE_WINDOW from pause | PASS: exit code 0 |
| Whitespace/diff check | PASS |

Native smoke testing used an external X11 driver to send key events and the standard WM_DELETE_WINDOW protocol message, inspect the title/state and client dimensions, and wait up to five seconds for shutdown. Four application instances were launched and exited successfully. This validates the Linux window-event path; it does not simulate a Windows graphics driver.

## Not executed / not claimed

- Windows 10/11 compilation or runtime checks.
- Sustained FPS on the specified reference hardware (NFR-001 / TC-PERF-01).
- Input latency, engine integration or full gameplay tests.
- Focus-loss automated test (manual checklist remains).
- Sanitizer, coverage, Catch2 or CI integration.

The issue has implementation and local smoke-test evidence, but should not be treated as fully Windows-validated or moved to Done automatically.

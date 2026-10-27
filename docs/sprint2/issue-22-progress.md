# Issue #22 — Linux Settings/recovery system smoke slice

Owner: C Dinesh Kumar. AI-assisted. **Partial; issue stays open.**

`tools/desktop_settings_smoke.py` drives a real SFML application with Xvfb and
xdotool. It validates 17 Settings, restart and recovery checks on Linux; raw
results: `evidence/audio-desktop-smoke.txt`. The exact environment, build and
source-snapshot identification are in `issue-20-progress.md`'s audio addendum.
It uses a temporary working directory and null-output audio; player files and
shared browser sessions are not used. It refuses an occupied display :99.

| Planned coverage | Current evidence |
| --- | --- |
| TC-SET-01 | I/K/J/L key-remap UI capture and serialized codes tested; actual steering/duplicate UI/manual tests remain |
| TC-SET-02 | Independent slider UI states and real SFML/OpenAL gain routing verified; human audible Windows test pending |
| TC-SET-03 | Theme choice saved/reloaded, Settings visual preview inspected; every gameplay theme/manual run pending |
| TC-SET-04 | Save, Cancel and real process relaunch tested on Linux; Windows runtime pending |
| TC-REL-01 | Missing/corrupt config defaults and actual startup regeneration tested; warning on stderr; final C13 log policy pending |
| TC-PERF-01 | NOT EXECUTED: no 3x5-minute FPS runs on target hardware |
| TC-PERF-02 | NOT EXECUTED: no 100-input acknowledgement latency measurements |
| Cold launch <2 s | NOT EXECUTED on target hardware; script polling timeouts are not startup benchmarks |
| TC-USE-01 | NOT EXECUTED: three independent first-time volunteers still required |

Do not interpret the 17 smoke checks or the 10 CTest suites as the complete STP
inventory. The script does not measure input latency, FPS or cold-launch timing.
It is a reproducible regression aid, not proof of target hardware or usability
acceptance. No final sign-off for #22, #25, #26 or #28 is claimed.

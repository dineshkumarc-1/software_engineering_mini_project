# Issue #22 — measured Linux diagnostics, NOT target-device acceptance

These are actual executions of the reconciled working-tree candidate. Source bytes
are identified by SOURCE-SHA256.txt; the collector's operator-supplied source label
explicitly says uncommitted smoke, not a release build. Do not relabel these runs
as a final published Windows artifact or cold-cache measurements.

Amazon Linux 2023 x64, software OpenGL/Xvfb, null audio. Classic/Easy seeded short
burst; 100 requested inputs per run in order, **5 accepted and 95 rejected per
run**. These are NOT 100 accepted turns, random target-device inputs, or three
300-second Hard/Obstacle sessions with power-ups.

| Run | Complete 1-second FPS min/median/max | Accepted/rejected inputs | Internal registration lower bound (microseconds) | First display after application (ms) | External process-start to observed first-frame marker (ms; 3 launches) |
| --- | --- | --- | --- | --- | --- |
| 1 | 53 / 58.0 / 60 | 5 / 95 | 0–2 | 184.4–407.7 | 178.7–272.4 |
| 2 | 55 / 59.0 / 60 | 5 / 95 | 0–2 | 185.2–410.5 | 160.7–194.8 |
| 3 | 58 / 59.5 / 60 | 5 / 95 | 0–2 | 184.1–404.3 | 127.4–156.5 |

Across nine launches: min 127.4 ms, median 173.1 ms,
max 272.4 ms. Cache state is **unverified**. External marker collection
includes IPC/scheduling and is an upper bound; internal main-to-first-frame is not
OS process cold-launch time. No complete FPS window was below 30 in these short
smokes, but no eligible long Hard/Obstacle session was collected.

Internal input registration excludes OS/SFML queue time and is a lower bound,
not end-to-end acknowledgement. First-display timestamps mean display() returned,
not measured photons; next-tick display values are informational. STP acceptance
concerns acknowledgement/engine registration, not a newly invented photon-latency
gate. None of these measurements establishes the target <100 ms requirement.

## Still Pending on the exact Windows candidate

1. Record candidate SHA, Actions run/artifact link, EXE SHA-256, OS and hardware.
2. Verify specified reference hardware and collect actual cold launch evidence.
3. Three continuous 300-second Hard/Obstacle runs with power-up activity; retain
   every 1-second FPS window and report any window below 30.
4. 100 randomized timed inputs with valid engine-acknowledgement measurement;
   retain accepted/rejected results and explain measurement scope.
5. Three outside-team first-time volunteers: at most three screens, no help;
   record actual observations/consent without unnecessary personal data.

Raw JSON and runtime logs remain private in the local build workspace, not in the
repository. Only whitelisted aggregate metrics are shared in linux-probe-aggregates.json.
All reports retain acceptance:not_assessed. Do not mark #22, UAT or final QA accepted.

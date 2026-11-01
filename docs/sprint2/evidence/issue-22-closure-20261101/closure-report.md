# Issue #22 closure review package — 2026-11-01 (initial review snapshot)

**Initial disposition: Not ready for Done.** This package reconciles evidence against
the approved STP v1.0 and records the remaining work. It does not claim a pass,
waiver, PR merge, Sprint 2 completion, or #28 demo. PR #69 remains Draft.

The issue owner later directed that TC-PERF-01 remain unchecked and #22 proceed
to issue-tracking closure. The current disposition and subsequent evidence are
recorded in the [owner disposition addendum](owner-disposition-addendum.md),
which supersedes this document for current #22 tracking status. This initial
review snapshot is retained as history.

## Candidate and environment

- PR #69 head reviewed: `841ee1dccac37236a4bdcf92ec3dbf17e7a18c80`.
- Product source revision in that head: `8411808f2682403cbdcd5f43d0fe742b56edd9da` (Stage 2); the head adds evidence documentation only.
- PR #69 changes startup order: render/display the first frame, then initialize audio. This addresses where audio startup time is charged, but is not evidence of a <2-second first launch. The draft has no Windows test results or reviews recorded.
- Windows target identified: Windows 10 Pro 22H2 / build 19045; Dell Inspiron 15-3567; Intel Core i3-6006U, 2 cores; Intel HD Graphics 520 driver 31.0.101.2111 and AMD Radeon R5 M330 driver 27.20.1034.6.
- Local Windows Release build from the exact PR #69 head: `source-pr69/build-pr69-app/Release/SnakeXtreme.exe`, 1,911,808 bytes, SHA-256 `341bd6394a93741137866c88d95c3b00ae9ccb6d160e19d9044a0fddde68b57b`; MSVC 19.44.35207 / VS 2022 Build Tools, static SFML, Windows SDK 10.0.26100. This is a locally built candidate, not a PR Actions artifact.
- The candidate has a normal-audio, cache-unverified five-run diagnostic set before reboot and a six-run collection after reboot. All raw reports validated and all processes exited 0. The first post-reboot external process-create-to-first-frame-marker upper bound was 1,419.062 ms; the probe's `first_frame_us` was 1,049.583 ms after `main`. Audio started at 2,925.751 ms after `main` (1,876.168 ms after first display). This did not meet the documented environment protocol: the laptop was on battery/offline, other applications had been restored after login, and OS cache state is unverified. No deviation is approved. See the [post-reboot summary](supporting/pr69-post-reboot/launch-summary.json), [environment metadata](supporting/pr69-post-reboot/protocol-metadata.json), and [pre-reboot warm summary](supporting/pr69-warm-launch-summary.json).

## Criterion-by-criterion result

| Criterion | Actual result | Build identity | Evidence | Closure status |
|---|---|---|---|---|
| Cold startup <2,000 ms, normal audio | First PR #69 launch after reboot: external request-to-first-frame-marker upper bound **1,419.062 ms**; probe `first_frame_us` **1,049.583 ms** after `main`. Audio started at **2,925.751 ms** after `main` (1,876.168 ms after first display). Six process reports validated and exited 0. Machine was on battery (73%), other apps had been restored after login, and OS cache state is unverified; the documented plugged-in/other-apps-closed protocol was not met, and no deviation is approved. | Local source `841ee1dccac37236a4bdcf92ec3dbf17e7a18c80`; EXE SHA-256 above. | [Post-reboot summary](supporting/pr69-post-reboot/launch-summary.json), [protocol and environment metadata](supporting/pr69-post-reboot/protocol-metadata.json), [first-run raw probe](supporting/pr69-post-reboot/launch-1.json), [historical diagnostics](supporting/startup-historical-diagnostics.json), [pre-reboot warm diagnostics](supporting/pr69-warm-launch-summary.json), and [PR #69](https://github.com/dineshkumarc-1/software_engineering_mini_project/pull/69). | **Limited evidence; strict protocol disposition pending** |
| TC-CORE-01 visible grid | Existing ten-run DWM results (2,001.85–2,527.62 ms) were recorded while the desktop was locked and timed out without pixel change; they are not valid visible-grid observations. Title-transition data is also not an acceptable rendering endpoint. | Existing timing build not tied to PR #69; PR #69 binary hash not recorded. | [Preserved measurement JSON](supporting/tc-core01-locked-poll-measurements.json) and [correction](supporting/tc-core01-measurement-correction.md). | **Pending; no valid timing sample** |
| TC-PERF-02 input acknowledgement | Existing run contains 100 ordered probe records, 18 accepted / 82 rejected, 0 dropped. Its 1–6 μs interval begins at SFML dequeue and ends at engine registration. This excludes OS/SFML queue waiting and is only an internal lower bound. | Historical build; no PR #69 candidate hash. | [Preserved dequeue-only summary](supporting/tc-perf02-dequeue-only-summary.json). | **Pending; required endpoint not measured** |
| TC-PERF-01 sustained FPS | No three continuous qualifying runs of 300 seconds each in Hard/Obstacle with power-ups active have been demonstrated. Failed/short collision attempts remain preserved and are not combined. | Historical runs are not the PR #69 candidate; no accepted campaign build identity. | [Preserved historical report](supporting/tc-perf01-historical-report.md); original attempts remain in the local QA workspace history. | **Pending; 0/3** |
| TC-REL-01 corrupt config | Existing manual result records malformed config detected, `[WARN] SettingsRepository: E_PARSE: defaults loaded`, usable gameplay, a volume change saved, and process exit. The exit code is null and initial displayed default values were not recorded. | Exact source/build/date not recorded; not attributed to PR #69. | [Preserved manual result](supporting/corrupt-config-manual-result.json). | **Partial evidence; preserve as recorded** |
| TC-USE-01 first game | Dinesh's owner report states at least three outside-team first-time friends built from GitHub and reached gameplay without help via Main Menu → Name Entry → Mode/Difficulty → Gameplay. The route matches the STP's ≤3 screens and 3/3 outcome. Exact build/date are absent. | Build/date not recorded; not candidate-specific. | [Preserved owner report](supporting/owner-usability-report.json). Attribution remains owner-reported; no participant detail is invented. | **Reported criterion match, limited provenance** |
| PR #69 affected audio behavior | Source review confirms `AudioService::start()` follows the first `window.display()`. First post-reboot phase data shows audio starts after the first frame. No Windows audible check of music, food, Game Over, F5 SFX, SFX=0 silence, or pause/resume is recorded. | PR #69 source revisions above; local EXE SHA-256 above. | [First-run phase data](supporting/pr69-post-reboot/launch-1.json) and [PR #69](https://github.com/dineshkumarc-1/software_engineering_mini_project/pull/69), still Draft. | **Pending** |

## Concrete protocol submitted for approval

The strict TC-PERF-01 acceptance thresholds remain unchanged. The proposed
execution-method deviation is to use a deterministic closed-loop steering
driver, because open-loop/random steering repeatedly ends in collision before
the required duration. The controller must choose only legal directions from
the live game state; all ordinary wall, body, and obstacle collision rules
remain enabled. It must not pause, restart, use invulnerability, alter game
speed, suppress power-ups, or stitch sessions. This is a request for approval,
not an approved waiver or an acceptance result.

For each of three separate runs on the identified Windows target and selected
candidate:

1. Start a fresh Hard/Obstacle game with the performance probe enabled and
   normal collisions. Mark gameplay start after the visible grid and initial
   snake are confirmed; exclude menu/setup time from the 300-second interval.
2. Use the proposed closed-loop steering driver to keep the game in ordinary
   play. Record every external direction request and acceptance/rejection.
3. Preserve one uninterrupted 300-second gameplay interval, all non-overlapping
   one-second FPS windows (including zero-frame stalls), mode/difficulty/screen
   labels, power-up spawn/collection/activity evidence, and the final raw probe.
4. Reject a run if it dies, restarts, pauses, changes context, has missing or
   dropped samples, lacks power-up activity, or is shorter than 300 seconds.
   Start a new full run; never concatenate attempts.
5. Report each run's min/mean/median FPS and every window below 30 FPS. A single
   below-threshold window fails the unchanged TC-PERF-01 requirement.

**Approval requested from:** the project owner and the authority responsible
for the approved STP/test-plan baseline (QA owner). The exact approver names,
decision, date, and approved wording must be recorded before using this method.
Until that happens, do not run this alternate protocol or mark any run accepted.

The other blocked measurements need separate valid execution on an unlocked
desktop: (a) ten confirmation-to-visible-grid measurements using capture whose
image is verified to contain the game client area, initial grid and snake;
and (b) 100 randomized inputs with a timestamp at OS key-event delivery paired
to engine direction registration, retaining rejected inputs and reporting the
maximum interval. The existing title and dequeue-only timing must not be reused
as those endpoints.

## Reuse and next decision

Reuse the existing corrupt-config and owner-reported usability observations
with their stated limits. Reuse reviewed functional/settings evidence only for
unaffected behavior; retest startup and audio behavior changed by PR #69. Do not
repeat unrelated checks solely to create a new report.

**Blocker:** no approved candidate-specific cold launch, valid TC-CORE-01 or
TC-PERF-02 results, three qualifying TC-PERF-01 runs, or PR #69 audio regression
results are available. Keep #22 Open/In Progress until those rows pass or the
authorized authority records an explicit disposition. Do not change #26/#28.

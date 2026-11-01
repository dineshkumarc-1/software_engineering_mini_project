# Issue #22 owner-directed closure addendum — 2026-11-01

**Issue disposition:** #22 is closed by the issue owner's explicit acceptance
and administrative closure direction, with TC-PERF-01 explicitly **unchecked /
not run**. The owner stated that the 300-second workload could not be completed
and directed that it be skipped unchecked. This does not claim a performance
pass, a separate QA/test-plan waiver of NFR-001, full STP acceptance, Sprint 2
completion, or release sign-off. Keep the underlying requirement recorded as
unmet. See the [owner's GitHub closure comment](https://github.com/dineshkumarc-1/software_engineering_mini_project/issues/22#issuecomment-6555082843).

**Scope:** Issue #22 only. No demo was recorded. Issues #26 and #28 were not
changed. PR #69 remains Draft and was not merged.

## Candidate identity and completed checks

Latest local PR #69 Release candidate: source commit
`4b1d21420ff20a8e06751b675bd20d73e9a13764`; executable SHA-256
`47e43387ecc4d6407de2a29022e8b9884027fea8e3adb09c24d8a9aa8f25a915`;
1,915,392 bytes. Release build exit code 0; CTest 20/20; normal-audio backend
smoke 10/10. Antivirus was not disabled. The run logs and metadata are in
[latest candidate evidence](supporting/pr69-latest-4b1d214/).

The owner-reported manual audio observations are recorded in the
[listening record](audio-listening-observations.md), with separate build
identities for the earlier Settings checks and latest-candidate gameplay
checks. No audio-unavailable warning was visible during the earlier Settings
check. These are not an audio recording or independent-observer record.

## Criterion-by-criterion closure table

| Criterion | Actual result | Build identity | Evidence | Closure disposition |
|---|---|---|---|---|
| Issue #22: cold launch <2,000 ms, normal audio | One first post-reboot request-to-first-frame marker was 1,517.495 ms on an earlier candidate. The run was normal-audio and AC-online, but the final protocol metadata shows one visible non-shell application window; cache state was unverified. No reboot measurement was made on the latest `4b1d214` candidate. | Earlier source `841ee1dccac37236a4bdcf92ec3dbf17e7a18c80`; EXE SHA-256 `341bd6394a93741137866c88d95c3b00ae9ccb6d160e19d9044a0fddde68b57b`. | [Startup summary](supporting/pr69-latest-4b1d214/startup-limited/launch-summary.json), [protocol metadata](supporting/pr69-latest-4b1d214/startup-limited/protocol-metadata.json), [first launch raw report](supporting/pr69-latest-4b1d214/startup-limited/launch-1.json). | **Limited evidence; latest-candidate/documented-protocol acceptance not established.** The numerical sample is below 2,000 ms but is not represented as a strict acceptance pass. |
| Issue #22: input acknowledgement <100 ms | 100 injected OS keyboard events paired in order to 100 engine registrations; 50 accepted, 50 rejected, 0 drops/invalid samples; maximum OS-event-to-registration interval 21.131 ms. This measures injected events, not physical keyboard switch/HID latency. | Earlier source `841ee1dccac37236a4bdcf92ec3dbf17e7a18c80`; EXE SHA-256 `341bd6394a93741137866c88d95c3b00ae9ccb6d160e19d9044a0fddde68b57b`. | [Input protocol/result](supporting/pr69-latest-4b1d214/tc-perf02/protocol-and-result.json), [paired-event results](supporting/pr69-latest-4b1d214/tc-perf02/os-hook-input-result.json). | **Measured interval below 100 ms for the documented injected-event endpoint; candidate-specific scope is the earlier build.** |
| Issue #22: corrupt config falls back without crashing | Existing manual report recorded `E_PARSE: defaults loaded`, usable gameplay, and successful saving afterward. Exit code was not captured and exact executable/date are unknown. Automated settings/recovery coverage also passed in the reviewed test runs. | Manual executable identity unknown; do not attribute the manual run to `4b1d214`. | [Corrupt-config manual record](supporting/corrupt-config-manual-result.json); latest-candidate CTest log in [build evidence](supporting/pr69-latest-4b1d214/ctest-release.log). | **Fallback behavior supported; manual record remains supplemental and incomplete for process-exit/build provenance.** |
| TC-CORE-01: confirmation to visible grid | 10/10 visible-grid captures; median 269.167 ms, mean 281.078 ms, maximum 315.944 ms; each was <=2,000 ms. | Earlier source `841ee1dccac37236a4bdcf92ec3dbf17e7a18c80`; EXE SHA-256 `341bd6394a93741137866c88d95c3b00ae9ccb6d160e19d9044a0fddde68b57b`. | [10-run summary and captures](supporting/pr69-latest-4b1d214/tc-core01/summary.json). | **Passed on the earlier candidate; not a cold-start measurement.** |
| TC-PERF-01 / NFR-001: three continuous 300-second Hard/Obstacle sessions | **0/3 qualifying traces.** Ten short setup attempts were preserved; they do not meet duration or acceptance criteria. No runs were stitched, collisions were not disabled, and no pass is claimed. | Latest candidate `4b1d21420ff20a8e06751b675bd20d73e9a13764`; EXE SHA-256 `47e43387ecc4d6407de2a29022e8b9884027fea8e3adb09c24d8a9aa8f25a915`. | [Attempt protocols, raw probe reports, screenshots, and summaries](supporting/pr69-latest-4b1d214/tcperf01-setup-attempts/). | **Unchecked / not run. Owner-directed omission for #22 tracking closure only. NFR-001 remains unmet and is not waived or passed.** |
| TC-USE-01: first game within three primary screens | Dinesh's existing report describes at least three first-time friends reaching gameplay without help via Main Menu → Name Entry → Mode/Difficulty → Gameplay. Exact build/date and participant-level records are absent. | Build/date not recorded. | [Owner usability record](supporting/owner-usability-report.json). | **Partial owner-reported evidence; attribution and limitations retained.** |
| PR #69 audio regression on latest candidate | Owner reported latest-candidate Main Menu music, Game Over cue, food cue, and pause/resume behavior. Earlier-candidate Settings checks cover F5, independent volume controls and SFX=0. CTest and real audio backend smoke also passed on latest candidate. | Latest source `4b1d21420ff20a8e06751b675bd20d73e9a13764`; EXE SHA-256 `47e43387ecc4d6407de2a29022e8b9884027fea8e3adb09c24d8a9aa8f25a915`. Settings observations: earlier source `841ee1`, EXE SHA-256 `341bd6394a93741137866c88d95c3b00ae9ccb6d160e19d9044a0fddde68b57b`. | [Latest candidate build/audio logs](supporting/pr69-latest-4b1d214/); [owner listening record](audio-listening-observations.md). | **Owner-reported manual checks completed on the identified builds; not independently recorded. PR #69 remains Draft.** |

## TC-PERF-01 status and retained attempts

The approved execution method called for three independent uninterrupted
300-second Hard/Obstacle sessions with live-board closed-loop steering, normal
collision rules and power-ups enabled, and all one-second FPS windows/events
retained. Setup validation exposed controller issues before any attempt could
qualify. The raw setup attempts remain available in the linked evidence folder.
The owner has elected not to continue the workload and explicitly directed that
the criterion remain unchecked for closure. No threshold, method, result, or
acceptance label has been changed to manufacture compliance.

## Issue status versus project/test-plan status

#22 is closed according to the owner's explicit acceptance/administrative
direction, while TC-PERF-01 remains unmet in both the execution matrix and test
summary. Issue closure is not evidence that all STP cases passed. Overall STP
completion, remaining issues, release approval, and Sprint 2 completion are
outside this disposition and remain unasserted.

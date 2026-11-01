# Sprint2 Test Summary Report — evidence reconciliation

**Status: In Progress; final sign-off Pending.** Prepared as additive owner QA close-out support,not impersonating Ayushi or a Windows operator. No new game tests were executed for this document edit.

## Scope and build separation

| Build/evidence | Identity | Treatment |
|---|---|---|
| Baseline game |7c6733d95a4fa61a3af83c413a27d1ad2f40d44b/treee83825372214b6c11296c59d47301cf236a5f716 | Existing integrated Linux and reported Windows testing |
| Documentation/tooling main |6db9a7a2f1d8e1ad37e3f827e8a8ea18749629c6 | PR #58/#59merged; product source unchanged from baseline |
| Official Windows artifact | CI40994581558/artifact12950672120;EXE1,902,080bytes,SHA2568442dcb9a8ea16d347af70b8e4af9b739399633f1d9e053885964a2a170b14fd | Package identity,not a substituted local source-build hash |
| Received local Windows 10build |EXE1,906,688bytes,SHA256FE3AA1316D634611F3D690665C5D802696BE80DA5124C34B2475D05391CEF1F3 | Actual local build reported by Implementation owner; raw supporting package review incomplete |
| New audio enhancement | PR #60, `09bbf96e14d9b8f037d46b4b55e07d32aeacf26c` | Merged into main at `3fcc586`. Owner-reported listening confirms collision-alert audibility, one-shot/no-loop behavior, and silence at SFX volume 0. The original Windows ZIP is reported to support 20/20 CTest, 161 audio assertions across five cases, and seven backend checks; these archive results have not been independently re-inspected here. Corrected pause/restart records in the revised archive remain unverified. |

## Executed evidence inventory — do not sum overlapping counts

| Evidence source | Result | Limits |
|---|---|---|
| Owner Linux Release |21/21 CTest | Existing published executed logs |
| Owner Linux Clang ASan/UBSan/leaks |21/21 CTest,no reports | No Windows/native listening inference |
| Coverage build |21/21 CTest;C6-C11=98.12,96.70,97.27,96.00,97.30,91.84% | All components>=80%; not every individual file necessarily>=80% |
| Python |33/33 | Tool tests,not33STPcases |
| cppcheck / layering / bannedAPI |0 findings /35 SFML-free files /0 hits in 47 files | Unchanged gates; controls included |
| Linux runtime checks |navigation23,Settings22,leaderboard29,security23,enhancedleaderboard75,powerups21allpass | Includes overlapping assertions; null audio is not human hearing |
| Focused Linux regressions |D6:4 assertions;security732/21cases;fault214/8cases pass | LD_PRELOAD injection is Linux-only; cleanup limitation documented |
| Received Windows 10report |Releasebuildexit0;20/20 CTest;33/33 Python;focusedD6andD7model/runtime-title evidence | Independent review of referenced logs/client screenshot incomplete; no generic carryover of Linux assertion counts |
| Owner acceptance |the implementation owner personally played and reports everything working | Supports implementation acceptance,not specific measurements,volunteers,firewall enforcement or undocumented channel-listening results |

## Approved STP reconciliation

Source: submitted Part1PDF,STP§13/AppendixA. **44 unique cases,43 unique requirements,30 High,13 Medium,1 Low.** Inventory integrity and allocation checked in [validationJSON](evidence/stp-inventory-validation.json).

| Scoped status | Count |
|---|---|
| Automated scope passed |24 |
| Partial evidence |17 |
| Measured thresholds passed on earlier candidate; not re-executed on latest |2 |
| Owner-directed not run / unchecked for #22 closure |1 |
| Pending acceptance measurement |0 |
| Pending human walkthrough |0 |

These are evidence-reconciliation statuses,not a final approved44-case execution result. TC-USE-01 moved from Pending human walkthrough to Partial evidence based on Dinesh's saved owner report; exact build/date are absent and no Passed status is assigned. **Do not claim100% execution,100% Highpass or100% requirement-pass coverage.** [Per-case matrix](STP_EXECUTION_MATRIX.md) and [CSV](STP_EXECUTION_MATRIX.csv) identify exact remaining steps. Mapping completeness100% does not mean execution completeness100%.

## Defects and deviations

D1=date validation,D2=>10-entry rejection,D3=schema-corrupt recovery,D4date-onlyserialization,D5stale cleanup,D6emptywarning,D7six-columnUI. Integrated regression evidence supports their scoped fixes. Canonical cleanup policy remains24-hourbest-effort,not next-start guarantee. No confirmed failure in executed checks is not exhaustive zero-defects proof.

[Deviation matrix](REQUIREMENTS_DEVIATION_MATRIX.md) covers all43 requirements,C1-C15and baseline constants. It records actual differences,missing approval/rationale,and the Windows 11proposal separately from evidence gaps. Historical preliminary matrix is preserved in history. PR #60 is merged at `3fcc586`; its scoped evidence is recorded separately and does not silently expand baseline sign-off.

## Team close-out and release gates

Implementation issues 14–21 are Done following their recorded implementation, tests, and owner-acceptance evidence. Dinesh's saved Issue #22 report records at least three outside-team first-time friends, built from GitHub, reaching gameplay without help through Main Menu → Name Entry → Mode/Difficulty → Gameplay, matching the STP's three primary screens. Exact source/EXE and date, plus participant-level observations, are absent; TC-USE-01 is Partial evidence, not Passed or candidate-specific. Issue #22 is closed by owner direction with TC-PERF-01 unchecked; this does not establish full STP completion. #23 portability disposition, #24 exact-build privacy, #25 final report/STP acceptance, #26 UAT/tag/freeze, and #28 complete-product video remain open. PR #60 is merged; this does not grant final QA sign-off.

Final-signoff decision must check the approved exit criteria:100% planned execution,100% Highpass,0 open Critical/High,agreedMediumplans,43/43 requirements withpassing evidence,C6-C11>=80%,nosanitizer/cppcheck findings. Current evidence does not yet establish all criteria. No release authorization/signature is asserted.

## Immediate handoff

- TC-PERF-01 remains unexecuted by owner direction for #22 tracking closure; do not claim it passed. Complete the separate remaining STP/release gates and approved privacy capture. Independently verify the revised audio archive before treating its corrected records as inspected evidence.
- Tejas: review portability evidence and deviations without duplicating the same Windows 10campaign.
- Bharath: review/document exact-build privacy and persistence-cleanup disposition.
- Ayushi: review this44-case/43-requirement reconciliation,incorporate final raw evidence and actual human observations,and request final sign-off only when conditions are met.
- Pin the accepted final artifact, accept or reject explicit deviations, and complete UAT/release/demo decisions. PR #60 is already merged; merge status does not authorize a release tag or final QA sign-off. See [release close-out](SPRINT2_RELEASE_CLOSEOUT.md).

<!-- Evidence reconciliation: Windows performance and grid review -->

### Historical evidence reconciliation — pre-follow-up snapshot (superseded below)

**Overall status remains In Progress; final sign-off Pending.** The reviewed Windows performance evidence consists of short diagnostic segments and does not meet the required three continuous 300-second Hard/Obstacle sessions. The input registration metric is internal dequeue-to-registration timing, not OS/SFML queue-inclusive acceptance; launch results do not establish cold-launch acceptance. The reported grid timing results do not establish first-grid-render acceptance, and the reported TC-CORE-01 screenshot depicts a conversation rather than the game grid. The referenced grid artifacts were not present in this checkout for independent reinspection.

The separate Linux Release regression is additional evidence for the merged code, not a substitute for Windows-target acceptance. The fresh warnings-as-errors Release build succeeded; CTest passed 21/21, Python tests passed 33/33, audio tests reported 161 assertions across 5 cases, cppcheck 2.13.0 reported zero findings, and Xvfb UI reruns passed navigation (23 checks), Settings (22), and leaderboard (29). An initial parallel UI run had a missing Settings app window and a leaderboard X11 display-startup failure; original logs were preserved and unchanged sequential reruns passed, with no causal diagnosis established. Permission checks ran with effective capabilities dropped and gates unchanged.

The Linux run does not establish Windows performance/privacy/volunteer acceptance, fresh sanitizer or coverage results, or final release approval. Its redacted 19-file evidence ZIP was supplied in the owner conversation and is not committed repository evidence. Issues #22 and #25 and final QA sign-off remain pending. PR #60's merge does not authorize a release tag or final QA sign-off.

## Issue #22 pre-follow-up review snapshot — 2026-11-01 (superseded)

The local Windows Release candidate built from PR #69 head `841ee1dccac37236a4bdcf92ec3dbf17e7a18c80` has executable SHA-256 `341bd6394a93741137866c88d95c3b00ae9ccb6d160e19d9044a0fddde68b57b` (1,911,808 bytes). It is a local build, not a CI artifact. The first post-reboot normal-audio launch produced a 1,419.062 ms external process-create-to-first-frame-marker upper bound and a 1,049.583 ms internal `main`-to-first-display phase. Audio began 1,876.168 ms after first display. Six process reports validated and exited 0. The laptop was on battery/offline, other applications were restored after login, and OS cache state is unknown; this does not meet the documented plugged-in/other-apps-closed protocol, and no deviation is approved. Five pre-reboot diagnostics were 1,027.585, 621.219, 579.294, 612.492 and 590.747 ms with unverified cache condition. These results are recorded as limited evidence, not cold-launch acceptance. Full metadata and raw reports are in the [closure package](../sprint2/evidence/issue-22-closure-20261101/closure-report.md).

TC-CORE-01 remains Pending: no valid ten-run confirmation-to-visible-grid set exists; the locked-desktop poll timed out without pixels. TC-PERF-01 remains Pending at 0/3 qualifying continuous 300-second Hard/Obstacle runs with power-ups. TC-PERF-02 remains Pending: the historical 100-record Windows probe reports 18 accepted and 82 rejected, but timestamps begin at SFML dequeue and omit OS/SFML queue wait. TC-REL-01's automated scope remains passed; the supplemental owner manual record supports `E_PARSE: defaults loaded`, usable play and subsequent save, with exit code and initial defaults unrecorded. TC-USE-01 is Partial as above. PR #69 remains Draft; its Windows-audio regression checks have not been recorded. No waiver or final sign-off is approved.

See the [Issue #22 closure report and criterion table](../sprint2/evidence/issue-22-closure-20261101/closure-report.md) and its [supporting evidence](../sprint2/evidence/issue-22-closure-20261101/supporting/).

## Current Issue #22 owner-directed closure — 2026-11-01

Dinesh directed that TC-PERF-01 remain unchecked because the three 300-second
Hard/Obstacle traces could not be completed and requested closure of #22. The
issue is now closed following the owner's explicit acceptance/administrative
closure comment ([GitHub comment](https://github.com/dineshkumarc-1/software_engineering_mini_project/issues/22#issuecomment-6555082843)).
TC-PERF-01 remains **not run / not passed**; the closure does not claim a
separate QA/test-plan waiver. This tracking decision does not certify the
complete STP, overall release, or Sprint 2. The earlier section above is
retained as history and is superseded for current issue status.

The latest PR #69 local Release candidate is source
`4b1d21420ff20a8e06751b675bd20d73e9a13764`, EXE SHA-256
`47e43387ecc4d6407de2a29022e8b9884027fea8e3adb09c24d8a9aa8f25a915`; build
exit 0, CTest 20/20, normal-audio backend smoke 10/10. Owner-reported audio
listening checks are recorded in the [owner disposition addendum](../sprint2/evidence/issue-22-closure-20261101/owner-disposition-addendum.md).
The latest candidate has no reboot-protocol startup measurement. The earlier
1,517.495 ms marker is limited evidence because the documented environment
conditions were not fully met.

Current case snapshot: 24 automated-scope cases passed; two thresholds were
measured below their limits on earlier candidate `841ee1` and were not rerun on
`4b1d214` (TC-CORE-01 visible grid, TC-PERF-02 injected-event acknowledgement);
17 cases have partial evidence; TC-PERF-01 is owner-directed unchecked. These
statuses do not add to a final STP pass percentage. See the updated
[case matrix](STP_EXECUTION_MATRIX.md), [CSV matrix](STP_EXECUTION_MATRIX.csv),
and [criterion-by-criterion closure addendum](../sprint2/evidence/issue-22-closure-20261101/owner-disposition-addendum.md).

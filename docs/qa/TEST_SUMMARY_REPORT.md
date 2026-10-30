# Sprint2 Test Summary Report — evidence reconciliation

**Status: In Progress; final sign-off Pending.** Prepared as additive owner QA close-out support,not impersonating Ayushi or a Windows operator. No new game tests were executed for this document edit.

## Scope and build separation

| Build/evidence | Identity | Treatment |
|---|---|---|
| Baseline game |7c6733d95a4fa61a3af83c413a27d1ad2f40d44b/treee83825372214b6c11296c59d47301cf236a5f716 | Existing integrated Linux and reported Windows testing |
| Documentation/tooling main |6db9a7a2f1d8e1ad37e3f827e8a8ea18749629c6 | PR #58/#59merged; product source unchanged from baseline |
| Official Windows artifact | CI40994581558/artifact12950672120;EXE1,902,080bytes,SHA2568442dcb9a8ea16d347af70b8e4af9b739399633f1d9e053885964a2a170b14fd | Package identity,not a substituted local source-build hash |
| Received local Windows 10build |EXE1,906,688bytes,SHA256FE3AA1316D634611F3D690665C5D802696BE80DA5124C34B2475D05391CEF1F3 | Actual local build reported by Antigravity/Dinesh; raw supporting package review incomplete |
| New audio enhancement |PR #60,09bbf96e14d9b8f037d46b4b55e07d32aeacf26c | Separate unmerged product change; cannot inherit old binary listening results |

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
| Owner acceptance |Dinesh personally played and reports everything working | Supports implementation acceptance,not specific measurements,volunteers,firewall enforcement or undocumented channel-listening results |

## Approved STP reconciliation

Source: submitted Part1PDF,STP§13/AppendixA. **44 unique cases,43 unique requirements,30 High,13 Medium,1 Low.** Inventory integrity and allocation checked in [validationJSON](evidence/stp-inventory-validation.json).

| Scoped status | Count |
|---|---|
| Automated scope passed |24 |
| Partial evidence |16 |
| Pending acceptance measurement |3 |
| Pending human walkthrough |1 |

These are evidence-reconciliation statuses,not a final approved44-case execution result. **Do not claim100% execution,100% Highpass or100% requirement-pass coverage.** [Per-case matrix](STP_EXECUTION_MATRIX.md) and [CSV](STP_EXECUTION_MATRIX.csv) identify exact remaining steps. Mapping completeness100% does not mean execution completeness100%.

## Defects and deviations

D1=date validation,D2=>10-entry rejection,D3=schema-corrupt recovery,D4date-onlyserialization,D5stale cleanup,D6emptywarning,D7six-columnUI. Integrated regression evidence supports their scoped fixes. Canonical cleanup policy remains24-hourbest-effort,not next-start guarantee. No confirmed failure in executed checks is not exhaustive zero-defects proof.

[Deviation matrix](REQUIREMENTS_DEVIATION_MATRIX.md) covers all43 requirements,C1-C15and baseline constants. It records actual differences,missing approval/rationale,and the Windows 11proposal separately from evidence gaps. Historical preliminary matrix is preserved in history. PR #60new audio is not silently folded into baseline signoff.

## Team close-out and release gates

Implementation issues14-21 are Done following merged implementation/tests and owner acceptance (including18test execution). Remaining22performance/usability,23portability disposition,24exact-build privacy,25finalreport/STPacceptance,26UAT/tag/freeze and28video stayopen. PR #60is an additional In Progress enhancement pendingWindowslistening.

Final-signoff decision must check the approved exit criteria:100% planned execution,100% Highpass,0 open Critical/High,agreedMediumplans,43/43 requirements withpassing evidence,C6-C11>=80%,nosanitizer/cppcheck findings. Current evidence does not yet establish all criteria. No release authorization/signature is asserted.

## Immediate handoff

- Antigravity/Dinesh: complete native live checks,proper performance campaign and approved privacy capture; build/listen separately forPR #60.
- Tejas: review portability evidence and deviations without duplicating the same Windows 10campaign.
- Bharath: review/document exact-build privacy and persistence-cleanup disposition.
- Ayushi: review this44-case/43-requirement reconciliation,incorporate final raw evidence and actual human observations,and request final sign-off only when conditions are met.
- Dinesh: choose merge/deferPR #60,pin the accepted final artifact,accept or reject explicit deviations,thenUAT/release/demo. See [release close-out](SPRINT2_RELEASE_CLOSEOUT.md).

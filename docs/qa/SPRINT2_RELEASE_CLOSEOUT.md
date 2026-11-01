# Sprint2 release close-out checklist — not a release approval

## Candidate decision
- [x] Original implementation/evidence stack merged through PR59; baseline 7c6733d,docs/tooling main 6db9a7a.
- [x] Owner functional Windows 10acceptance recorded; implementation issues15/16/17/19/20/21closed with scoped rationale.
- [x] PR #60 audio enhancement is merged into main at `3fcc586` (feature commit `09bbf96`). Record owner-reported collision-audio listening and original Windows archive results with their evidence limitations. Preserve baseline `7c6733d` evidence and all historical run identities.
- [ ] Verify the accepted final candidate's actual CI results, executable and runtime package contents; preserve licenses, fonts and DLLs. Do not treat the PR #60 merge or owner-reported ZIP results as verification of the final release artifact.

## Evidence acceptance
- [ ] Review44-case CSV,all30 Highcases and43 requirements; distinguish scoped automated evidence from final case pass.
- [ ] Finish10 new-game render measurements,50 system ticks per difficulty and remaining explicit manual/native cases where required.
- [ ] Record actual independent-channel listening/F5(SFX0silent),visualtheme/table evidence,remaps and restart persistence. General owner acceptance is not invented individual observations.
- [ ] Execute three 300 s Hard/Obstacle FPS sessions and100 random-time input protocol; verify instrumentation timestamps/accounting and target hardware.
- [ ] Three outside-team first-time volunteers,without help; count <=3 primary screens,notclicks.
- [ ] Review exact-candidate privacy capture with explicit scoped firewall authorization,source/EXE provenance,actual offlinegameplay,redaction and sampling limits.
- [x] Historical Windows 11 evidence status for candidate `7c6733d` is documented for traceability. Later Windows 11 evidence for candidate `3fcc586` was accepted for Issue #23. This historical row does not reopen Issue #23 or constitute final QA sign-off or release approval.
- [ ] Review concrete deviations:cleanup guarantee,uncollected5s power-up lifetime,UI labels/interfaces,Loggerrotation and testseams. Record rationale and actual acceptance,not inferred signatures.
- [ ] FinalTest Summary Report and deviations reviewed by QA; residual defect severities and accepted risks recorded.

## UAT/tag/freeze (#26)
- [ ] the implementation owner/QA execute end-to-end all3modes,settings,leaderboard,restart/exit on selectedfinalbuild.
- [ ] Required exit criteria satisfied or explicit authorized changes documented. Owner release decision recorded.
- [ ] Tag `v1.0.0-final` on accepted commit,not an old or assumed latest SHA.
- [ ] Publish complete identified runtime package and evidence links; verify extracted package runs.
- [ ] Freeze development only after accepted release decision. No command in this checklist creates a tag.

## Video (#28)—last
- [ ] Record real2minute complete-product demo after finalintegration/acceptance. Suggested sequence:0:00title/build;0:05menu/nameentry;0:20Classic+pause;0:40Timed;0:55Obstacle;1:10powerups;1:30Top10persistence;1:45settings/save/relaunch;1:58exit.
- [ ] Ensure Speed/Multiplier/Shrink and required features are actually visible; adjust cuts honestly. Do not present fabricated gameplay or unobserved events as recordings.
- [ ] Upload `demo/sprint2.mp4` or documented README link; verify playback/access. Use actual captured footage,not a generated placeholder.

## Status discipline
Implementation Done is not blanket release certification. Issue #22 is owner-accepted closed; retain its outstanding measurement limitations. Issue #23 is Closed / Completed following explicit owner acceptance. Issue #24 remains Open pending Bharath's PR #76 corrections, green cppcheck and Windows MinGW CI, correction and verification of SHA256SUMS.txt, and final review. The existing reviewed corpus and privacy/firewall evidence should be referenced without repeating the firewall test. The native Windows/macOS crash-injection gap still requires explicit disposition; do not report it as passed. Issue #25 remains In Progress until its required sign-offs are completed. Issues #26 and #28 remain incomplete until their acceptance criteria are met. Final QA sign-off and release readiness remain Pending. No release tag or approval is implied.

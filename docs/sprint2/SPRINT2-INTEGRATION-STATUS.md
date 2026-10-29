# Sprint 2 owner integration candidate — NOT final acceptance

AI-assisted implementation/review for C Dinesh Kumar. This is an isolated
candidate, not a merge into main, QA sign-off, release freeze or final demo.
The original staged owner workspace and existing teammate branches are untouched.

## Inputs reviewed and combined locally

| PR | Tested input head |
| --- | --- |
| #42 validator | 517c169433b4139d047acaead806314dd524de03 |
| #44/#45/#46 owner UI/Settings/probes | 51fd38fa513a21f3272d3e58a42ae49e1f44cde3 |
| #48 persistence | 98a75488682a20663dce40fc3f4c12063fa60eb7 |
| #49 power-ups | 21168fd6480a1ca6b558b9d2117ea743e2ed1f94 |
| #50 fuzz/audit | 07d37d22941b7a2db97d68229fc17a230345b497 |
| #51 quality gates | 66ddaa4c5a63bf5c7ffc81335b7977777e7a5a76 |
| #52 security corpus | 8d6acd28cb0da1a708da402fd4a4ef4be0e7137c |
| #47 preliminary QA | 09181844f81ed5ac12dfeb9f79238ecb939211e0 |

The QA merge has a gameplay-test conflict: retained the deterministic food
placement from the validator branch rather than reintroducing flaky restart
scoring. QA document content is imported unchanged, not upgraded to final sign-off.
Local merge history is retained locally; the browser-published candidate is a
content integration based on #46. It is NOT a replacement for reviewing/merging
original PRs in dependency order. Reconcile the candidate with their final heads
before any main merge; do not merge overlapping work twice without review.

## Owner integration implemented

- **#20:** SettingsRepository now delegates bounded JSON reads and atomic writes
  to C13 SecureFileStore, with a Settings nesting limit of **8**. Settings schema,
  key/volume/theme validation, stable fallback-warning codes, missing-parent
  failure, invalid-save target preservation, and the existing Save/Cancel UI
  policy are retained. Nested duplicate JSON keys are rejected by the shared
  parser too. No competing file implementation remains in SettingsRepository.
- **#21:** real Top-10 table with Rank / Player Name / Score, empty/corrupt state,
  visible warning and keyboard/mouse Back navigation. Completed games submit
  through C11 exactly once, with active mode/difficulty and normalized player
  name; repeated frames and refresh do not submit. The table reloads from disk
  on entry and survives process restart. Failures are visible, not silently
  reported as saved. New SFML-free LeaderboardModel regression tests exercise
  that application adapter. `top10` has an optional warning-output argument;
  existing no-argument callers remain source-compatible.
- Integration-only cppcheck findings corrected with behavior-preserving STL
  algorithms and diagnostic local-variable renaming. No sanitizer/static-analysis
  gate is weakened. The Linux CI desktop step also runs the leaderboard smoke.

## Executed verification (NOT exhaustive acceptance)

Amazon Linux 2023 x64, GCC 11.5, Clang 15.0.7, CMake 4.4.4, cppcheck **2.13.0**
(built to match the PR #51 baseline), Python 3.13, SFML 2.5.1, Catch2 3.8.1,
JSON 3.11.3. Local dependency caches; desktop app built with warnings as errors
and BUILD_TESTING=OFF. Separate headless/sanitized test builds supply the results.

| Executed check | Observed result |
| --- | --- |
| Combined headless CTest, unprivileged user | 20/20 suites pass, including corpus and banned-API tests |
| Clang ASan + UBSan, leak detection, unprivileged user | 20/20 suites pass, no sanitizer reports |
| cppcheck 2.13, unchanged project gate | 0 findings |
| Python tooling unit suite | 33 pass |
| Real Xvfb leaderboard integration | 12 checks pass, including 10 rows, one-shot save, restart, corruption and Back |
| Real Xvfb navigation | 23 checks pass |
| Real Xvfb Settings | 22 checks pass, including corrupt-config fallback and failed Save/Cancel |
| Real Xvfb security UI | 23/23 pass; includes the newly written leaderboard file privacy check |
| Probe tooling smoke | 100 synthetic requests + 3 validated launches pass; acceptance remains not_assessed |

**Critical qualification:** the corpus suite contains **five tests tagged
`[!shouldfail]`** for D1–D5. All five still fail as expected. Therefore “20/20
suites” and green CI are NOT evidence those defects are fixed or that #18/#17
acceptance passes. The two known malformed leaderboard fixtures remain exceptions
in the corpus. Read the retained #52 report and tests, not just CTest's exit code.
Permission/fault tests were run as `nobody`; the default sandbox's elevated
filesystem capabilities otherwise bypass directory restrictions. No such failure
was hidden by altering an assertion. Logs: `evidence/integration/`.

The first desktop-navigation attempt overlapped local app build/relink work and
could not find a window; a stable post-build rerun passed 23 checks. Initial
local cppcheck 2.7.4 (environment package) was not treated as the CI baseline;
the reported clean result is the later 2.13.0 run. New published-head GitHub CI
must be checked separately. Local source hashes identify the tested file inputs.

## Open blockers and responsible follow-up

| Work | Status / action |
| --- | --- |
| #15/#17/#18 — D1 | Bharath: reject invalid/non-YYYY-MM-DD date values; add calendar-validity coverage |
| #15/#17/#18 — D2 | Bharath: reject an on-disk board with >10 entries, not silently truncate it |
| #15/#17/#18 — D3 | Bharath: next valid save must atomically replace schema-invalid data, retaining warning; never overwrite unsafe/I/O/oversize targets blindly |
| #15/#17/#18 — D4 | Bharath: persist date_achieved as YYYY-MM-DD rather than a timestamp |
| #15/#17/#18 — D5 | Bharath/Tejas: safe stale-temp cleanup policy and deterministic fault-injection coverage; do not delete arbitrary or another active writer's files |
| #18 test maintenance | After each fix, remove only its `[!shouldfail]` tag, remove relevant corpus exception, and rerun normal regression; no silent acceptance |
| #24 | UDP verification and enforced offline/firewall evidence remain incomplete in #50; source search and sampled TCP absence are not exhaustive proof |
| #23 | Real Windows 10 AND Windows 11 clean build/run checklist remains pending; CI Windows Server is not a substitute |
| #22 | Target-device <2s cold-launch and full end-to-end <100ms input acknowledgement remain unverified; internal lower-bound/Xvfb probes are not acceptance |
| #25 | Ayushi: reconcile tests with actual integrated SHA/artifact, finish old-primary-key remapping check and traceability/manual evidence; final QA sign-off pending |
| #26 | Joint UAT, final tag and development freeze wait until defects and release exit criteria are satisfied |
| #28 | Final two-minute video is LAST, after all Sprint-2 PRs are merged and the executable is ready. Not started prematurely |

**Merge ordering still applies:** #49 before #51; #42 before persistence; #50
before #52; owner #44 -> #45 -> #46. Recheck conflicts and CI at each final
integration head. Do not close the sprint, tag `v1.0.0-final`, claim 100% tests,
freeze development or record the final demo from this candidate.

## First published-head CI follow-up

On candidate `6f753d67184e897440bb1db65877ecec511010d4`, Linux app configure,
build, CTest, audio, navigation and Settings checks passed. The new leaderboard
smoke then reached its second normal exit and xdotool reported BadWindow: the
Escape key-down closed the app before xdotool could send key-up. This failed CI
run is retained, not counted as green:
https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/40841815410

The smoke now tolerates that command error only at the intentional app-exit
step, then still requires the actual child to exit within 10 seconds with
status **zero**. An app hang, crash or nonzero exit cannot pass. No production
behavior or test expectations changed. Three consecutive local leaderboard
runs pass **12 checks each** after the correction, and all **33 Python tests**
still pass. See `evidence/integration/leaderboard-ui-repeat.txt`. Fresh-head
CI must be checked separately before claiming CI success; D1–D5 and manual
acceptance blockers remain unchanged.

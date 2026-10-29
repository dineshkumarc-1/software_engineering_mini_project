# Sprint 2 owner integration candidate — NOT final acceptance

AI-assisted implementation/review for C Dinesh Kumar. This is a content-integration
candidate for Draft PR #53, not a merge into main, QA sign-off, release freeze,
or final demo. The original staged workspace and teammate branches are untouched.

## Refreshed inputs

| Input | Exact revision |
| --- | --- |
| Main, including merged #49, #51 and preliminary #47 | d9bd416dbeb6ae6976168127c3b376330dcdde2d |
| Published #53 before this follow-up | dbf08b51b9465e7ceac66e5200b8bcad874db408 |
| #48 persistence fixes | 2402449795c862b309383eb7edff6e5d7f8c2649 |
| #50 combined store tests / privacy documentation | 6e281183a8d8a022c80a9f1f1dced2383a0a0bfc |
| #52 promoted corpus / fault tests / UI tooling | 810769cc3fde14245c600474ed2b15b76445c06e |

Latest persistence sources, combined SecureFileStore tests, promoted corpus,
fault-injection tests, teammate evidence and current main QA documentation were
imported without editing their contents. CMake combines the new fault target
with the existing owner targets. Historical teammate observations are not new
owner executions. The source hash manifest identifies this tested content.

This branch is still based on #46, not main. Reconcile original PR histories and
final heads before any main merge; do not merge overlapping work twice blindly.
The deterministic gameplay restart test and owner static-analysis corrections
remain. The integration does not weaken any sanitizer or static-analysis gate.

## Dinesh's implementation

| Issue | Implemented / checked | Remaining acceptance |
| --- | --- | --- |
| #19 | Navigation, name entry, nine mode/difficulty choices, keyboard/mouse transitions; real-window regression | Final candidate Windows/manual QA |
| #20 | SettingsRepository uses SecureFileStore; depth limit 8, existing schema/validation, warning codes, Save/Cancel and failure preservation retained | Final Windows audio/persistence/remapping evidence |
| #21 | D7 fixed: Rank, Player name, Score, Mode, Difficulty, Date; glyph-width-aware fitting, all ten rows and Back. One-shot game-over submission, actual mode/difficulty, ISO date and reload on entry retained | D6 warning defect and final candidate QA |
| #22 | Instrumentation wiring and strict 100-input smoke; accepted/rejected/consumed/displayed remain distinct, no dropped samples or name leakage | Target-device cold launch, FPS and true end-to-end input latency; smoke is NOT acceptance |
| #26 | Not started prematurely | Joint UAT, release exit criteria, tag/freeze |
| #28 | Not started | LAST: only after integration, executable and acceptance are ready |

The leaderboard formatter is SFML-free and preserves the existing QA title
protocol (`| rank. name score | mode / difficulty / date`). Three new ordinary
regressions cover all nine combinations, a 15-wide-W name, maximum integer score,
a valid leap-day date, nonoverlapping column geometry and the parser delimiter.
The six-column screenshot is from the final tested desktop binary.

The probe driver previously issued a key across a Hard/Obstacle Game Over
transition (99 of 100 recorded, zero dropped). For this wiring-only smoke it now
uses Classic/Easy and a bounded seeded burst, requires the game remain Playing,
and requires ALL 100 records in exact order. This is not a substitute for the
separate Hard/Obstacle/reference-hardware performance campaign.

## Executed verification

Amazon Linux 2023 x64, GCC 11.5, Clang 15.0.7, cppcheck 2.13.0, SFML 2.5.1,
Python 3.13. Desktop uses software GL and null OpenAL: no audible acceptance.
Headless and sanitizer tests run unprivileged so permission tests are meaningful.

| Executed check | Result |
| --- | --- |
| Headless CTest | 21/21 suites pass |
| Clang ASan + UBSan with leak detection | 21/21 suites pass, no sanitizer reports |
| Leaderboard model/format regressions | 5 cases, 106 assertions pass |
| Leaderboard model + gameplay repeats | Each passes 20 consecutive executions |
| cppcheck 2.13, unchanged gate | Zero findings |
| Python tooling unit suite | 33 pass |
| Final six-column real-window smoke | 17 checks pass; all ten rows, all nine combinations, max-width boundary, one-shot save, restart, corrupt file and Back |
| Final real-window navigation | 23 checks pass |
| Final real-window Settings | 22 checks pass |
| Final security UI | 23/23 checks pass |
| Unchanged Tejas TC-LB-02 tool | 8/8 checks pass; final screenshots reviewed |
| Strict probe wiring, three consecutive repeats | Each records all 100 requests in order plus 3 valid launch traces; acceptance NOT ASSESSED |

**Critical qualification:** one D6 test still has `[!shouldfail]`. Empty and
whitespace-only on-disk leaderboard files return no warning; its two warning
assertions still fail as expected. The latest #52 D1–D5/D7 tag promotions are
imported unchanged; they were not reverted. Green CTest is NOT final acceptance.
D6 is tracked in issue #55 and must remain visible.

Evidence: `evidence/owner-followup/`. Earlier `evidence/integration/` files and
teammate run reports are historical. Fresh published-head GitHub CI must be
checked separately; local success is not a claim that hosted CI passed.

### Harness failures retained, not hidden

- Earlier leaderboard smoke startup saw no window. The owner harness now waits
  for an actual X display query, rather than socket creation alone, and reports
  an early child exit with its runtime log. Final rerun passes; assertions remain.
- Independent teammate tooling required xdpyinfo and xwininfo in this environment.
  Its parser exposed the initial owner title delimiter incompatibility, fixed
  in product formatting with an ordinary regression, not by editing their tool.
- Initial probe smoke recorded 99/100 as described above. Failure logs are kept;
  no missing-input assertion or sample requirement was relaxed.
- An intermediate batch used an incorrect security-script filename and stopped.
  The correct unchanged `security_ui_smoke.py` is used for the actual result.

## Open acceptance / coordination

| Work | Required follow-up |
| --- | --- |
| D6 / #55 | Bharath persistence warning fix; coordinate Tejas's regression promotion only after confirmed correction |
| #24 | Latest imported report records reviewed Windows TCP/UDP samples and enabled executable-scoped firewall at start/end. This is sampled evidence, not exhaustive zero-network proof; retain timing/build limitations and revalidate final build as required |
| #23 | Real Windows 10 AND Windows 11 clean build/run checklist; CI Windows Server is not a substitute |
| #22 | Reference-device cold start <2s and true input-to-display <100ms, FPS and specified campaign; Xvfb/internal lower bounds do not establish these |
| #25 | Ayushi: exact final candidate SHA/run/artifact, pending remapping/manual tests, traceability, audible/visual checks and final QA sign-off |
| #26 | Joint UAT and volunteer evidence, final release exit criteria, tag/freeze only after blockers clear |
| #28 | Final two-minute demo LAST, after all required merges and executable readiness |

No final release tag, sprint closure, development freeze or final QA sign-off is
claimed by this follow-up. Keep PR #53 Draft and unmerged while blockers remain.

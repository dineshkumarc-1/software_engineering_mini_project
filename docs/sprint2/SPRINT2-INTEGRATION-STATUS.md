# Sprint 2 owner reconciliation — code integration, NOT final acceptance

AI-assisted implementation/review for C Dinesh Kumar. This candidate reconciles
PR histories and retains #53's tested fixes. Integration does not mean final QA,
release freeze/tag, sprint closure or demo approval. The user's original staged
workspace is untouched. Owner-requested conflict fixes were published to #42/#44;
Bharath/Tejas implementation branches were not rewritten or force-pushed.

## Reviewed inputs and history

| Input | Exact revision / result |
| --- | --- |
| #42 validator conflict resolution | 4790254ff22aa43f1740b3be6ff2158880ff1e40; 12/12 local CTest, zero cppcheck, 8/8 CI; merged into main |
| #44 UI conflict resolution | c0827f476cb8eef5bd5d7a434ff566f959e12096; 12/12 local CTest, zero cppcheck, 8/8 CI; merged into main |
| Main after #42/#44, already including #49/#51/preliminary #47 | 374ab7482e082f07380c21510d5de520c6575f22 |
| Tested owner #53 baseline | 8432828849e281caf649ca884a1aa6577bfb5492 |
| Latest #48 persistence | 4793a7ad79937ea6598ffa84639594cae33afdff; includes f898f8c D6 warning fix |
| Latest #50 / merged #52 history | 4c0d1e0bba6327a123698181ba5f09ec5dd62164 |
| Owner reconciliation | #57 imports #48 into isolated integration/sprint2-reconciled; #56 targets #53, then #53 targets main |

Conflict resolutions preserve the owner's fixed name buffer and stress tests,
Settings storage depth 8 and validation, deterministic restart, combined store
fuzz/cleanup tests, canonical D1–D7 assertions/fixtures and fault-injection target.
A duplicate power-up drawing block introduced by a clean automatic main merge is
removed: the tested renderer draws each active power-up once. No sanitizer/static
or assertion gate was weakened. Published heads and hosted checks must be verified
after each history merge; a clean merge alone is not acceptance.

## Dinesh's work

| Issue | Implemented and locally verified | Remaining acceptance |
| --- | --- | --- |
| #19 | Navigation/name entry, nine choices, keyboard/mouse transitions, bounded input/stress handling | Exact final Windows candidate manual QA |
| #20 | SecureFileStore integration; Settings schema/validation, depth 8, warning codes, Save/Cancel and failed-save preservation | Final Windows audible/audio, persistence and key-remapping evidence |
| #21 | Six columns; 10 rows, wide-name/integer boundaries, one-shot save, mode/difficulty/date; D6 existing blank/whitespace warning without rewrite | Final candidate Windows/manual QA |
| #22 | Instrumentation, strict request accounting, Linux smoke measurements and raw evidence | Reference Windows campaign and volunteer usability; NOT accepted from Linux smoke |
| #26 | Release checklist/preparation only | Joint UAT, traceability/coverage/exit criteria, final tag/freeze |
| #28 | Not started prematurely | LAST, after integration, executable and acceptance |

## Actual verification

Linux x64, GCC 11.5, Clang 15.0.7, cppcheck 2.13, SFML 2.5.1, Python 3.13.
Desktop uses Xvfb/software GL and null audio. Permission-sensitive headless and
sanitizer tests run unprivileged. These results are not Windows/audio sign-off.

| Executed check | Result |
| --- | --- |
| Headless CTest | 21/21 suites pass |
| Clang ASan + UBSan + leaks | 21/21 suites pass, no sanitizer reports |
| Latest unchanged cppcheck 2.13 gate | Zero findings |
| D6 ordinary regression | All 4 assertions pass, no [!shouldfail] tag |
| Python tooling unit suite | 33 pass |
| Real-window leaderboard, including D6 and false-warning controls | 29 checks pass |
| Real-window navigation | 23 checks pass |
| Real-window Settings | 22 checks pass |
| Security UI | 23/23 checks pass |
| Unchanged Tejas TC-LB-02 tool | 8/8 checks pass |
| Probe wiring, three runs | All 100 requests recorded in order each; 9 launch traces |

D6 is no longer hidden as an expected failure. The integration-only promotion
removes only known-defect/expected-failure metadata and preserves all assertions
and fixtures. Existing missing-file and valid-empty-board reads remain warning-free.
No C++ testcase retains [!shouldfail] in this candidate.

Evidence: evidence/final-integration/, including source SHA-256 manifest, actual
path-sanitized logs, screenshots, whitelisted aggregate probes and LINUX-MEASUREMENTS.md.
Raw process/runtime reports remain private in the local build workspace. Older owner evidence and
teammate execution reports remain historical; they are not silently relabelled.
The earlier probe 99/100 failure, X display/tool setup issues and incorrect batch
preset/script attempts remain historical harness failures, not passing executions.

## Exact candidate handoff and open work

Pin one published SHA plus its matching Actions run and Windows artifact after
final hosted checks pass. Record EXE SHA-256 and carry DLL/assets/notices with it.
Do not use a previous artifact while claiming the new candidate was tested.

| Owner | Remaining exact-candidate work |
| --- | --- |
| Dinesh / #22 | Target hardware/cold launch; three continuous 300s Hard/Obstacle runs with power-ups and every 1s FPS window; 100 randomized inputs with valid acknowledgement timing; coordinate 3 outside-team first-time volunteers |
| Bharath / #24 | Repeat TCP/UDP and enforced executable-scoped firewall/offline checks on the pinned EXE; record source SHA, compiler, binary size/hash, rule state and limitations. The older capture lacks runtime build identity and cannot be attributed to this candidate |
| Tejas / #18/#23 | Confirm final corpus/defect disposition and acceptance mapping, coverage evidence; collect real Windows 10 AND 11 build/run checklist evidence. CI Windows Server is not client-OS acceptance |
| Ayushi / #25 | Preserve earlier results; test pinned artifact, pending remapping/audio/manual cases; record expected/actual/date/evidence; final QA sign-off only after required results exist |
| Owner + team / #26 | Confirm all required test/coverage/traceability/UAT and severity gates, then tag/freeze |
| Dinesh / #28 | Final demo LAST |

The #22 Linux data is diagnostic: 5 accepted/95 rejected requests per seeded
Classic/Easy run, short sessions, no verified target hardware/cold cache. Internal
registration excludes OS/SFML queue delay. Display-return timing is informational,
not photons and not a substitute for acknowledgement acceptance. Keep all Pending
requirements visible; no final QA, release tag, sprint completion or demo is claimed.

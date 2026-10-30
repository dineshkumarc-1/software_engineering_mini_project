# STP execution and traceability matrix — reconciliation snapshot

**Status:** documentation reconciled; final QA sign-off Pending. This is not a new test execution. Baseline game 7c6733d; docs/tooling main 6db9a7a; PR #60 audio 09bbf96 is separate unmerged product code and is not covered by the baseline Windows EXE.

## Authoritative baseline

`Team16_SnakeXtreme_Deliverables_Part1_SRS_SAD_STP.pdf`, STP section13 and AppendixA (printed pages43-50): **44 cases,43 requirements,30 High-priority cases**. Case titles/priorities/requirement allocation below follow that inventory. PWR05 is effect replacement, not simply pause. NFR005 is modularity; NFR006 is portability. Three-screen usability is not a three-click rule.

The submitted document itself has Draft approval fields; this reconciliation does not invent approval signatures or change its requirements.

## Status meaning and scope

- **Automated scope passed:** reviewed source tests and published executed logs support the defined automated portion on the stated baseline. It is not a new Windows execution or final signed STP pass.
- **Partial evidence:** some valid evidence exists; specific prescribed method/observations or a deviation decision remain.
- **Pending acceptance measurement / human walkthrough:** diagnostic tools or general owner acceptance cannot establish the specified result.

No column marks Pending as Passed. A mapped requirement is not a passing requirement. Do not sum CTest entries,assertions and UI checks into unique STP-case counts. Native scope and general owner acceptance are described separately.

Evidence anchors: [owner Linux report](OWNER-AUTOMATED-QA-7c6733d.md), [Tejas case execution](TC-PWR-LB-SEC-EXECUTION.md), [quality gates](../quality-gates.md), [Windows received-report register](WINDOWS10-EVIDENCE-REGISTER.md). Machine-readable matrix: [CSV](STP_EXECUTION_MATRIX.csv).

## Inventory

| Case | Priority | Requirement(s) | Title | Scoped execution status | Remaining action |
|---|---|---|---|---|---|
| TC-CORE-01 | H | FR-001 | Grid and snake render on new game | Pending acceptance measurement | Record approved 10 new-game first-grid measurements <=2000ms; do not substitute 3 warm process launches. |
| TC-CORE-02 | H | FR-002 | Tick interval per difficulty | Partial evidence | Attach 50 consecutive system tick timestamps per difficulty and +/-10% calculation. |
| TC-CORE-03 | H | FR-003 | Direction via arrows and WASD | Partial evidence | Reconcile all eight physical key/direction combinations at system level. |
| TC-CORE-04 | H | FR-003 | 180-degree reversal and queued turns | Automated scope passed | None for defined automated scope; preserve candidate identity. |
| TC-CORE-05 | H | FR-004 | Wall collision at all four boundaries | Automated scope passed | None for defined automated scope. |
| TC-CORE-06 | H | FR-005 | Self collision and vacating-tail rule | Automated scope passed | None for defined automated scope. |
| TC-CORE-07 | H | FR-006 | Food on free cell and full board | Automated scope passed | None for defined automated scope. |
| TC-CORE-08 | H | FR-007 | Growth by exactly one | Automated scope passed | None for defined automated scope. |
| TC-MODE-01 | H | FR-008 | Mode selection | Partial evidence | Specific native manual record of HUD/timer/obstacle distinctions remains to reconcile. |
| TC-MODE-02 | H | FR-009;FR-002 | Difficulty sets speed in all nine combinations | Automated scope passed | None for defined automated scope. |
| TC-MODE-03 | H | FR-010 | Timed Challenge ends at zero | Partial evidence | Reconcile system HUD/final-score observations at time-up; unit timing passed. |
| TC-MODE-04 | H | FR-011 | Obstacle placement and fatal collision | Automated scope passed | None for defined automated scope. |
| TC-PWR-01 | M | FR-012 | Power-up spawn interval | Automated scope passed | None for defined automated scope; separate uncollected-item policy deviation recorded. |
| TC-PWR-02 | M | FR-013 | Speed boost lasts five seconds | Automated scope passed | None for defined automated scope. |
| TC-PWR-03 | M | FR-014 | Multiplier lasts ten seconds | Automated scope passed | None for defined automated scope. |
| TC-PWR-04 | M | FR-015 | Shrink and minimum length | Automated scope passed | None for defined automated scope. |
| TC-PWR-05 | M | FR-013;FR-014;FR-015 | Only one active effect and replacement | Automated scope passed | None for defined automated scope. This case is replacement, not merely pause. |
| TC-SCORE-01 | H | FR-016 | Mode-specific score per food | Automated scope passed | None for defined automated scope. |
| TC-SCORE-02 | H | FR-017 | Real-time score HUD in every theme | Partial evidence | Record five foods and same-frame visible HUD correctness/legibility in each theme. |
| TC-LB-01 | H | FR-018 | Persist Top10 with every field | Automated scope passed | None for defined automated scope. |
| TC-LB-02 | H | FR-019 | View ten-entry and empty leaderboard | Partial evidence | Windows actual client-area screenshot and exact ten-entry/empty-fixture manual reconciliation; title mirror alone is not visual proof. |
| TC-LB-03 | H | FR-020 | Insert only within Top10 | Automated scope passed | None for defined automated scope. |
| TC-LB-04 | M | FR-021;NFR-003 | Missing/corrupt leaderboard fallback | Automated scope passed | None for defined automated scope; native report supporting logs remain separate review input. |
| TC-UI-01 | H | FR-022 | Main menu options and navigation | Partial evidence | Document label difference Start Game versus baseline New Game; exact native keyboard/mouse/Exit record. |
| TC-UI-02 | H | FR-023;SEC-006 | Name entry boundaries | Automated scope passed | None for defined automated scope; no invented native per-case observation. |
| TC-UI-03 | H | FR-024 | Pause/resume with active timer/effect | Partial evidence | Reconcile exact Timed+active-power-up five-second pause and unchanged state observation. |
| TC-UI-04 | H | FR-025 | Game Over restart and Main Menu | Partial evidence | Native final-score/restart controls record remains to reconcile; PR #60 audio is separate new code. |
| TC-SET-01 | M | FR-026 | Remap all movement keys | Partial evidence | Native four-direction I/K/J/L gameplay and old-primary results require per-case evidence; general acceptance not invented detail. |
| TC-SET-02 | M | FR-027 | Independent music and SFX volume | Partial evidence | Human listening for both channel combinations and range behavior; earlier only background sound was explicitly reported. F5 preview at SFX0 must be silent. |
| TC-SET-03 | L | FR-028 | Classic Neon Retro visual themes | Partial evidence | Record snake/grid appearance in gameplay for all three themes; no Dark theme. |
| TC-SET-04 | H | FR-029 | Settings persist across restart | Automated scope passed | None for defined automated scope; native report can add independent results. |
| TC-PERF-01 | H | NFR-001 | Sustained minimum30FPS | Pending acceptance measurement | Three continuous300sHard/Obstacle sessions with power-ups on reference hardware; no one-second window below30FPS, report min/avg/median. |
| TC-PERF-02 | H | NFR-002 | Input acknowledgement within100ms | Pending acceptance measurement | Approved100 random-time keypress campaign with full requested/accepted/rejected/registered accounting; verify timestamp origin and queue-delay scope. Visible turn is informational, not acknowledgement threshold. |
| TC-REL-01 | H | NFR-003;SEC-003 | Missing/corrupt config fallback | Automated scope passed | None for defined automated scope. |
| TC-USE-01 | M | NFR-004 | First game within three screens | Pending human walkthrough | Three outside-team first-time volunteers without help, count primary screens. Not three total clicks/keystrokes and not an AI usability result. |
| TC-MNT-01 | M | NFR-005 | Module separation | Automated scope passed | None for defined automated scope; specific SAD interface differences remain documented separately. |
| TC-PORT-01 | M | NFR-006 | Windows 10 and11 clean build/run | Partial evidence | Windows 11 Not tested; proposed omission awaiting explicit disposition. Preserve Windows 10 report and supporting raw logs; no duplicated teammate campaign needed. |
| TC-TEST-01 | M | NFR-007 | Headless testability | Automated scope passed | None for defined automated scope. |
| TC-SEC-01 | H | SEC-001 | Name sanitization | Automated scope passed | None for defined automated scope. |
| TC-SEC-02 | M | SEC-002 | No PII stored | Partial evidence | Reconcile final Windows saved files/log and redacted inspection evidence; network/firewall audit is additional Issue #24 scope, not the entire definition of SEC002. |
| TC-SEC-03 | H | SEC-003;SEC-007 | Schema and size validation | Automated scope passed | None for defined automated scope; preserve actual platform-specific assertion counts. |
| TC-SEC-04 | H | SEC-004 | Safe write under injected faults | Partial evidence | 24-hour best-effort cleanup differs from unconditional startup cleanup; disposition required. Linux crash injection is not newly executed native Windows/macOS injection. |
| TC-SEC-05 | H | SEC-005 | Untrusted data never executable | Automated scope passed | None for defined automated scope. |
| TC-SEC-06 | H | SEC-006 | 10,000-event and validator buffer limit | Partial evidence | Reconcile complete event/string/memory-stability scope against approved case; no native Win10 sanitizer execution claimed. |

## Reconciliation totals

- Automated scope passed: **24**.
- Partial evidence: **16**.
- Pending acceptance measurement: **3**.
- Pending human walkthrough: **1**.

Inventory completeness:44/44 unique cases,43/43 unique requirement references,30 High,13 Medium,1 Low. **Final STP pass count and requirement pass percentage are NOT certified**: some cases are partial/pending and evidence/deviations need acceptance.

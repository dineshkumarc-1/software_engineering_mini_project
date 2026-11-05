# Test case update — Part 3 sprint changes

**Tested source:** `04d782a345464349da10c91d6796b3ed99fc7b96` (tree `e7f5e8ef5f943d067375af0b375420bf81b421f3`), clean detached checkout of main.  
**Run:** 5 November 2026 UTC, automated Linux headless Debug coverage build; not a Windows or human acceptance test.  
**Status:** In progress; no final QA sign-off.

The submitted [Part 1 STP](../../Team16_SnakeXtreme_Deliverables_Part1_SRS_SAD_STP.pdf) Appendix A remains the baseline: 44 cases and 43 requirements. The [case-by-case execution matrix](../qa/STP_EXECUTION_MATRIX.md) and [CSV](../qa/STP_EXECUTION_MATRIX.csv) are the current reconciled test-case document and list case IDs, priority, evidence and remaining steps. This addendum updates the execution focus for merged sprint changes without silently changing requirements, inventing approvals or changing partial cases to passed.

| Sprint change | Updated test focus / case mapping | Status and limits |
|---|---|---|
| Integrated modes, difficulty and power-ups | TC-MODE-01–04, TC-PWR-01–05: check all mode/difficulty combinations, timer and obstacle behavior, spawn, each effect and replacement. | Fresh headless suite passed; native HUD/time-up observations remain as listed in the matrix. |
| Menus, names, settings, audio and collision alert | TC-UI-01–04, TC-SET-01–04: verify navigation, input limits, pause/restart, remapping, independent audio and persistence; additionally verify collision alert is one-shot and silent at SFX zero. | Audio model is in fresh CTest; no new listening or native visual results. Prior audio is attributed and build-specific in [test summary](../qa/TEST_SUMMARY_REPORT.md). |
| Leaderboard, schema validation and safe writes | TC-LB-01–04, TC-REL-01, TC-SEC-01–06: test corrupt/missing data, limits, recovery and fault paths; distinguish Linux injection from Windows test-only child-process injection. | Fresh Linux headless suites passed; PR #76 Windows 11 evidence and its limitations are separately recorded in the matrix. Physical power loss, real disk-full, and native Windows 10/macOS crash injection remain untested in that record. |
| System performance, portability and usability | TC-CORE-01–03, TC-PERF-01–02, TC-PORT-01, TC-USE-01: retain prescribed measurement and volunteer protocols. | Not completed by this headless run. See [system report](SYSTEM_VALIDATION_REPORT.md). |

For each outstanding execution, record exact build/platform, test date, tester, steps, expected and actual observations, pass/fail/blocked and evidence link. Preserve baseline IDs and the deviation decisions in [the deviation matrix](../qa/REQUIREMENTS_DEVIATION_MATRIX.md). The new 21/21 result is additional automated evidence, not 44/44 case completion.

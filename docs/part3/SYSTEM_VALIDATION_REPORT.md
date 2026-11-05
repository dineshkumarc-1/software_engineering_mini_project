# System validation report — Part 3

**Tested source:** `04d782a345464349da10c91d6796b3ed99fc7b96` (tree `e7f5e8ef5f943d067375af0b375420bf81b421f3`), clean detached checkout of main.  
**Run:** 5 November 2026 UTC, automated Linux headless Debug coverage build; not a Windows or human acceptance test.  
**Status:** In progress; no final QA sign-off.

**Not complete.** New Linux/Xvfb system checks were performed; no Windows or human acceptance validation was performed in this report. Prior evidence has distinct build and platform identities; do not claim it as a fresh run on the tested commit.

## Fresh Linux desktop checks

The current source was built as a Linux Release desktop executable (SHA-256 `b5c5c2cc8cb059e44e0cbf4ab881c2e33b639b16e89e3df4ad97dbcca1d62d8c`). Automated Xvfb/Mesa software-display checks ran with a null audio device and process capabilities dropped:

| Check | Result | Limitation |
|---|---|---|
| Navigation and settings | 23/23 checks | Linux automation, not Windows acceptance |
| Settings | 22/22 checks | Null audio is not audible listening |
| Leaderboard | 29/29 checks | Linux visual/runtime smoke, not Windows case approval |
| Security UI | 23/23 checks | Scoped input and runtime-file checks; not universal privacy proof |
| Power-ups | 16/16 checks; Speed, Multiplier and Shrink collected | 150-second automated run; not the required 300-second performance campaign |
| SFML/OpenAL backend | 7/7 checks | Null audio; no human listening |

[Execution evidence](EVIDENCE.md) contains the outputs. Checks overlap and must not be counted as unique STP case passes. No fresh Windows execution, reference-hardware performance campaign or human usability/listening test was performed.

## Existing evidence and outstanding work

- [44-case execution matrix](../qa/STP_EXECUTION_MATRIX.md) records 24 automated-scope passed, 16 partial, three pending acceptance measurements and one pending human walkthrough. Those are prior reconciliation statuses, not final case passes.
- [Sprint 2 test summary](../qa/TEST_SUMMARY_REPORT.md), [quality gates](../quality-gates.md) and [deviations](../qa/REQUIREMENTS_DEVIATION_MATRIX.md) preserve platform, artifact and approval limitations. Earlier Windows 10/11 evidence and scoped owner acceptance do not prove a new run on `04d782a345464349da10c91d6796b3ed99fc7b96`.
- [Fresh demo guide](../../demo/README.md), where present, is demonstration evidence, not a replacement for prescribed measurements or independent observers.

## Priority completion actions

1. Execute and record outstanding case steps from the matrix: 10 first-grid measurements, 50 system ticks per difficulty, physical input/navigation, timed mode, theme and score visibility, leaderboard, pause, settings, privacy and native cases. Preserve actual environment, source and binary hashes, dates, tester, expected and observed results.
2. Run three continuous 300-second Hard/Obstacle FPS sessions on reference hardware and the approved 100 random-time input acknowledgement protocol, with full accepted/rejected accounting and correct timestamp scope. Diagnostic probes alone are not acceptance.
3. Obtain three outside-team first-time volunteers for the no-help usability walkthrough, counting primary screens.
4. Reconcile deviations and outstanding native evidence with the responsible owners and designated QA approver. Retain prior Issue #24 accepted scoped firewall evidence; no repeat is implied.

STP exit criteria include 100% planned execution, all High cases passing, all 43 requirements supported by passing evidence, defect/deviation disposition, C6–C11 >=80% line coverage, and clean sanitizer/cppcheck results. Fresh coverage is documented separately; the other criteria are not established by this headless run. No final sign-off or release decision is claimed.

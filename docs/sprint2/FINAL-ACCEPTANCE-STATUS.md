# Sprint 2 Final Acceptance Status

**Decision: NOT APPROVED — final QA sign-off remains blocked.**

This is the refreshed candidate assembled for validation; it is not a release tag, a merge into main, or a substitute for the QA owner's sign-off.

## Candidate composition

- Owner integration starting point: `dbf08b51b9465e7ceac66e5200b8bcad874db408` (PR #53; retained as the base branch).
- Persistence source: `0d516f34535cc213bee4b652a52c666208d55d51` (PR #48).
- Fuzz/offline audit and reviewed runtime evidence: `6e281183a8d8a022c80a9f1f1dced2383a0a0bfc` (PR #50).
- Canonical corpus promotion: `03794519c28cad06b3abd9aaad1cf302453cd149` (PR #52). The six obsolete `[!shouldfail]` tags and the two `KnownAccepted` malformed-fixture exceptions were removed. The malformed fixture loop now requires all 25 cases to be rejected.
- This candidate additionally handles the zero-byte leaderboard warning (D6), adds mode/difficulty/date columns to the leaderboard UI (D7), adds regression coverage, and registers the Linux fault-injection suite. Current-candidate CI must validate these last changes.

## Automated checks observed before this refreshed candidate

| Component / commit | Observed result |
|---|---|
| PR #48 `0d516f3` | 6/6 standard GitHub Actions jobs passed |
| PR #50 `6e28118` | 6/6 standard GitHub Actions jobs passed |
| PR #52 `0379451` | 6/6 standard GitHub Actions jobs passed after canonical expected-failure promotion |
| Private integrated source composition | Owner reported cppcheck 2.13: zero findings, no suppressions |
| Private ordinary-expectation security corpus | 20 cases / 728 assertions passed |
| Private crash/fault-injection copy | 8 cases / 214 assertions passed |

These component/private results are not a substitute for the full integration workflow on the exact refreshed candidate head. The newly added D6/D7 fixes require that fresh run, including cppcheck and ASan/UBSan.

## Offline runtime evidence reviewed

The uploaded Windows 11 JSON (capture dated 2026-10-29) records 60 process samples; the game process was running in all 60, with no process-owned TCP connection records and no UDP endpoint records in those sampled snapshots. The executable-scoped outbound-block rule was verified active in ActiveStore at start and end. Bharath confirmed that the game remained usable while the rule was active.

The evidence timing is qualified: although the script was configured for 60 seconds, sample timestamps span approximately 98.35 seconds, and capture-start to final JSON timestamp spans approximately 104.70 seconds because each loop includes endpoint-query overhead. This is point-in-time endpoint sampling, not packet capture; transient activity between samples is not ruled out. The raw JSON was not committed because it contains local machine/path information. Its SHA-256 is recorded in the Issue #24 evidence document.

## Outstanding acceptance gates

1. **Fresh integration CI on this candidate head:** all standard build/test jobs, cppcheck 2.13, and Clang ASan/UBSan must pass on the exact candidate SHA after the D6/D7 changes.
2. **Windows 10 / final-candidate Windows 11 validation:** the previous offline capture was taken against a local executable before these D6/D7 edits. Build and launch the current candidate artifact on supported target OS versions and attach traceable evidence.
3. **D6/D7 acceptance:** verify the empty-file warning appears and is logged, a valid score recovers the empty file atomically, and the six-column leaderboard renders every required field on the current candidate.
4. **Performance acceptance:** target cold-launch under 2 seconds and end-to-end input acknowledgement under 100 ms remain not assessed on the required target environment.
5. **Full STP / usability / requirement traceability:** execution of the full planned test inventory, High-priority pass rate, first-time usability participants, requirements-deviation reconciliation, defect exit criteria, and final Test Summary Report are not all evidenced as complete.
6. **Human approval:** Ayushi's final QA decision and the owner's integration/release approval are still required. They cannot be inferred from green CI or private verification.
7. **D5 policy limitation:** cleanup is best-effort for matching files older than 24 hours; it does not guarantee cleanup on the very next startup.

## Final decision

**Do not mark Sprint 2 as signed off, close the linked issues, merge the release stack, tag a final release, freeze development, or record the final demo yet.** Re-evaluate this status only after the current-candidate workflow and the missing platform/manual acceptance evidence are recorded against the exact final integrated SHA.

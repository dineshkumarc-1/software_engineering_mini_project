# Windows 11 portability and exact-build privacy — preflight handoff

**Prepared:** 2026-10-31  
**Scope:** Issues #23 (TC-PORT-01) and #24 (SEC-002..004 / SEC-007)  
**Status:** Preflight only. This document does not claim a Windows 11 build, test, gameplay run, privacy capture, or QA sign-off.

## Candidate identity — blocked pending owner pin

The existing records refer to more than one candidate:

- Historical final-QA pin: `7c6733d95a4fa61a3af83c413a27d1ad2f40d44b`.
- Product revision including PR #60: `3fcc586d740d2b9b461193e482d6861ca8f905e5`.
- Current repository main at preparation time: `5b947efaeccfcf56f874d068854df935256eafc0`. Its delta from `3fcc586` is documentation/evidence only (QA checklist, deviation matrix, release closeout, test summary, the #60 gate log and quality-gates document); this is not a selection of the accepted test candidate.

Dinesh and Tejas must name one exact full SHA and agree it is the shared target for both issues. The test must not be run against a branch tip or a prebuilt Windows 10 executable by inference. Record the accepted SHA, tree SHA, and a clean checkout before building.

Coordination requests:
- [Issue #23 candidate pin request](https://github.com/dineshkumarc-1/software_engineering_mini_project/issues/23#issuecomment-6545038214)
- [Issue #23 Windows 11 availability confirmation](https://github.com/dineshkumarc-1/software_engineering_mini_project/issues/23#issuecomment-6545105677)

## TC-PORT-01 — Windows 11

**Status: Not tested.** No Windows 11 source build or visible-client gameplay test is represented by this handoff. The exact OS edition/version/build, Visual Studio/MSVC, CMake version, source-tree status, compiler output, configure/build/CTest logs, EXE size and EXE SHA-256 must come from the actual Windows 11 machine.

Use the documented commands and options in [quality-gates.md](../../quality-gates.md), from a clean checkout of the agreed SHA:

```powershell
git checkout --detach <AGREED_FULL_SHA>
git rev-parse HEAD
git status --porcelain
cmake -S . -B build -DSNAKE_FETCH_SFML=ON -DBUILD_SHARED_LIBS=OFF
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
(Get-Item .\build\Release\SnakeXtreme.exe).Length
Get-FileHash .\build\Release\SnakeXtreme.exe -Algorithm SHA256
```

Run `ctest -N -C Release` against that configured build to record the actual Windows test inventory. Do not hard-code a Linux count: the historical Windows 10 evidence records 20/20 because `secure_write_fault_tests` is registered only on Linux in the candidate CMake configuration. Use the registered tests from the agreed SHA.

The manual checklist and evidence must include:
1. Menu window opens and visible menu text confirms font loading.
2. Classic: eat food and verify +10.
3. Timed: countdown runs from 1:00.
4. Obstacle: collision with brown block displays “You hit an obstacle”.
5. Collect a power-up; HUD shows name and seconds remaining.
6. Esc pauses/resumes; M returns to menu; Esc on menu exits.
7. Window X exits cleanly.
8. Leaderboard shows a score after a game; a changed setting survives restart.

Required evidence: complete configure/build/CTest logs (including warnings/failures), `winver` screenshot, menu/gameplay/leaderboard client-area screenshots, manual observations, source SHA/tree/clean status, toolchain, and the built EXE size/hash. Preserve failed attempts and their original logs.

A helper script was prepared outside the repository for running the documented clean checkout/build/log capture. It has not been executed or validated on PowerShell/Windows in this environment and is not a substitute for the actual Windows run.

## Issue #24 — exact-build privacy

**Status: Blocked pending candidate pin, collector correction and explicit owner approval for elevation/firewall changes.**

Historical public evidence:
- [Redacted 60-observation JSON](https://github.com/dineshkumarc-1/software_engineering_mini_project/blob/d59d85e2e028a244fee71d8b95c5e2e77e6c8b66/docs/sprint2/evidence/issue-24-offline-evidence.redacted.json)
- [Historical privacy report at that commit](https://github.com/dineshkumarc-1/software_engineering_mini_project/blob/d59d85e2e028a244fee71d8b95c5e2e77e6c8b66/docs/sprint2/evidence/issue-24-fuzz-privacy-audit.md)

That capture has 60 samples with the process reported running in every sample, zero observed process-owned TCP records and zero UDP endpoint records, and ActiveStore verification at the start and end. The requested 60 seconds took about 104.70 seconds overall; sample timestamps span about 98.35 seconds with observed intervals about 1.521–1.999 seconds (average about 1.667 seconds). It lacks the source SHA, executable size and executable SHA-256, so it is historical only and is not attributable to the accepted candidate.

### Existing collector review

The existing [PowerShell collector](https://github.com/dineshkumarc-1/software_engineering_mini_project/blob/feature/issue-24-fuzz-privacy-audit/docs/sprint2/evidence/Collect-Issue24-OfflineEvidence.ps1) was statically inspected. Before it is used for final evidence, it needs to:
- preserve per-sample TCP/UDP query errors rather than suppressing them;
- record process-liveness query errors separately from a process that has exited;
- calculate and record actual elapsed duration and sampling-interval statistics from timestamps;
- record cleanup outcome after the temporary rule is removed, including a post-removal ActiveStore check.

The current script writes its JSON before its `finally` cleanup and silently ignores removal errors, so its existing JSON does not prove cleanup succeeded. The source review and findings are recorded on [Issue #24](https://github.com/dineshkumarc-1/software_engineering_mini_project/issues/24#issuecomment-6545090501).

### Approval gate

[Dinesh was asked to explicitly approve or decline](https://github.com/dineshkumarc-1/software_engineering_mini_project/issues/24#issuecomment-6545039355) the following limited operation: elevate PowerShell, create one temporary outbound-block rule scoped only to the exact agreed test EXE, verify that rule in ActiveStore, exercise the game offline, remove only that temporary rule, and verify it is absent afterward. No profile-wide block or unrelated firewall-policy change is intended. No elevation or firewall change has been made as part of this handoff.

After approval and collector correction, retain raw JSON privately, publish only a reviewed redacted copy, and record source SHA/tree, build configuration/toolchain, EXE size/hash separately from the JSON evidence hash, all timestamps, sampling interval/duration, collection errors, firewall rule verification, game usability observations, and cleanup verification.

Interpretation limit: process-owned TCP/UDP endpoint snapshots are point-in-time observations, not packet capture. They cannot rule out brief activity between samples or prove that no possible network attempt occurred.

## Persistence/security handoff — existing evidence, not duplicated

Use the already-published regression evidence:
- [Run 3 persistence-fix verification](../../qa/evidence/issue-18-run3-fix-verification.txt)
- [Run 4 final-candidate regression evidence](../../qa/evidence/issue-18-23-final-candidate-7c6733d.txt)
- [Full TC-PWR-LB-SEC execution record](../../qa/TC-PWR-LB-SEC-EXECUTION.md)
- [24-hour cleanup policy](./issue-15-17-temp-cleanup-policy.md)

The historical Run 4 reports D6 at 4/4 assertions and the Linux fault-injection suite at 214 assertions / 8 cases on candidate `7c6733d`. Keep that scoped to its recorded candidate/platform; do not relabel it as a new Windows 11 run.

The persistence cleanup contract remains **24-hour best-effort**. Eligible stale temporary/orphan-lock files are removed by a later read or write after the age threshold. Fresh crash leftovers are intentionally preserved; cleanup is not guaranteed on the next startup.

Deterministic malformed/truncated-input tests are **fuzz-style regression tests**, not coverage-guided fuzzing. Linux `LD_PRELOAD` crash-injection evidence is not native Windows crash-injection evidence. Native Windows locked-file refusal tests are separate and do not prove full crash-injection coverage. Tejas has been asked to confirm the outstanding Windows crash-injection disposition on [Issue #24](https://github.com/dineshkumarc-1/software_engineering_mini_project/issues/24#issuecomment-6545052538).

## Current status table

| Work item | Status | Evidence / action |
|---|---|---|
| Shared exact candidate SHA | **Blocked** | Await Dinesh + Tejas full-SHA confirmation |
| Windows 11 clean source checkout/build | **Not tested** | Run TC-PORT-01 on the agreed SHA |
| Windows 11 CTest | **Not tested** | Record `ctest -N` inventory and complete CTest output on that SHA |
| Windows 11 EXE size/hash | **Not tested** | Hash the locally generated Release EXE |
| Windows 11 gameplay/manual screenshots | **Not tested** | Execute all checklist observations and capture real client area |
| Historical privacy observations | **Pass, historical only** | 60 snapshots; not tied to exact source/EXE identity |
| Final-candidate passive TCP/UDP capture | **Blocked** | Candidate pin + collector corrections |
| Firewall/offline privacy test | **Blocked** | Dinesh approval required before elevation/rule creation |
| Persistence regression (existing Linux evidence) | **Pass, scoped** | Run 3 / Run 4 links above |
| Native Windows crash injection | **Not tested / Pending** | Tejas to confirm method or record remaining gap |
| Final review PR and issue closure | **Blocked** | Assemble after actual Windows evidence is returned; issues remain open |

No issue closure, final QA approval, Sprint 2 sign-off, release/tag/freeze, or demo approval is implied by this preflight note.

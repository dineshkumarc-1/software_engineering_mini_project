# Windows 11 portability and exact-build privacy — preflight handoff

**Prepared:** 2026-10-31  
**Scope:** Issues #23 (TC-PORT-01) and #24 (SEC-002..004 / SEC-007)  
**Status:** Preflight only. This document does not claim a Windows 11 build, test, gameplay run, privacy capture, or QA sign-off.

## Candidate identity — pinned for this verification campaign

**Selected test candidate:** `3fcc586d740d2b9b461193e482d6861ca8f905e5`  
**Source tree:** `41b54378c177d7d1e01ff17f2bf7b22663789ec6`  
**Reason:** This is the merged #60 product revision containing the integrated collision/Game Over audio change.

This exact SHA is selected **only for the Windows 11 verification campaign**. It is not final release approval, a release tag, a scope waiver or Sprint 2 sign-off. The Windows 11 source build and privacy collection must both use this exact commit and its own generated EXE identity.

The earlier candidate `7c6733d95a4fa61a3af83c413a27d1ad2f40d44b` remains historical QA evidence only. Its Windows 10 EXE and test results are not relabelled. Windows 10 evidence is recorded separately below. Current main at `447b84c8f6e8196b78009be7d1b70b86e6bc916b` differs from the selected candidate only in QA documents/evidence, but the required Windows 11 clean checkout is still made directly from the exact selected SHA.

Tejas's written Windows TC-PORT-01 instructions on [Issue #23](https://github.com/dineshkumarc-1/software_engineering_mini_project/issues/23#issuecomment-6528475763) specify **20/20 CTest on Windows** because `secure_write_fault_tests` is Linux-only. Capture `ctest -N -C Release` on the selected SHA and preserve any inventory discrepancy rather than changing the expectation.

Owner confirmation:
- Candidate SHA selected by Dinesh: `3fcc586d740d2b9b461193e482d6861ca8f905e5` (this conversation, 31 Oct 2026).
- Campaign scope: test verification only; final acceptance remains pending review.

## Separate Windows 10 evidence — do not relabel

The earlier Windows 10 artifacts are retained as independent records and are not inputs to, substitutes for, or evidence of this Windows 11 source-build campaign:

- **Windows 10 baseline report:** source `7c6733d95a4fa61a3af83c413a27d1ad2f40d44b`, tree `e83825372214b6c11296c59d47301cf236a5f716`; Windows 10 Pro build 19045; MSVC 19.44.35229.0 / CMake 4.4.4; reported EXE size 1,906,688 bytes and SHA-256 `FE3AA1316D634611F3D690665C5D802696BE80DA5124C34B2475D05391CEF1F3`; reported 20/20 CTest. The report register says the complete raw logs and actual OpenGL client-area screenshot are not all uploaded for independent review. See [WINDOWS10-EVIDENCE-REGISTER.md](../../qa/WINDOWS10-EVIDENCE-REGISTER.md).
- **Separate Windows 10 PR #60 audio build:** source revision `09bbf96e14d9b8f037d46b4b55e07d32aeacf26c`; EXE size 1,906,688 bytes and SHA-256 `780B5AB208B7FD6DF91916E4E386848E179FE4B71CBA27BC4BCEE60207D2E946`. Its reported 20/20 CTest, audio assertions and backend checks remain attached to that actual build identity; do not relabel it as a binary built from the later merge commit.
- **Windows 11:** this campaign must produce its own detached source checkout, source/tree SHA, toolchain details, configure/build/CTest logs, game screenshots and EXE size/SHA-256. No Windows 10 binary, report, screenshot or privacy observation is reused as Windows 11 evidence.

## Supplemental hosted Windows CI — not TC-PORT-01

GitHub Actions run [#41215354989](https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/41215354989) for PR #67 completed **8/8 CI jobs successfully**. The Windows jobs were `Tests (Windows MSVC)`, `Tests (Windows MinGW-w64)` and `App build (Windows MSVC)`. The desktop MSVC job completed Release configure/build, then **20/20 CTest passed** (2.65 seconds), and uploaded `SnakeXtreme.exe`.

This run's PR merge snapshot had base `5b947efaeccfcf56f874d068854df935256eafc0` and PR head `61c9dac732d6a9d5ae252e65833ff63f3156bfec`. Comparing the base against the selected candidate `3fcc586` shows only QA documentation/evidence changes; the product source/assets/build inputs are unchanged. Still, this is a GitHub-hosted `windows-latest` runner and a PR merge snapshot, **not a clean Windows 11 PC checkout of the selected SHA**, not manual visible-client gameplay, and not the requested local Windows 11 EXE hash.

The uploaded CI artifact `SnakeXtreme-windows-x64` from that hosted run contains a 1,903,104-byte EXE with SHA-256 `f56a53a288f1d16abc633718d70ad691682a0057011c52171117d07c82b26495`. This identity is recorded only as a hosted-CI artifact; it must not be attributed to Windows 11, the local machine, or the exact selected commit. CI logs also show non-project Node.js deprecation warnings from GitHub Actions tooling; preserve them as CI/tool warnings, not application compiler warnings.

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

Tejas's documented Windows expectation is **20/20**: `secure_write_fault_tests` is registered only on Linux in the candidate CMake configuration. Run `ctest -N -C Release` after the build to record the actual Windows inventory. If the agreed SHA reports anything other than 20 registered tests, preserve the output and report the discrepancy; do not silently change the expectation.

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

A clean-checkout runner is included as [`Run-TC-PORT-01-Windows11.ps1`](./Run-TC-PORT-01-Windows11.ps1). Run it from a **Visual Studio 2022 Developer PowerShell** with Git, MSVC, CMake and CTest available:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\Run-TC-PORT-01-Windows11.ps1 -CandidateSha <AGREED_FULL_SHA> -EvidenceRoot .\TC-PORT-01-Windows11-Evidence-<UNIQUE_SUFFIX>
```

It refuses to reuse an existing evidence directory, verifies the requested full SHA and clean source checkout, builds out-of-tree using the documented CMake options, saves complete configure/build/CTest/toolchain logs, records `ctest -N`, and treats **20** as the Windows inventory expectation. It also records the generated EXE size and SHA-256, opens `winver` first, then launches the game. The operator is prompted for the actual result (`Passed`, `Failed`, or `Not tested`), observation and screenshot filename for each checklist item; if the EXE is absent, all game checks are recorded **Not tested**. Actual client screenshots must still be captured manually and independently inspected. The helper has not been run or validated on Windows/PowerShell here, so preserve any first-run errors as evidence rather than assuming success.

## Issue #24 — exact-build privacy

**Status: Candidate pinned; collector corrections prepared and Tejas review requested. Passive exact-build capture is ready once the Windows 11 EXE exists. The outbound firewall step remains blocked pending Dinesh's explicit approval.**

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

A replacement helper, [`Collect-Issue24-ExactBuildEvidence.ps1`](./Collect-Issue24-ExactBuildEvidence.ps1), is included in this draft. Passive process-owned TCP/UDP/liveness sampling is the default and does not create firewall rules. It records exact source commit/tree, EXE byte size/SHA-256, OS and PowerShell identity, UTC sample timestamps, actual elapsed time, sampling interval min/average/max, and per-sample/global collection errors. It writes the JSON after the cleanup path has completed. The helper has not been executed or validated on Windows here.

Passive capture command (run against the EXE produced by the agreed Windows 11 source build):

```powershell
$source = .\TC-PORT-01-Windows11-Evidence-<UNIQUE_SUFFIX>\source
$sourceSha = (git -C $source rev-parse HEAD).Trim()
$treeSha = (git -C $source rev-parse 'HEAD^{tree}').Trim()
.\Collect-Issue24-ExactBuildEvidence.ps1 -ExecutablePath .\TC-PORT-01-Windows11-Evidence-<UNIQUE_SUFFIX>\build\Release\SnakeXtreme.exe -SourceCommitSha $sourceSha -SourceTreeSha $treeSha -DurationSeconds 60 -OutputPath .\issue-24-exact-build-raw.json
```

Only after explicit owner approval in the conversation, the separate firewall run may be made from an elevated PowerShell session by adding `-EnableScopedOutboundBlock`. The script then requires the operator to type `APPROVED`, verifies the exact executable-scoped outbound block in ActiveStore before and during sampling, removes only the unique rule it created, and records whether it is absent afterward. The typed phrase is an additional guard, not a substitute for obtaining Dinesh's explicit approval. Do not launch this mode before that approval. The raw JSON includes the computer name and full EXE path; keep it private and publish only a reviewed redacted copy. A helper [`New-Redacted-Issue24Evidence.ps1`](./New-Redacted-Issue24Evidence.ps1) creates a derivative with the computer name, executable path and temporary firewall rule name redacted, and prints the redacted artifact SHA-256:

```powershell
.\New-Redacted-Issue24Evidence.ps1 -RawEvidencePath .\issue-24-exact-build-raw.json -RedactedOutputPath .\issue-24-exact-build-redacted.json
Get-Content .\issue-24-exact-build-redacted.json
```

The script does **not** publish files. Manually review the entire redacted JSON for paths, usernames, hostnames or local details in collection error messages before adding the redacted derivative and its SHA-256 manifest to the PR. Never commit the raw file.

### Approval gate

[Dinesh was asked to explicitly approve or decline](https://github.com/dineshkumarc-1/software_engineering_mini_project/issues/24#issuecomment-6545039355) the following limited operation: elevate PowerShell, create one temporary outbound-block rule scoped only to the exact agreed test EXE, verify that rule in ActiveStore, exercise the game offline, remove only that temporary rule, and verify it is absent afterward. No profile-wide block or unrelated firewall-policy change is intended. No elevation or firewall change has been made as part of this handoff.

After Tejas reviews the collector, run passive exact-build capture against the locally built EXE. Run the separate firewall mode only after Dinesh's explicit approval. Retain raw JSON privately, publish only a reviewed redacted copy, and record source SHA/tree, build configuration/toolchain, EXE size/hash separately from the JSON evidence hash, all timestamps, sampling interval/duration, collection errors, firewall rule verification, game usability observations, and cleanup verification.

Interpretation limit: process-owned TCP/UDP endpoint snapshots are point-in-time observations, not packet capture. They cannot rule out brief activity between samples or prove that no possible network attempt occurred.

## Persistence/security handoff — existing evidence, not duplicated

Use the already-published regression evidence:
- [Run 3 persistence-fix verification](../../qa/evidence/issue-18-run3-fix-verification.txt)
- [Run 4 final-candidate regression evidence](../../qa/evidence/issue-18-23-final-candidate-7c6733d.txt)
- [Full TC-PWR-LB-SEC execution record](../../qa/TC-PWR-LB-SEC-EXECUTION.md)
- [24-hour cleanup policy](./issue-15-17-temp-cleanup-policy.md)

The historical Run 4 reports D6 at 4/4 assertions and the Linux fault-injection suite at 214 assertions / 8 cases on candidate `7c6733d`. Keep that scoped to its recorded candidate/platform; do not relabel it as a new Windows 11 run.

The persistence cleanup contract remains **24-hour best-effort**. Eligible stale temporary/orphan-lock files are removed by a later read or write after the age threshold. Fresh crash leftovers are intentionally preserved; cleanup is not guaranteed on the next startup.

Deterministic malformed/truncated-input tests are **fuzz-style regression tests**, not coverage-guided fuzzing. Linux `LD_PRELOAD` crash-injection evidence is not native Windows crash-injection evidence. Native Windows locked-file refusal tests are separate and do not prove full crash-injection coverage. Tejas's [Issue #23 handoff](https://github.com/dineshkumarc-1/software_engineering_mini_project/issues/23#issuecomment-6528464409) already records Windows crash injection as **Pending** because no native injector exists. Tejas suggests a test-only interposer DLL (for example, Detours or IAT hooks) on `WriteFile`, `FlushFileBuffers`, `MoveFileExW` and `ReplaceFileW`, loaded only into a child test process; the parent would verify old/new board recovery. This must not add a product hook. Dinesh must decide whether this additional platform-specific test is in scope or is documented as a limitation.

## Current status table

| Work item | Status | Evidence / action |
|---|---|---|
| Shared exact candidate SHA | **Pinned for this campaign** | `3fcc586d740d2b9b461193e482d6861ca8f905e5` (not release approval) |
| Windows 11 clean source checkout/build | **Not tested** | Run TC-PORT-01 on pinned SHA; hosted Windows CI is supplemental only |
| Windows 11 CTest | **Not tested locally** | Hosted Windows MSVC CTest 20/20 passed; actual Windows 11 run still required |
| Windows 11 EXE size/hash | **Not tested** | Hash the locally generated Release EXE |
| Windows 11 gameplay/manual screenshots | **Not tested** | Execute all checklist observations and capture real client area |
| Historical privacy observations | **Pass, historical only** | 60 snapshots; not tied to exact source/EXE identity |
| Exact-build passive TCP/UDP/liveness collector | **Prepared; not run** | Run helper against EXE built from pinned SHA |
| Redacted privacy evidence helper | **Prepared; not run** | Produces a redacted derivative; manual review and exact-build raw capture still required |
| Firewall/offline privacy test | **Blocked** | Dinesh approval required before elevation/rule creation; no rule has been created in this work |
| Persistence regression (existing Linux evidence) | **Pass, scoped** | Run 3 / Run 4 links above |
| Native Windows crash injection | **Not tested / Pending** | Tejas records no native injector; proposed test-only interposer DLL (Detours/IAT); Dinesh decides scope vs limitation |
| Hosted Windows CI | **Pass, supplemental only** | Run `41215354989`: Windows MSVC app build and 20/20 CTest; not TC-PORT-01 |
| Final review PR and issue closure | **Blocked** | Review actual Windows 11 evidence and Tejas collector review; issues remain open |

No issue closure, final QA approval, Sprint 2 sign-off, release/tag/freeze, or demo approval is implied by this preflight note.

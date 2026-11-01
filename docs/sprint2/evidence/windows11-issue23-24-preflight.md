# Windows 11 portability and exact-build privacy — preflight handoff

**Prepared:** 2026-10-31  
**Scope:** Issues #23 (TC-PORT-01) and #24 (SEC-002..004 / SEC-007)  
**Status:** Windows 11 local GCC/MinGW and Visual Studio 2022/MSVC Release builds have both completed, each with a matching 20/20 CTest pass on the pinned candidate. The original MSVC manual checklist remains recorded as **7/8 passed and 1 failed**; its initial leaderboard-after-restart observation was made before the relaunch was controlled to use the same working directory. A separate controlled MSVC persistence retest on 1 November 2026 used the same executable and identical isolated working directory before and after restart: `leaderboard.json` and the `Neon` setting both survived, with matching before/after hashes. The original failure is preserved as historical evidence; the controlled retest suggests a likely working-directory mismatch, not a reproduced persistence defect. The original eight MSVC screenshots and five controlled-retest screenshots, JSON snapshots, launch logs, summary and hash manifest are committed. The controlled-retest package is under `docs/sprint2/evidence/msvc-persistence-retest-20261101/`. The exact-build passive Issue #24 capture was completed against the GCC executable only; the scoped firewall test remains gated on Dinesh's explicit approval. This remains a partial preflight, **not** final QA sign-off or Sprint 2 sign-off.

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

**Status: Windows 11 local MSVC build and automated checks passed; the original manual client checklist remains 7/8 with one historical persistence/leaderboard failure, while the controlled persistence retest passed both leaderboard and settings checks.** The exact candidate was used in a clean fresh checkout, and the runner completed the documented Visual Studio 2022/MSVC Release build and Windows CTest inventory. The original manual result is not relabelled or overwritten; the controlled retest did not reproduce a persistence defect. The exact cause of the initial mismatch remains unproven.

### Actual local Windows 11 MSVC run — completed 1 November 2026 (local time)

- OS: Windows 11 Home Single Language, 64-bit, version/build family 25H2 / OS build `26200.9711` (kernel version `10.0.26200`). A separate `winver` screenshot is saved as `00-msvc-windows-version.png`.
- Source: clean fresh clone at commit `3fcc586d740d2b9b461193e482d6861ca8f905e5`; tree `41b54378c177d7d1e01ff17f2bf7b22663789ec6`. The checkout was clean before build.
- Toolchain: Visual Studio 2022 Community `17.12.35707.178`; MSVC compiler `19.42.34436.0` (tools directory version `14.42.34433`); CMake/CTest `4.5.0-rc1`; Git `2.54.0.windows.1`; Windows PowerShell `5.1.26100.9711`; CMake generator `Visual Studio 17 2022`.
- Configure: `-DSNAKE_FETCH_SFML=ON -DBUILD_SHARED_LIBS=OFF`; configure exit code `0`. Third-party SFML CMake deprecation/author/policy warnings appeared, but configuration and generation completed successfully.
- Build: Release build exit code `0`.
- CTest: Windows inventory contained exactly `20` tests and matched the expected names; **20/20 passed**, inventory exit code `0`, CTest exit code `0`.
- Local MSVC executable: `1,896,960` bytes; SHA-256 `3496FD8C9E8ED95FB886BEF272DBAF9D62A181237E3FDB39474FF7627C3E42B5`.
- Manual checklist: **7 Passed / 1 Failed**. Passed: menu/font visibility; Classic food score +10; Timed countdown observed at 0:55 after starting at 1:00; brown-obstacle collision message; Speed boost HUD name and remaining duration (3.9 seconds observed); Esc pause/resume, M to menu and Esc exit; window-X clean exit.
- **Initial manual check 8 (historical failure):** the first manual relaunch displayed `0` entries / “No scores yet”, despite the previous Game Over screen reporting “Score saved: rank 3”. Settings persistence was not checked during that initial run. This result remains in `manual-gameplay-results.txt` as originally observed and is not overwritten by the controlled follow-up.
- **Controlled persistence retest — 1 November 2026:** the same MSVC executable (1,896,960 bytes; SHA-256 `3496FD8C9E8ED95FB886BEF272DBAF9D62A181237E3FDB39474FF7627C3E42B5`) was launched twice with the exact same isolated working directory. First run wrote `leaderboard.json` (Bharath, score 20, Timed, Medium, date 2026-11-01) and `config.json`. After changing Theme from Classic to Neon, saving, and exiting normally, the same executable was relaunched with the same working directory. The leaderboard entry remained visible and the settings screen still showed Neon. JSON comparisons returned `LeaderboardSurvivedRestart=True` and `SettingsSurvivedRestart=True`.
- Persistence hashes: before/after/restart `leaderboard.json` SHA-256 `57CAB33B258A3236DCC1BE1BABAB8FAD1D450D2224CB21F47792221590567B47` (unchanged); initial Classic `config.json` SHA-256 `79428BEF2B5DFED806FF22A1FAE22A4DA292F65EA664AAA7267622DCC22172D7`; saved and post-restart Neon `config.json` SHA-256 `AAF0B2D53E7862DDC108C7CA5339B1731F6C795AE63F0AC6B0C85EC9CD0949D5` (identical after saving and after restart).
- First-launch stderr recorded only `[WARN] SettingsRepository: E_FILE_MISSING: defaults loaded` because the isolated directory had no initial config; second-launch stderr was empty. Both launches exited normally. The stdout/stderr logs, JSON snapshots, five screenshots, summary and hash manifest are committed under `docs/sprint2/evidence/msvc-persistence-retest-20261101/`.
- The original eight MSVC screenshots are committed in PR #67: `00-msvc-windows-version.png`, `01-msvc-main-menu.png`, `02-msvc-classic-score-plus10.png`, `03-msvc-timed-countdown.png`, `04-msvc-obstacle-collision.png`, `05-msvc-powerup-hud.png`, `06-msvc-pause-screen.png`, and `08-msvc-leaderboard-after-restart-empty.png`. Five additional controlled-persistence screenshots are committed under `docs/sprint2/evidence/msvc-persistence-retest-20261101/`: first-run leaderboard, changed Theme before save, Neon menu after save, leaderboard after restart, and Neon settings after restart. The saved-score Game Over message was observed, but a separate screenshot of that exact message was not captured during this retest; the first-run leaderboard screenshot and persisted JSON independently show the saved entry.
- The runner summary reports no overall error; the candidate and source tree hashes match the pinned values. The MSVC executable identity is distinct from the earlier GCC executable identity and must not be conflated with it.

### Actual local Windows 11 GCC run — supplemental evidence

- OS: Windows 11 Home Single Language, version 25H2, build `26200.9711` (kernel/build number `26200`), 64-bit.
- Source: clean fresh checkout at commit `3fcc586d740d2b9b461193e482d6861ca8f905e5`; tree `41b54378c177d7d1e01ff17f2bf7b22663789ec6`.
- Toolchain: GCC/G++ `16.2.0` (MSYS2 UCRT64), CMake/CTest `4.5.0-rc1`, GNU Make `4.4.1`, generator `MinGW Makefiles`, SFML `2.5.1` with system dependencies (`SFML_USE_SYSTEM_DEPS=ON`, `BUILD_SHARED_LIBS=OFF`). This was a custom GCC configuration using UCRT64 system dependencies, not the repository's documented MSVC configuration.
- Build: Release build completed successfully; `SnakeXtreme` and `audio_backend_smoke` reached 100%; build exit code `0`. Earlier failed attempts were preserved in the private evidence folder. Compiler/dependency warnings were observed in SFML 2.5.1 (`std::auto_ptr` deprecation and a bundled `stb_image.h` diagnostic); no project build failure remained in the successful retry.
- CTest inventory: **20 tests**, matching the expected Windows inventory. Full CTest result: **20/20 passed**, 0 failed, 23.41 seconds; exit code `0`.
- Local executable: `1,495,228` bytes; SHA-256 `c043cd941e55888a13e06cbce5c049729dcb27118e1997f6d941b5e4287e992c`.
- Manual visible states shown: main menu; Classic gameplay with a visible purple diamond power-up; Game Over after wall collision (score 110, saved, rank 1); leaderboard showing rank, name, score, mode, difficulty and date. Four corresponding screenshots were captured during the manual walkthrough. The full eight-item checklist below was **not** completed: Timed countdown, obstacle collision message, pause/resume and menu/exit keyboard flows, window-X exit, and settings persistence after restart have not been evidenced here. Confirm the screenshot image files are saved into the local evidence directory before treating them as attached repository evidence.
- The privacy-reviewed Windows 11 MSVC configure/build/CTest/toolchain and identity evidence is committed under `docs/sprint2/evidence/windows11-msvc-logs-20261101/`: `configure.log`, `build-release.log`, `ctest-inventory.log`, `ctest-release.log`, `toolchain.log`, `source-and-machine-identity.txt`, and `run-summary.txt`. User-profile paths, computer identity and standard absolute installation roots were redacted; the raw local logs remain private and unchanged.

The `Run-TC-PORT-01-Windows11.ps1` helper is MSVC-specific and was executed on the local Windows 11 machine against the pinned candidate, with a successful Release build and 20/20 CTest inventory. The manual checklist is not fully passed on the original run; the controlled persistence retest passed with an identical working directory. Remaining acceptance/review work is captured in the current status table below.

Use the documented commands and options in [quality-gates.md](../../quality-gates.md), from a clean checkout of the agreed SHA:

```powershell
git checkout --detach 3fcc586d740d2b9b461193e482d6861ca8f905e5
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
.\docs\sprint2\evidence\Run-TC-PORT-01-Windows11.ps1 -CandidateSha 3fcc586d740d2b9b461193e482d6861ca8f905e5 -EvidenceRoot .\TC-PORT-01-Windows11-Evidence-20261031
```

It refuses to reuse an existing evidence directory, verifies the requested full SHA and clean source checkout, builds out-of-tree using the documented CMake options, saves complete configure/build/CTest/toolchain logs, records `ctest -N`, and treats **20** as the Windows inventory expectation. It also records the generated EXE size and SHA-256, opens `winver` first, then launches the game. The operator is prompted for the actual result (`Passed`, `Failed`, or `Not tested`), observation and screenshot filename for each checklist item; if the EXE is absent, all game checks are recorded **Not tested**. Actual client screenshots must still be captured manually and independently inspected. The helper has not been run or validated on Windows/PowerShell here, so preserve any first-run errors as evidence rather than assuming success.



## Issue #24 — exact-build privacy

**Status: Exact-build passive GCC capture and one-off owner-approved MSVC scoped-firewall capture completed on 31 October / 1 November 2026; the MSVC temporary rule was removed and its absence verified.** The passive capture used the locally built Windows 11 GCC EXE and did not create or modify a firewall rule. The separate MSVC capture used the local MSVC EXE and Dinesh's explicit one-run authorization; keep the two executable identities and results separate.

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

A replacement helper, [`Collect-Issue24-ExactBuildEvidence.ps1`](./Collect-Issue24-ExactBuildEvidence.ps1), is included in this draft. Passive process-owned TCP/UDP/liveness sampling is the default and does not create firewall rules. It records exact source commit/tree, EXE byte size/SHA-256, OS and PowerShell identity, UTC sample timestamps, actual elapsed time, sampling interval min/average/max, and per-sample/global collection errors. It writes the JSON after the cleanup path has completed. The helper was executed in passive mode against the pinned GCC-built EXE and in the separately approved scoped-firewall mode against the pinned MSVC-built EXE; results are recorded in the respective sections below.

Passive capture command (run against the EXE produced by the agreed Windows 11 source build):

```powershell
$source = .\TC-PORT-01-Windows11-Evidence-20261031\source
$sourceSha = (git -C $source rev-parse HEAD).Trim()
$treeSha = (git -C $source rev-parse 'HEAD^{tree}').Trim()
.\docs\sprint2\evidence\Collect-Issue24-ExactBuildEvidence.ps1 -ExecutablePath .\TC-PORT-01-Windows11-Evidence-20261031\build\Release\SnakeXtreme.exe -SourceCommitSha $sourceSha -SourceTreeSha $treeSha -DurationSeconds 60 -OutputPath .\issue-24-exact-build-raw.json
```

Only after explicit owner approval in the conversation, the separate firewall run may be made from an elevated PowerShell session by adding `-EnableScopedOutboundBlock`. The script then requires the operator to type `APPROVED`, verifies the exact executable-scoped outbound block in ActiveStore before and during sampling, removes only the unique rule it created, and records whether it is absent afterward. The typed phrase is an additional guard, not a substitute for obtaining Dinesh's explicit approval. Do not launch this mode before that approval. The raw JSON includes the computer name and full EXE path; keep it private and publish only a reviewed redacted copy. A helper [`New-Redacted-Issue24Evidence.ps1`](./New-Redacted-Issue24Evidence.ps1) creates a derivative with the computer name, executable path and temporary firewall rule name redacted, and prints the redacted artifact SHA-256:

```powershell
.\docs\sprint2\evidence\New-Redacted-Issue24Evidence.ps1 -RawEvidencePath .\issue-24-exact-build-raw.json -RedactedOutputPath .\issue-24-exact-build-redacted.json
Get-Content .\issue-24-exact-build-redacted.json
```

The script does **not** publish files. Manually review the entire redacted JSON for paths, usernames, hostnames or local details in collection error messages before adding the redacted derivative and its SHA-256 manifest to the PR. Never commit the raw file.

### Actual exact-build passive capture — 31 October 2026

The updated `Collect-Issue24-ExactBuildEvidence.ps1` was executed from PR #67 head `e965d3a08b31c07de8f886355f2cc8a2d7f9a2fd` against the locally built Windows 11 GCC executable described above. The source commit/tree and executable identity were checked by the script and matched the pinned candidate. `Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass` was used only in that PowerShell process to permit the local signed-off script invocation; no machine-wide execution policy was changed.

Published redacted evidence:
- [`issue-24-windows11-gcc-passive-redacted.json`](./issue-24-windows11-gcc-passive-redacted.json)
- [`issue-24-windows11-gcc-passive-redacted.sha256.txt`](./issue-24-windows11-gcc-passive-redacted.sha256.txt)
- Redacted JSON SHA-256: `404FD695AA6D48BA7CE3178911A6676C3C14DF9FE340BED02C16BDFFDE14CE6D`.

Capture results: requested 60 seconds; actual elapsed `60,062.836 ms`; 58 observations spanning `59,047.278 ms` from first to last sample. The 57 observed sample intervals ranged from `1,000.518 ms` to `2,497.032 ms`, average `1,035.917 ms`. The process was reported running in all 58 samples and at the end. TCP and UDP queries succeeded in all 58 samples; no sample contained a process-owned TCP record or UDP endpoint. There were zero per-sample/global collection errors and no fatal error. This records endpoint snapshots only: it does not rule out brief activity between samples or prove that no network attempt is possible.

The capture was **passive**: `Firewall.Requested=false`, `Firewall.RuleCreated=false`; no firewall block rule was created as part of this exact-build run. `GameplayObservation` is `Not recorded`, so the capture proves process liveness and successful endpoint queries, not that gameplay/menu interaction was independently verified during the sampling window. Separate manual UI screenshots are recorded in the TC-PORT-01 section.

The raw JSON remains private on the test machine and was not committed. The redacted derivative was manually reviewed, scanned for the local username, computer name, local paths, evidence-root path, repository path and temporary rule name, and its hash was independently checked against the manifest before being committed in PR #67. Commit `66d76408231b8b3cedcb314f6d1889aa5ebceac1` added only the redacted JSON and hash manifest. A prior read-only check after the historical 29 Oct firewall test found its old rule absent from both ActiveStore and PersistentStore; this is separate from the 31 Oct passive capture.

### Actual exact-build MSVC scoped-firewall capture — 1 November 2026

Dinesh explicitly approved one narrowly scoped firewall run on Issue #24 on 1 November 2026. The approved operation used elevated PowerShell, created one temporary Windows Firewall outbound-block rule scoped only to the exact pinned MSVC executable, verified the exact rule in ActiveStore, exercised the game offline, removed only the rule created by this run, and verified its absence in both PersistentStore and ActiveStore. No profile-wide block or unrelated firewall-policy change was made.

- Platform: Windows 11 Home Single Language, OS version `10.0.26200`, build `26200`; Windows PowerShell `5.1.26100.9711`.
- Source: `3fcc586d740d2b9b461193e482d6861ca8f905e5`; tree `41b54378c177d7d1e01ff17f2bf7b22663789ec6`.
- Executable: MSVC Release `SnakeXtreme.exe`, 1,896,960 bytes; SHA-256 `3496FD8C9E8ED95FB886BEF272DBAF9D62A181237E3FDB39474FF7627C3E42B5`. This is the MSVC binary, distinct from the GCC binary used by the earlier passive capture.
- Capture: requested 60 seconds; actual elapsed `60,121.390 ms`; 57 observations spanning `59,106.795 ms` from first to last sample. All 57 observations reported the process running. TCP query failures: `0`; UDP query failures: `0`; observations with process-owned TCP records: `0`; observations with UDP endpoints: `0`; collection errors: `0`; fatal error: `null`.
- Sampling intervals: 56 intervals; minimum `1,001.446 ms`, average `1,055.478 ms`, maximum `3,371.548 ms`.
- Gameplay observation while the rule was active: "Main menu displayed correctly; started Classic mode; gameplay remained responsive; controls worked; no crash or error dialog observed."
- Firewall result: `VerifiedAtStart=True`; `VerifiedAtEndBeforeCleanup=True`; `CleanupSucceeded=True`; `AbsentInPersistentStore=True`; `AbsentInActiveStore=True`; `VerifiedAbsentAfterCleanup=True`. The temporary rule was removed successfully and is verified absent from both stores.
- Published redacted evidence: [`issue-24-windows11-msvc-firewall-redacted-20261101.json`](./issue-24-windows11-msvc-firewall-redacted-20261101.json) and [matching SHA-256 manifest](./issue-24-windows11-msvc-firewall-redacted-20261101.sha256.txt). Redacted JSON SHA-256: `CDDA55E8D4D3A96F6F0837B4BF9302C9789B407B4D90802FDD01FBFDCA7681C5`. The redacted file was inspected for personal identifiers, absolute paths, escaped paths and the temporary rule GUID; no matches remained. The raw JSON remains private and was not committed.
- The 31 October passive GCC capture remains a different run against a different executable; its results must not be combined with this MSVC capture. Both runs record sampled process-owned endpoint snapshots, not packet capture. The maximum interval in this MSVC run was `3,371.548 ms`, so gaps between observations remain; these results do not prove that no brief network activity or attempted connection occurred between samples.

### Owner approval and execution

Dinesh's authorization applied to the single 1 November 2026 run described above. It is not blanket approval for any additional firewall change or repeat run; seek fresh explicit owner approval for any further firewall-policy operation.

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
| Windows 11 clean source checkout/build | **MSVC and GCC Release builds passed** | Exact candidate/tree; local MSVC 19.42.34436.0 and separate GCC 16.2.0 builds; see distinct executable identities |
| Windows 11 CTest | **Pass, local MSVC and GCC** | Each local build matched the expected 20-test inventory; 20/20 passed in each configuration |
| Windows 11 MSVC build logs and toolchain identity | **Published, privacy-reviewed** | Seven redacted configure/build/CTest/toolchain/identity/summary files committed in `docs/sprint2/evidence/windows11-msvc-logs-20261101/`; raw logs remain local |
| Windows 11 EXE size/hash | **Recorded, local GCC and MSVC** | GCC: 1,495,228 bytes / `c043cd941e55888a13e06cbce5c049729dcb27118e1997f6d941b5e4287e992c`; MSVC: 1,896,960 bytes / `3496FD8C9E8ED95FB886BEF272DBAF9D62A181237E3FDB39474FF7627C3E42B5` |
| Windows 11 gameplay/manual screenshots and persistence | **Original manual: 7/8; controlled persistence retest: Pass** | Original initial restart mismatch retained; controlled identical-working-directory retest confirmed leaderboard and Neon settings survive restart. All 8 original MSVC PNGs and 5 retest PNGs plus JSON/log/hash artifacts are committed |
| Historical privacy observations | **Pass, historical only** | 29 Oct capture used another executable; not tied to exact source/EXE identity |
| Exact-build passive GCC TCP/UDP/liveness capture | **Completed, passive** | 31 Oct capture: 58 samples, 0 query failures/errors, no TCP/UDP records; no firewall rule created; distinct from the later MSVC scoped-firewall run |
| Redacted privacy evidence | **Published for both runs** | GCC passive JSON + manifest committed in `66d7640`; MSVC scoped-firewall JSON + manifest committed in `c84c8a6`; raw files remain private |
| Exact-build MSVC firewall/offline privacy test | **Run completed; cleanup verified; review pending** | Dinesh approved the one-off run on 1 Nov 2026. Exact EXE-scoped rule verified at start/end; 57 samples, 0 query failures/errors, no TCP/UDP records; rule removed and absent from PersistentStore and ActiveStore; redacted JSON + hash manifest committed in `c84c8a6` |
| Persistence regression (existing Linux evidence) | **Pass, scoped** | Run 3 / Run 4 links above |
| Native Windows crash injection | **Not tested / Pending** | Tejas records no native injector; proposed test-only interposer DLL (Detours/IAT); Dinesh decides scope vs limitation |
| Hosted Windows CI | **Pass, supplemental only** | Run `41215354989`: Windows MSVC app build and 20/20 CTest; not TC-PORT-01 |
| Final review PR and issue closure | **Blocked** | Review actual Windows 11 evidence and Tejas collector review; issues remain open |

No issue closure, final QA approval, Sprint 2 sign-off, release/tag/freeze, or demo approval is implied by this preflight note.

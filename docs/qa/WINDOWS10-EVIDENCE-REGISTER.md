# Received Windows 10 evidence register

**State:** user-supplied report reviewed; not a new execution by this agent. Full referenced raw logs and actual OpenGL client-area screenshot have not all been uploaded for independent review. Do not publish local usernames/hostnames/absolute personal paths.

## Exact baseline identity

| Field | Received information |
|---|---|
| Source | `7c6733d95a4fa61a3af83c413a27d1ad2f40d44b` |
| Source tree | `e83825372214b6c11296c59d47301cf236a5f716`; reported clean |
| OS | Windows 10Pro10.0.19045,build19045 |
| Git / CMake | Git2.51.0.windows.1; CMake4.4.4 |
| Compiler | MSVC19.44.35229.0; VisualStudio2022BuildTools |
| Local game EXE | 1,906,688bytes; SHA256`FE3AA1316D634611F3D690665C5D802696BE80DA5124C34B2475D05391CEF1F3` |
| Build | Reported clean Release exit0,warnings-as-errors enabled |
| CTest | Reported20/20passed,8.24seconds; report records30Oct2026,12:13:42-12:13:50IST |
| Python | Reported33/33passed,1.09seconds |
| Focused D6 | Reported4 assertions/1case pass plus isolated runtime warning and zero-byte-file preservation |
| D7 | Reported106model assertions/5cases and title mirror. Actual visible OpenGL client-area layout still needs adequate screenshot/manual record; PrintWindow frame/title alone does not prove rendering. |

Windows20 versus Linux21CTest entries is expected: `secure_write_fault_tests` is guarded by `if(CMAKE_SYSTEM_NAME STREQUAL "Linux")` in the pinned CMakeLists. Native locked-file tests are distinct from Linux LD_PRELOAD injection. Do not require Windows to report21entries or pretend native crash injection was run.

## Owner acceptance versus detailed manual results

Dinesh explicitly reports personally playing on Windows 10 and that everything works; this is owner functional acceptance. Implementation issues15,16,17,19,20,21 were closed with that attestation and existing automated evidence. It is not a fabricated record of each prescribed manual observation or threshold.

The user specifically reported hearing background sound without the volume settings recorded. Later Antigravity text claimed independent-channel/F5 passes without the corresponding explicit observations in the supplied record. Keep those detailed listening checks Pending until actual observations are supplied. At SFX0,F5 silence is expected. Available themes are Classic/Neon/Retro,not Dark.

## Requested corrections and received follow-up

- D1=date validation; D2=>10-entry rejection; D3=schema-corrupt next-save recovery; D4=YYYY-MM-DD serialization; D5=24-hour best-effort temp/orphan cleanup; D6=empty/whitespace warning; D7=six-column table. Do not reuse historical incorrect labels.
- Use "no confirmed failures in executed checks",not an exhaustive zero-defects certification.
- User relayed corrected Windows corpus totals729assertions/21security cases and362assertions/3settings cases. These are collaborator-reported platform-specific totals; supporting final raw logs must govern. Do not copy Linux732assertions onto Windows or retain stale98/100claims.
- Reported warm launch bounds around811-838ms are diagnostic; no verified cold-cache/new-game10run acceptance or3x300sFPS/100input/volunteer campaign supplied.
- Windows 11 remains Not tested. Earlier historical Windows 11 privacy sampling does not establish candidate7c portability/privacy.
- Baseline Win10report does not cover new PR #60audio commit09bbf96.

## Evidence to preserve/review (relative names only)

`07-ctest-fresh.txt`,`08-system-snapshot.txt`,`09-d6-unit-verbose.txt`,`10-d7-column-evidence.txt`,`11-d6-runtime-warning.txt`,`12-d7-synthetic-navigation.txt`,`screenshots/leaderboard-six-columns.png`,final corrected`WINDOWS10-QA-REPORT.md` and actual build manifest. Preserve original failed/misclassified report text with corrections rather than silently replacing history.

## Privacy provenance

Historical redacted capture exists at audit branch commit`d59d85e2e028a244fee71d8b95c5e2e77e6c8b66`,not baseline main; redacted JSON hash`0a2ac44068c9fecf7c288d2f1fbc9023026bb06598f35b7700681ed439c47dc4`. It records60samples,zero observed process-ownedTCP/UDP and ActiveStore checks at endpoints;104.7soverall,98.35ssample span. It lacks exact source/EXE identity. Sampling is not a packet capture or a universal no-network proof. Final exact-candidate approved firewall/offline evidence remains Pending.

<!-- Evidence reconciliation: Windows performance and grid review -->

### Evidence reconciliation — performance and grid review

**Disposition: Incomplete / pending acceptance.** The reviewed Windows performance archive contains 32 entries (71,558 bytes; SHA-256 `150317713668114380ac058700c50df177d6c55772ba93504b84ba55a49393c5`). Its manifest identifies source/EXE revision `09bbf96` and EXE SHA-256 `780B5AB208B7FD6DF91916E4E386848E179FE4B71CBA27BC4BCEE60207D2E946`. The unchanged analyzer from merged revision `3fcc586` accepts the schema and timestamps in all 11 supplied raw probe JSON files. This validates the supplied traces and their format; it does not independently establish Windows execution, binary provenance, or target-machine conditions.

Manual FPS traces contain only 3-, 1-, and 5-second continuous Hard/Obstacle segments. The eligible windows reported are respectively 4 (mean 60.25 FPS, minimum 60), 1 (45 FPS), and 10 (mean 59.8 FPS, minimum 59); no eligible window below 30 FPS was observed. Each of the three original automated attempts had only one qualifying second, while the idle-menu run had no gameplay. These observations do **not** meet the required three continuous 300-second sessions and do not justify a waiver.

The input trace records 100 directions in the requested order, with 18 accepted, 82 rejected, and zero drops. The reported 1–6 microsecond registration interval is an internal dequeue-to-registration measurement, not an OS/SFML queue-inclusive input-acceptance measurement. Launch times of 3.007, 1.179, and 1.321 seconds remain cache-unknown/warm diagnostics; cold-launch acceptance is unverified.

The owner/reviewer-reported grid-results JSON retains `all_passed: true` / `passed: true` for title-based 4–16 ms timing. That result is not proof of first-grid-render acceptance; the corrected report leaves first render pending. The reported `tc_core_01_grid_snake_frame.png` was visually inspected and depicts a conversation rather than the game grid, so it cannot support TC-CORE-01 rendering. The referenced grid artifacts were not found in this checkout for independent reinspection. Preserve the historical report and results, document their scope, and do not silently substitute a replacement capture.

Issue #22 remains In Progress and issue #25/final QA sign-off remain pending. No confirmed performance product defect was established by these short runs, but required acceptance remains unproven. The reported volunteer attestation is retained with build/route unknown. This reconciliation does not approve a waiver, close issues, or authorize release.

### Addendum — subsequent performance, grid, and functional evidence review (2026-10-31)

This addendum records two later-reviewed evidence captures separately from the historical 71,558-byte archive documented above. The raw files for these two captures are not present in this checkout. Their identifiers, sizes, and review observations below are recorded from the supplied review information; they have not been independently rehashed or reproduced during this documentation update.

| Evidence record | Reported size | Supplied identifier | Review disposition |
|---|---:|---|---|
| Subsequent performance archive | 1,028,500 bytes | `5c63ca97...` | Separate later capture; do not merge its results into the historical 71,558-byte archive record. |
| Subsequent evidence batch | 1,420,509 bytes | `6ede6694...` | Separate batch; its reported 45-file manifest validation does not imply that all observations passed. |

**Performance scope:** The batch review found zero complete qualifying Hard/Obstacle windows in its two new probes. These probes do not satisfy the required three continuous 300-second Hard/Obstacle campaigns with power-ups active. Preserve the reported probe results as diagnostic evidence only. Performance acceptance remains Pending.

**Grid scope:** All ten grid observations in the reviewed batch were inconclusive or skipped. The box-loop observation has no obstacle-pixel feedback. These observations do not establish first-grid-render timing, correct obstacle rendering, or TC-CORE-01 acceptance.

**Functional evidence corrections:**

- The reported nine-combination array is empty; it does not establish successful coverage of the nine required combinations.
- The 16-character and invalid-character assertions were executed on Mode Selection. They do not establish the corresponding settings-screen validation behavior.
- `settings_saved=false` does not establish successful settings persistence.
- `cancel_preserved_music=false` does not establish that Cancel preserved the previous music setting.
- A no-crash observation establishes only that no crash was observed in that run; it does not prove that a requested theme loaded or rendered correctly.
- Retain the valid pause/resume/restart observations and corrupt-board observations within their demonstrated scope.
- Retain the genuine six-column image with its actual provenance. Do not substitute a different image or claim an unverified build association.

The manifest's reported validation of 45 files is a batch-integrity observation, not a blanket pass for the individual probes or functional checks. Preserve individual results and limitations rather than promoting the supplied batch summary wholesale to Passed.

The two new captures do not replace, relabel, or retrospectively extend the historical 71,558-byte archive. The historical 3-, 1-, and 5-second Hard/Obstacle segments remain short diagnostic runs and do not satisfy the required 300-second campaigns.

**Disposition:** Evidence review only; acceptance remains incomplete. Issue #22 remains In Progress, issue #25 remains open, and final QA/release sign-off remains Pending. This addendum approves no waiver and authorizes no release.

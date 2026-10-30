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

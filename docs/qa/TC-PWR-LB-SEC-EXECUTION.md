# Test execution: power-ups, leaderboard and security (issue #18)

STP v1.0 test cases TC-PWR-01…05, TC-LB-01…04 and TC-SEC-01…06, plus the acceptance criteria of issue #18. Executed by B V Tejas.

The cases were run twice:

| Run | Date | Build under test |
| --- | --- | --- |
| 1 | 29 Oct 2026 | All open pull requests merged locally: `main` `df9b5cc`, #49 `21168fd`, #51 `66ddaa4`, #46 `51fd38f`, #50 `07d37d2`, #52 `8d6acd2`. The leaderboard was not yet in the game. |
| **2 (current)** | 29 Oct 2026 | **The integrated candidate #53 at `dbf08b5`** (all Sprint 2 branches, including #49, #50, #51, this pull request at `8d6acd2` and the leaderboard UI of #21), plus this pull request's test update at `12441e5`. The test update adds or changes only test, tool and report files: no product code. |

Unless a row says otherwise, every result below is from run 2.

**Environment.** An Ubuntu 24.04 container:
- GCC 13.3, Release, `-Werror`: the game and all tests.
- Clang 18, Debug, AddressSanitizer + UndefinedBehaviorSanitizer, `-Werror`: the game and all tests, run as a normal user, so the permission tests are real.
- The real game ran on Xvfb with XTEST key and mouse events.

**Not covered here:** real Windows 10, Windows 11 and macOS machines. CI ran the unit and integration suites on Windows Server (MSVC, MinGW) and macOS runners. CI is not a manual run on those systems. See "Not executed" below.

## Summary

| Status | What |
| --- | --- |
| **Passed** | TC-PWR-01…05, TC-LB-03, TC-SEC-01, TC-SEC-02, TC-SEC-05, TC-SEC-06. Acceptance criteria 2 (64 KiB + 1) and 3 (timers, multiplier). Every TC-SEC-04 check except the stale-temporary-file cleanup (D5) |
| **Remaining failures** | TC-LB-01 (D4), TC-LB-02 (D7), TC-LB-04 (D2, D3, D6), TC-SEC-03 (D1, D2), TC-SEC-04 (D5), acceptance criterion 1 (D1, D2). The six known-defect tests for D1–D5 still fail as expected, so they remain acceptance blockers. D5 has two tests: a simulated leftover and an injected crash. D6 and D7 are new in run 2 |
| **Blocked** | Turning the six known-defect tests into ordinary regression tests, removing the two corpus exceptions, and re-running on a fixed candidate. All of these wait for the persistence fixes (Bharath) |
| **Not executed** | Manual runs on real Windows 10/11 and macOS machines. Crash injection on Windows and macOS (the injector is Linux-only). Audio is not involved in these cases |

All 21 CTest suites of the candidate pass with GCC and again under Clang ASan + UBSan as a normal user. Expected failures are counted as passes by Catch2's `[!shouldfail]`, so the suites are green while D1–D5 exist. cppcheck reports 0 findings and the banned-API scan finds 0 hits in 46 files ([evidence](evidence/issue-18-candidate-ctest-sanitizers.txt)).

## Acceptance criteria (#18)

| # | Criterion | Result |
| --- | --- | --- |
| 1 | 25 valid + 25 malformed JSON corpus executed | **Fail (D1, D2).** Executed: 98 of 100 files behave as specified. `leaderboard.json`: all 25 valid files load exactly as written; 23 of 25 malformed files fall back to an empty board, but 2 are accepted (D1, D2). The test lists those 2 as known exceptions until they are fixed. `config.json`: 25/25 valid load as written; 25/25 malformed fall back to the defaults (7 × `E_PARSE`, 18 × `E_SCHEMA`). The slowest load took 1 ms. No sanitizer report |
| 2 | 64 KiB + 1 byte boundary | **Pass** for both files. 65,536 bytes loads. 65,537 bytes is refused (`TooLarge` / `E_FILE_TOO_LARGE`) and the board or settings fall back to defaults. A 256 MiB file is refused in under 1 s: the reader stops after 64 KiB plus one 4 KiB block |
| 3 | Power-up timer expirations and multiplier calculations | **Pass**, in the unit and system tests and in the real game, with both builds. See TC-PWR-01…05 |

## Results per test case

| Test case | Levels run | Result | What was checked |
| --- | --- | --- | --- |
| TC-PWR-01 spawn interval | Unit, system, runtime | **Pass** | 10 simulated minutes per seed: every gap 8–15 s, on a free cell, one at a time, gone after exactly 5 s. Real game: power-ups appeared 8.0–13.7 s apart. The harness cannot see an item under the HUD corner panels, so one 17.2 s gap (Release run) contains one item it could not see |
| TC-PWR-02 Speed 5 s | Unit, system, runtime | **Pass** | Medium: 72 ms for 5.0 s, then 120 ms. The title and HUD show the seconds left. Real game, Release and ASan: 71.8 ms over 29 moves, the boost ends after 5.07–5.10 s (observer latency included), then 119.7–120.5 ms |
| TC-PWR-03 ×2 for 10 s | Unit, system, runtime | **Pass** | Food at 1 s, 9.9 s and 10.1 s gives +20, +20, +10. Real game: +20 for each of 14 (Release) and 20 (ASan) foods eaten under ×2, +10 for 25 and 21 foods without it. Pause freezes the time left (10.0 s before and after 3 s of pause) |
| TC-PWR-04 Shrink | Unit, system, runtime | **Pass** | Lengths 6, 4, 3 become 5, 3, 3. Real game: 39→38, 41→40, 40→39 (Release); 37→36, 40→39 (ASan) |
| TC-PWR-05 one effect | Unit, system | **Pass** | Speed ends the moment a Multiplier is collected. No speed change when the Multiplier expires. Speed works again afterwards |
| TC-LB-01 persist Top 10 | Integration, app | **Fail (D4)** | Real game: 5 games with different modes, difficulties and scores (Classic/Easy 30, Timed/Medium 10, Obstacle/Hard 15, Classic/Hard 0, Obstacle/Easy 45). `leaderboard.json` is valid JSON, sorted 45, 30, 15, 10, 0. Each entry has exactly the 5 Appendix B fields and matches the game played. After a restart the Leaderboard shows the same 5 rows, and viewing does not rewrite the file. But `date_achieved` is `2026-10-29T09:09:14Z`, not `YYYY-MM-DD` (D4) |
| TC-LB-02 view leaderboard | System | **Fail (D7)** | 10-entry board, stored unsorted, with all modes and difficulties: Leaderboard opens from the main menu, 10 rows in descending score order, ties keep their stored order, and Esc returns to the menu. Empty board: "No scores yet", no warning, and Esc returns to the menu. But the screen shows only Rank, Player name and Score. The STP also requires mode, difficulty and date (D7). [Screenshot](evidence/screenshots/lb02-ten-entries.png) |
| TC-LB-03 Top-10 insert | Unit | **Pass** | On a full board 100…1000: 1001 and 101 are inserted (board stays at 10, lowest dropped). 100 (tie with 10th) and 99 are rejected with the file byte-for-byte unchanged. On a 3-entry board, 1 is accepted at rank 4 |
| TC-LB-04 missing/corrupt | Integration, app | **Fail (D2, D3, D6)** | Real game, one launch per fixture, then one real game with a valid score. **Missing:** empty board, then the score is saved. **Truncated:** empty board, the warning on screen and in the log, then the score is saved and the bad file replaced. **Empty (0 bytes):** empty board and the score is saved, but no warning (D6). **Wrong type, negative score, bad enum:** empty board and the warning, but the score is refused with `E_LEADERBOARD_SAVE` and the bad file is kept (D3). **11 entries:** 10 rows shown with no warning; the score "did not reach the Top 10" and the file is kept (D2). No crash and no temporary file in any case |
| TC-SEC-01 name sanitisation | Unit, system | **Pass** | 14 unsafe inputs (`Ab,cd`, `a"b`, newline, tab, `{"x":1}`, `../x`, NUL, emoji, `ñ`, empty, blanks, `C:\x`, `a;b`, `a|b`) are rejected by the validator and by `addScore`, and nothing is written. Real game: every refused character shows the inline message at once, and the field keeps only allowed characters. Tab inserts nothing, an empty name cannot be confirmed, and `A_b-1 x` is accepted |
| TC-SEC-02 no PII | Inspection | **Pass** | A real session wrote `config.json` (only the 8 Appendix B keys), `leaderboard.json` and a log. None of them contains the user name, host name or an absolute path. `leaderboard.json` holds only `version` and the 5 fields per entry |
| TC-SEC-03 schema and size | Integration | **Fail (D1, D2)** | 98 of 100 corpus files behave as specified, as in acceptance criterion 1. Every load takes under 1 s (maximum 1 ms). Oversize files are refused without a full read. Clean under ASan + UBSan |
| TC-SEC-04 safe write | Integration (fault injection), app | **Fail (D5)**, all else pass | See the next section |
| TC-SEC-05 data, never code | Review, test | **Pass** | Names `%s%n`, `$(calc)`, `<script>` and `${PATH}` in a hand-edited file are rejected, and the board falls back to empty. Hostile text in ignored fields stays inert data. Banned-API scan: 0 hits |
| TC-SEC-06 name buffer | Unit, system | **Pass** | A 10,000-character string: the validator rejects it, sanitising gives ≤ 15 characters, and `addScore` refuses it. Real game: 10,000 typed characters leave 15 and show "Maximum 15 characters." Release build: memory 108,996 → 109,000 kB. The ASan + UBSan game reports no error and no leak. Under ASan, memory rises while typing because ASan keeps freed memory in quarantine; see O5 |

## TC-SEC-04: simulated leftovers, injected faults and real refusals

Three kinds of check, kept apart in the test names:

| Kind | Test file | What it does |
| --- | --- | --- |
| **Simulated leftovers** | `tests/SecurityCorpusTests.cpp`, "TC-SEC-04 (simulated leftovers)…" | The test writes the files a crash would leave (the old board plus an empty, partial or complete `leaderboard.json.tmp.*`). No save runs and nothing crashes. Only the loader is checked |
| **Injected faults** | `tests/SecureWriteFaultTests.cpp` with `tests/fault/` (Linux) | A child process saves a real score through `LeaderboardRepository` → `SecureFileStore::writeJson`, with an `LD_PRELOAD` library that acts only on the temporary file. It either aborts the process (SIGABRT) at an STP point, or makes a system call fail. No product code is changed |
| **Real refusals** | `SecurityCorpusTests.cpp`, "TC-SEC-04 (real refusal)…", and `tools/leaderboard_ui_check.py` | The operating system refuses the save: a read-only folder (Linux and macOS, normal user) or a file locked by another handle (Windows) |

Results of the injected faults. Each starts from a valid 3-entry board; the child tries to add a 400 score. The settings rows only run when the settings module saves through `SecureFileStore`, as in the candidate.

| Injection | Child | After it: `leaderboard.json` | Temporary file | Next save | `config.json` |
| --- | --- | --- | --- | --- | --- |
| Abort after the temporary file is opened (a) | killed | old board, byte for byte | left (empty) | works, all scores kept | old, byte for byte |
| Abort after writing, before the flush (b) | killed | old board, byte for byte | left (complete) | works | old |
| Abort after the flush, before the rename (c) | killed | old board, byte for byte | left (complete) | works | old |
| Abort just after the rename (d) | killed | new board (4 entries) | none | works | new |
| `write()` fails with ENOSPC | error 3 | old board, byte for byte | removed | – | old, byte for byte |
| `fsync()` fails with EIO | error 3 | old board, byte for byte | removed | – | old |
| `rename()` fails with EACCES | error 3 | old board, byte for byte | removed | – | old |
| Same three failures on a first save (no file yet) | error 3 | no file created | removed | – | – |
| No fault (control) | saved | new board | none | – | – |

A crash never leaves a partial, empty or invalid board, and a failed save always reports the error and keeps the old file. **D5:** the temporary file of a crashed save is never removed, even an hour later, after the next load and a successful save.

Real refusals and safety:
- **Read-only folder, repository level:** the save returns an error and the old board stays byte for byte. This also runs in CI on Linux and macOS as a normal user, and as a locked file on Windows (MSVC, MinGW).
- **Read-only folder, real game:** run as a normal user with `tools/leaderboard_ui_check.py` on both builds. The game-over screen shows "E_LEADERBOARD_SAVE: score not saved" and the log has `[WARN] E_LEADERBOARD_SAVE`. The Leaderboard still shows the old 3 rows, `leaderboard.json` is unchanged byte for byte, no temporary file is left, and the game exits normally. [Screenshot](evidence/screenshots/sec04-refused-save.png)
- **Safety for the D5 fix:** loads and saves never remove a file that is not their own temporary file, and never touch what a symbolic link points to. Checked files: `.bak`, `leaderboard.json.tmp` without a nonce, another store's `config.json.tmp.*`, `leaderboard.jsonx.tmp.*`, a directory named `leaderboard.json.tmp.777`, and a symbolic link to a file outside. This passes today and keeps passing only if the cleanup stays this narrow.

## Defects

All D1–D5 still reproduce on candidate `dbf08b5`. D6 and D7 are new in run 2.

| ID | Component (owner) | Requirement | Observed | Expected | Reproduce |
| --- | --- | --- | --- | --- | --- |
| D1 | C11 leaderboard validation (Bharath) | SAD 4.3.5, SRS Appendix B | `date_achieved` is only checked for being non-empty: `"28/10/2026"` loads | Not `YYYY-MM-DD` → schema violation → empty board | `malformed/23-bad-date-format.json`; test "Known defect D1" |
| D2 | C11 (Bharath) | SAD 4.3.5 "at most 10 entries", TC-LB-04 | A board with 11 entries loads and shows 10. No warning. The next score is judged against it and the file is kept | Schema violation → empty board, warning, next valid score stored | `malformed/24-eleven-entries.json`; test "Known defect D2"; `leaderboard_ui_check.py` |
| D3 | C11 (Bharath) | TC-LB-04 step 2 | After a schema-invalid file (wrong type, negative score, bad enum), `addScore` returns `Error` ("E_LEADERBOARD_SAVE: score not saved" in the game) and keeps the bad file | The valid score is stored and the bad file replaced atomically, as already happens for malformed JSON | `malformed/14`, `15`, `18`; test "Known defect D3"; `leaderboard_ui_check.py` |
| D4 | C11 (Bharath) | SRS Appendix B "Date (YYYY-MM-DD)", TC-LB-01 | New entries get `date_achieved` like `2026-10-29T09:09:14Z` | `2026-10-29` | Test "Known defect D4"; `leaderboard_ui_check.py` |
| D5 | C13 secure file store (Bharath) | SAD 4.3.4 `removeStaleTemp`, TC-SEC-04 | `leaderboard.json.tmp.*` left by a crashed save stays forever | Removed on a later load or save, without touching anything else | Tests "Known defect D5 (simulated leftover)" and "Known defect D5 (injected crash)" |
| D6 | C11 (to triage) | TC-LB-04 "warning logged" | An empty (0-byte) or whitespace-only `leaderboard.json` is read as a new board: nothing on screen and nothing in the log. Truncated or invalid files do show and log `E_LEADERBOARD_READ` | The same warning as for other corrupt files | `leaderboard_ui_check.py`, "TC-LB-04 empty"; `malformed/01`, `02`. No known-defect unit test yet: the warning API (`top10(std::string*)`) exists only in #53, not in this pull request's base |
| D7 | Leaderboard screen, #21 (to triage) | STP TC-LB-02 | The table has Rank, Player name and Score only | Rank, name, score, mode, difficulty and date | [lb02-ten-entries.png](evidence/screenshots/lb02-ten-entries.png) |

### Known-defect tests

| Test | File | When its defect is fixed |
| --- | --- | --- |
| Known defect D1: a date_achieved that is not YYYY-MM-DD is accepted | `SecurityCorpusTests.cpp` | Remove `[!shouldfail]`, rename it to the behaviour, and remove `23-bad-date-format.json` from the corpus exceptions |
| Known defect D2: a board with more than 10 entries is accepted | `SecurityCorpusTests.cpp` | Same, with `24-eleven-entries.json` |
| Known defect D3: a schema-invalid board is not replaced by the next valid score | `SecurityCorpusTests.cpp` | Remove `[!shouldfail]` and rename it |
| Known defect D4: date_achieved is written as a timestamp, not YYYY-MM-DD | `SecurityCorpusTests.cpp` | Remove `[!shouldfail]` and rename it |
| Known defect D5 (simulated leftover): a stale temporary file is never removed | `SecurityCorpusTests.cpp` | Remove `[!shouldfail]` and rename it. If the agreed cleanup is age-based, its leftover file must first be made older than the threshold (setup only; the assertion stays) |
| Known defect D5 (injected crash): a crashed save's temporary file is never removed | `SecureWriteFaultTests.cpp` | Remove `[!shouldfail]` and rename it |

Each test passes while its defect exists and fails once the defect is fixed. Its assertions describe the specified behaviour, so they stay unchanged. Once all of D1–D5 are fixed, the leaderboard corpus test checks all 25 valid and 25 malformed files with no exceptions.

### Observations

- **O1 (updated).** The candidate now reports corrupt boards on the Leaderboard screen ("Leaderboard unavailable: empty table shown") and in the log (`[WARN] E_LEADERBOARD_READ`). A failed save shows `E_LEADERBOARD_SAVE` on the game-over screen and logs it. The gaps are D2 and D6.
- **O2.** STP TC-SEC-03 lists "extra fields" as malformed, but SAD 4.3.5 says "unknown keys are ignored". The corpus follows the SAD: valid files 13, 14, 22 and 25 carry unknown keys.
- **O3 (resolved).** Run 1 had no way to crash a real save. The `LD_PRELOAD` injector now does this without any product change, but only on Linux.
- **O4 (resolved).** Run 1 could not run the app-level steps of TC-LB-01, TC-LB-02, TC-LB-04 and TC-SEC-02. They ran on the candidate.
- **O5.** In the ASan + UBSan game, memory rises by about 29 MB while 10,000 characters are typed, so the security UI check's memory line fails there. ASan holds freed memory in a quarantine of up to 256 MB, so this does not point to a leak. With the quarantine off (`quarantine_size_mb=0`), the same build changes by 16 kB and passes all 23 checks. The Release game changes by 4 kB, and LeakSanitizer reports no leak at exit.

## Not executed

| Check | Why | Needed |
| --- | --- | --- |
| TC-LB-02 and the other system cases by hand on real Windows 10, Windows 11 and macOS | No such machines in this environment. CI covers unit and integration suites on Windows Server and macOS runners, which is not a manual run | A team member with each machine. The Windows 10/11 checklist is in `docs/quality-gates.md` (#51) |
| TC-SEC-04 crash injection on Windows and macOS | `LD_PRELOAD` and `fork`/`execve` are Linux-only | The Windows locked-file refusal runs in CI |
| TC-SEC-04 real refusal at app level on Windows | Linux only (read-only folder) | – |

## Reproduce

```sh
# Unit, integration and fault-injection suites
cmake -S . -B build -DSNAKE_BUILD_APP=OFF -DSNAKE_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/security_corpus_tests "[.evidence]"          # per-file corpus table
./build/secure_write_fault_tests -d yes               # Linux: every injection

# The real game (Linux, Xvfb + xdotool + Pillow), on a build with the app
python3 tools/security_ui_smoke.py "$PWD/build-app/SnakeXtreme"
python3 tools/leaderboard_ui_check.py "$PWD/build-app/SnakeXtreme" out/leaderboard   # as a normal user for the TC-SEC-04 part
python3 tools/powerup_runtime_check.py "$PWD/build-app/SnakeXtreme" out/powerups 150
```

For the sanitizer configuration, use the configure line in `docs/quality-gates.md` (#51).

## Evidence files

- [issue-18-candidate-ctest-sanitizers.txt](evidence/issue-18-candidate-ctest-sanitizers.txt): run 2, all 21 suites with GCC and with ASan + UBSan as a normal user, per-suite counts, cppcheck and the banned-API scan.
- [issue-18-fault-injection.txt](evidence/issue-18-fault-injection.txt): run 2, every injected crash and failure.
- [issue-18-leaderboard-ui.txt](evidence/issue-18-leaderboard-ui.txt): run 2, TC-LB-01/02/04 and TC-SEC-04 in the real game, Release and ASan + UBSan.
- [issue-18-powerups-runtime.txt](evidence/issue-18-powerups-runtime.txt): run 2, power-ups in the real game, Release and ASan + UBSan.
- [issue-18-ui-security.txt](evidence/issue-18-ui-security.txt): TC-SEC-01/02/06 in the real game, runs 1 and 2.
- [issue-18-desktop-smokes.txt](evidence/issue-18-desktop-smokes.txt): run 2, the candidate's own leaderboard, navigation and settings smoke tests.
- [issue-18-leaderboard-corpus.txt](evidence/issue-18-leaderboard-corpus.txt) and [issue-18-config-corpus.txt](evidence/issue-18-config-corpus.txt): every corpus file with its result and load time (run 2).
- [issue-18-integration-ctest-sanitizers.txt](evidence/issue-18-integration-ctest-sanitizers.txt): run 1, all 19 suites under ASan + UBSan.
- [screenshots/](evidence/screenshots/): the Leaderboard screen with 10 entries, empty, after a corrupt file and after 11 entries, plus the refused save.

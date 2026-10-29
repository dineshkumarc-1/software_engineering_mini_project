# Test execution: power-ups, leaderboard and security (issue #18)

STP v1.0 test cases TC-PWR-01…05, TC-LB-01…04 and TC-SEC-01…06, plus the acceptance criteria of issue #18. Executed by B V Tejas on 29 Oct 2026.

## What was tested

**Build under test.** All open work merged together locally, without conflicts:

| Part | Commit |
| --- | --- |
| `main` | `df9b5cc` |
| #49 power-ups | `21168fd` |
| #51 quality gates | `66ddaa4` |
| #46 UI, settings, audio, probes (includes #44 and #45) | `51fd38f` |
| #50 persistence and fuzz tests (includes #42 and #48) | `07d37d2` |
| this pull request | |

**Environment.** An Ubuntu 24.04 container:
- GCC 13.3, Release, `-Werror`.
- Clang 18, Debug, AddressSanitizer + UndefinedBehaviorSanitizer, `-Werror`. Run as a normal user, so permission tests are real.
- The real game ran on Xvfb with XTEST key events.

**Windows and macOS:** not run by hand. CI on this pull request ran `security_corpus_tests` and passed on Linux GCC, macOS Clang, Windows MSVC and Windows MinGW-w64, including the Windows locked-file case. `settings_corpus_tests` joins those jobs once #45/#46 is merged, and the #51 sanitizer job covers everything once #51 is merged.

**New in this pull request:**
- The JSON corpus in `tests/fixtures/json-corpus/`: 25 valid + 25 malformed `leaderboard.json` files and 25 valid + 25 malformed `config.json` files.
- Two suites: `security_corpus_tests` (leaderboard and security) and `settings_corpus_tests` (config; built once the settings module from #20 is merged).
- The UI check `tools/security_ui_smoke.py`.
- This report and its evidence files.

## Acceptance criteria (#18)

| # | Criterion | Result |
| --- | --- | --- |
| 1 | 25 valid + 25 malformed JSON corpus executed | **Executed, 98 of 100 files behave as specified.** `leaderboard.json`: all 25 valid files load exactly as written; 23 of 25 malformed files fall back to an empty board, 2 are accepted (defects D1, D2). `config.json`: 25/25 valid load as written; 25/25 malformed fall back to the defaults (7 × `E_PARSE`, 18 × `E_SCHEMA`). The slowest load took 2 ms. No sanitizer report |
| 2 | 64 KiB + 1 byte boundary | **Pass** for both files. 65,536 bytes loads. 65,537 bytes is refused (`TooLarge` / `E_FILE_TOO_LARGE`) and the board or settings fall back to defaults. A 256 MiB file is refused in under 1 s: the reader stops after 64 KiB plus one 4 KiB block, so it never reads the file fully |
| 3 | Power-up timer expirations and multiplier calculations | **Pass.** See TC-PWR-01…05 below and [docs/powerups.md](../powerups.md) |

## Results per test case

| Test case | Level run | Result | What was checked |
| --- | --- | --- | --- |
| TC-PWR-01 spawn interval | Unit, system, runtime | **Pass** | 10 simulated minutes per seed: every gap 8–15 s, on a free cell, one at a time, gone after exactly 5 s. Real game: 13 power-ups, 8.6–12.9 s apart |
| TC-PWR-02 Speed 5 s | Unit, system, runtime | **Pass** | Medium: 72 ms for 5.0 s, then 120 ms; the HUD shows the seconds left. Real game: 71.5–72.4 ms over 29 moves, ends after ~5.1 s (observer latency included), then 119–120 ms |
| TC-PWR-03 ×2 for 10 s | Unit, system, runtime | **Pass** | Food at 1 s, 9.9 s and 10.1 s: +20, +20, +10. In play at 0.96 s, 9.96 s and 10.08 s: the same. Real game: +20 for each of 3 foods during ×2, +10 for 19 foods without |
| TC-PWR-04 Shrink | Unit, system, runtime | **Pass** | Lengths 6, 4, 3 become 5, 3, 3; the power-up is consumed. Real game: −1 on each of 5 pick-ups |
| TC-PWR-05 one effect | Unit, system | **Pass** | Speed ends the moment a Multiplier is collected. No speed change when the Multiplier expires. Speed works again afterwards |
| TC-LB-01 persist Top 10 | Integration (repository) | **Fail (D4)** | 5 games with different modes and difficulties are stored sorted, each entry with exactly the 5 Appendix B fields, and survive a restart. But `date_achieved` is written as `2026-10-29T03:40:12Z`, not `YYYY-MM-DD`. The app-level step is blocked: the game does not submit scores yet (#21) |
| TC-LB-02 view leaderboard | System | **Blocked** | The Leaderboard screen in #46 is still the placeholder "Persistence integration pending (#21)". Its Back button works |
| TC-LB-03 Top-10 insert | Unit | **Pass** | On a full board 100…1000: 1001 and 101 are inserted (board stays at 10, lowest dropped); 100 (tie with 10th) and 99 are rejected with the file byte-for-byte unchanged. On a 3-entry board, 1 is accepted at rank 4 |
| TC-LB-04 missing/corrupt | Integration | **Fail (D2, D3)** | Missing, empty and truncated files: empty board, then the next valid score is stored. Wrong type, negative score and bad enum: empty board, but the next valid score is refused (D3). 11 entries: 10 are shown instead of an empty board (D2). "Warning logged": see O1 |
| TC-SEC-01 name sanitisation | Unit, system | **Pass** | 14 unsafe inputs (`Ab,cd`, `a"b`, newline, tab, `{"x":1}`, `../x`, NUL, emoji, `ñ`, empty, blanks, `C:\x`, `a;b`, `a|b`) are rejected by the validator and by `addScore`, and nothing is written. Real game: every refused character shows the inline message at once and the field keeps only allowed characters. Tab inserts nothing, an empty name cannot be confirmed, and `A_b-1 x` is accepted |
| TC-SEC-02 no PII | Inspection | **Pass** | A real session wrote only `config.json` (the 8 Appendix B keys) and its log. Neither contains the user name, host name or any absolute path. `leaderboard.json` as written by the repository holds only `version` and the 5 fields per entry. The game does not write `leaderboard.json` itself yet (#21) |
| TC-SEC-03 schema and size | Integration | **Fail (D1, D2)** | 98 of 100 corpus files as specified, as in acceptance 1. Every load ≤ 1 s (max 2 ms). Oversize files are refused without a full read. Clean under ASan + UBSan |
| TC-SEC-04 safe write | Integration | **Partial, Fail (D5)** | There is no test hook that aborts the process inside `SecureFileStore` (O3), so states (a)–(c) were reproduced as the files a crash leaves: the old file plus an empty, partial or complete temporary file. The old board loads each time. (d) After the rename, the new board loads. A successful save leaves no temporary file. A locked target (read-only directory on Linux as a normal user; an exclusive lock on Windows in CI) keeps the old file and returns an error. A stale temporary file is never removed (D5), and the error is not logged (O1) |
| TC-SEC-05 data, never code | Review, test | **Pass** | Names `%s%n`, `$(calc)`, `<script>` and `${PATH}` in a hand-edited file are rejected, so the board falls back to empty. Hostile text in ignored fields stays inert data. Banned-API scan (#51): 0 hits |
| TC-SEC-06 name buffer | Unit, system | **Pass** | A 10,000-character string: the validator rejects it, sanitising gives ≤ 15 characters, and `addScore` refuses it. The `NameEntry` unit test (10,000 inputs → 15) passes under ASan + UBSan. Real game: 10,000 typed characters leave 15 and show "Maximum 15 characters."; memory stays at 108,840 kB before and after. The ASan + UBSan build of the game reports nothing |

All 19 CTest suites of the integration build pass with GCC, and again under Clang ASan + UBSan as a normal user ([evidence](evidence/issue-18-integration-ctest-sanitizers.txt)).

## Defects raised

All are in C11/C13 persistence, owner Bharath (PR #48 / #50). Each has a "Known defect" test in `tests/SecurityCorpusTests.cpp`.

| ID | Requirement | Observed | Expected | Reproduce |
| --- | --- | --- | --- | --- |
| D1 | SAD 4.3.5, SRS Appendix B | `date_achieved` is only checked for being non-empty: `"28/10/2026"` loads | A date that is not a valid `YYYY-MM-DD` is a schema violation, so defaults are used | `malformed/23-bad-date-format.json` |
| D2 | SAD 4.3.5 ("at most 10 entries"), STP TC-LB-04 | A board with 11 entries loads and shows 10 | Schema violation, so an empty board | `malformed/24-eleven-entries.json` |
| D3 | STP TC-LB-04 step 2 | After a schema-invalid file (wrong type, negative score, bad enum), `addScore` returns `Error` and keeps the bad file | The valid result is stored and the bad file replaced atomically, as already happens for malformed JSON | `malformed/14`, `15`, `18` |
| D4 | SRS Appendix B ("Date (YYYY-MM-DD)", 10 characters), STP TC-LB-01 | New entries get `date_achieved` like `2026-10-29T03:40:12Z` | `2026-10-29` | Test "Known defect D4" |
| D5 | SAD 4.3.4 (`removeStaleTemp`), STP TC-SEC-04 | `leaderboard.json.tmp.*` files left by a crash stay forever | Removed on the next load or save | Test "Known defect D5" |

The known-defect tests are tagged `[!shouldfail]`. They pass while the defect exists, so CI stays green. Once a defect is fixed, its test fails, and the tag should then be removed.

### Observations

- **O1.** No warning is logged for a corrupt leaderboard (TC-LB-04) or a locked target (TC-SEC-04). Only `addScore` returns a warning text, and only for malformed JSON. Once #21 shows the leaderboard, its UI or log should report these.
- **O2.** STP TC-SEC-03 lists "extra fields" as malformed, but SAD 4.3.5 says "unknown keys are ignored". The corpus follows the SAD: valid files 13, 14, 22 and 25 carry unknown keys.
- **O3.** True fault injection for TC-SEC-04 (abort the process after temp opened, written, flushed, renamed) needs a test hook in `SecureFileStore::writeJson`. Adding one is recommended.
- **O4.** TC-LB-02 and the app-level steps of TC-LB-01 and TC-SEC-02 wait for the leaderboard UI integration (#21). They should be re-run then.

## Reproduce

```sh
cmake -S . -B build -DSNAKE_BUILD_APP=OFF -DSNAKE_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/security_corpus_tests "[.evidence]"     # per-file corpus table
python3 tools/security_ui_smoke.py "$PWD/build-app/SnakeXtreme"   # Linux, Xvfb + xdotool, needs the #19 UI
```

For the sanitizer run, use the configure line in `docs/quality-gates.md` (#51).

## Evidence files

- [issue-18-leaderboard-corpus.txt](evidence/issue-18-leaderboard-corpus.txt): every `leaderboard.json` fixture with its read status, entries loaded and load time.
- [issue-18-config-corpus.txt](evidence/issue-18-config-corpus.txt): every `config.json` fixture with its result and warning code.
- [issue-18-integration-ctest-sanitizers.txt](evidence/issue-18-integration-ctest-sanitizers.txt): all 19 suites under Clang ASan + UBSan.
- [issue-18-ui-security.txt](evidence/issue-18-ui-security.txt): the real-game checks, Release (22/22) and ASan + UBSan (21/21, before the memory check was added).
- Power-up runtime results are in [docs/powerups.md](../powerups.md) (#49).

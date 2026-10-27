# QA Test Execution Log

**Project:** SnakeXtreme  
**Issue:** #25 — QA Sign-off, Test Summary Report & Documentation  
**Sprint:** Sprint 2 (26–30 October 2026)  
**Tester:** Ayushi Srivastava  
**Status:** Preliminary testing — final sign-off pending

## 1. Test Environment

### Run A — Earlier Headless Test Run

- Operating system: macOS
- Build system: CMake
- Test framework: Catch2
- Build preset: `headless`
- Test branch: `review/issue-16-validator`
- Reported result: 8 passed, 0 failed
- Reported execution time: 1.10 seconds
- Exact tested commit and compiler/tool versions: Not captured in the original record

### Run B — Reproducible Headless Test Run

- Execution date: 27 October 2026
- Operating system: macOS ARM64
- Tested commit: `aa1cc1e22319f018f7dd515e77f70d45d5a3712d`
- Build preset: `headless`
- Compiler: Apple Clang 21.0.1 (`clang-2100.1.1.101`)
- CMake: 4.4.4
- CTest: 4.4.4
- Test framework: Catch2
- Build result: Successful
- CTest result: 7 passed, 0 failed
- Total CTest execution time: 2.70 seconds

Environment and raw output files:

- `docs/qa/evidence/headless-environment-aa1cc1e.txt`
- `docs/qa/evidence/ctest-output-aa1cc1e.txt`

The environment capture was recorded at 17:08:35 IST. This is the environment-capture time, not a separately measured CTest start time.

## 2. Commands Executed

The following commands were executed for Run B at the tested commit:

```bash
cmake --preset headless
cmake --build --preset headless
ctest --preset headless --output-on-failure
```

The CTest output was captured using:

```bash
ctest --preset headless --output-on-failure 2>&1 | tee ctest-output.txt
```

## 3. Results — Run A

The original recorded run reported the following results:

| Registered test | Result |
|---|---|
| `app_state_flow` | Passed |
| `score_service` | Passed |
| `input_validator_tests` | Passed |
| `game_session` | Passed |
| `game_engine_tests` | Passed |
| `game_mode_tests` | Passed |
| `gameplay_tests` | Passed |
| `layering_core_is_sfml_free` | Passed |

- Total registered CTest tests: 8
- Passed: 8
- Failed: 0
- Reported pass rate: 100%
- Reported execution time: 1.10 seconds

The exact tested commit and tool versions were not captured for this run.

## 4. Results — Run B

The new CTest run at commit `aa1cc1e22319f018f7dd515e77f70d45d5a3712d` reported:

| Registered test | Result |
|---|---|
| `app_state_flow` | Passed |
| `score_service` | Passed |
| `game_session` | Passed |
| `game_engine_tests` | Passed |
| `game_mode_tests` | Passed |
| `gameplay_tests` | Passed |
| `layering_core_is_sfml_free` | Passed |

- Total registered CTest tests: 7
- Passed: 7
- Failed: 0
- Pass rate: 100%
- Total execution time: 2.70 seconds

The two runs have different registered test inventories. In particular, `input_validator_tests` appears in Run A but not Run B. These results must remain separate; the later run does not establish that the validator test was executed at the newer commit.

## 5. Preliminary Findings

- The headless configuration and build succeeded for Run B.
- All seven CTest tests registered for Run B passed.
- All eight tests in the earlier Run A were reported as passing.
- The earlier validator unit tests covered allowed characters, invalid characters, length limits, sanitization, and maximum sanitized output length.
- Searches of the inspected source and header directories on the earlier review branch did not identify calls to `isValidPlayerName()` or `sanitizePlayerName()` outside the validator component itself.
- UI input integration and pre-persistence sanitization remain pending verification.
- These automated results cover only the registered headless CTest tests for their respective runs.

## 6. Outstanding QA Work

The following activities still require execution or verification against the project test plan:

- Complete execution of the planned STP test cases.
- Verify that 100% of planned High-priority test cases pass.
- Verify UI and gameplay integration.
- Verify persistence and leaderboard behaviour.
- Execute performance and input-latency testing.
- Complete security, portability, and applicable static-analysis checks.
- Review outstanding defects and release exit criteria.
- Document implementation deviations against SRS v1.0/v1.1, SAD v1.0, and STP v1.0.
- Download and manually verify the Windows artifact for PR #45, including music/SFX levels, F5 behaviour, Save/Cancel, pause/resume, relaunch persistence, key remapping, and themes.
- Record the exact artifact commit and actual Windows manual-test results.

## 7. Preliminary QA Conclusion

Both recorded headless runs reported successful results for their respective registered CTest inventories. Run A reported 8/8 passing tests; Run B reported 7/7 passing tests at commit `aa1cc1e22319f018f7dd515e77f70d45d5a3712d`.

These results do not establish completion of the full planned test inventory, successful execution of every High-priority test case, or successful Windows manual verification.

**Final QA sign-off remains pending. Issue #25 must remain open until the planned test cases, outstanding integration and manual checks, requirements deviations, defect status, and applicable release exit criteria have been assessed.**
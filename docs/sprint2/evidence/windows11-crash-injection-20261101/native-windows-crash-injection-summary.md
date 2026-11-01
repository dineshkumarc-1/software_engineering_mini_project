# Native Windows Crash-Injection Evidence

Date: 1 November 2026

## Result — review-fix verification

Native Windows TC-SEC-04 process-crash injection and recovery tests passed on Windows 11.

- Focused Windows fault-injection suite: **8/8 test cases passed**.
- Assertions: **206/206 passed**.
- Full configured Windows Release build: **passed**.
- Full Windows CTest suite: **21/21 passed**, 0 failures.
- Full CTest elapsed time: **13.07 seconds**.
- The corrected Windows fault-test target was rebuilt successfully with MSVC and warnings treated as errors.

The eighth test verifies that a fresh crash leftover is retained byte-for-byte through the next repository load and save. The existing 24-hour cleanup policy continues to preserve fresh leftovers.

## Environment

- OS: Microsoft Windows 11 Home Single Language.
- Display version: 25H2.
- OS version: 10.0.26200.
- Build: 26200.9711.
- Architecture: 64-bit.
- PowerShell: 5.1.26100.9711.
- Visual Studio: 2022 Community.
- MSVC: 19.42.34436.0.
- MSVC tools: 14.42.34433.
- Windows SDK: 10.0.22621.0.
- CMake: 4.5.0-rc1.
- Generator: Visual Studio 17 2022.
- Configuration: Release.
- Catch2: 3.8.1.
- SNAKE_BUILD_APP=OFF.
- SNAKE_WARNINGS_AS_ERRORS=ON.

## Source identity

Branch: feature/issue-24-windows-crash-injection

Base commit (pre-review-fix PR head):
36ac9e2bf562c6cdfe546dc5a0611c559b248ea8

Base tree:
dd83fe9661adaf5ee8f2b35fcb817d1056be377b

The review-fix source changes were present in the local working tree when this verification was performed. The exact Git blob hashes of the four relevant source/build files are recorded in run-identity.txt. The three-file review-fix diff had not yet been committed at run-identity capture time.

The review fixes:
- Add the MinGW-only -municode linker option for the Windows child writer's wmain entry point.
- Make the two PE import-inspection pointers const to satisfy cppcheck.
- Add the eighth Windows parity test for preserving a fresh crash leftover across a subsequent load and save.

## Implementation

The test harness consists of:

- WindowsFaultInjection.cpp: test-only Windows API fault injector DLL.
- WindowsFaultWriter.cpp: isolated child process that performs one persistence save.
- SecureWriteFaultWindowsTests.cpp: parent-side Catch2 recovery tests.

The DLL is explicitly loaded by the child writer and patches that process's Import Address Table. The desktop game does not load the DLL.

The injector targets:
- CreateFileW
- WriteFile
- FlushFileBuffers
- MoveFileExW
- CloseHandle

## Injected crash points

1. After temporary-file creation.
2. After writing, before flushing.
3. After flushing, before replacement.
4. After successful replacement.

The child process is terminated at each point. The parent process then checks the persisted data.

## Injected API failures

- WriteFile returns ERROR_DISK_FULL.
- FlushFileBuffers returns ERROR_WRITE_FAULT.
- MoveFileExW returns ERROR_ACCESS_DENIED.

These are simulated API failures, not actual disk-full or hardware failures.

## Recovery checks

The eight Windows Catch2 cases verify:
- The old complete leaderboard remains valid before replacement.
- The new complete leaderboard is valid after replacement.
- Subsequent saves work after injected process termination.
- Failed saves preserve the old data.
- Failed first saves do not leave a destination or temporary files.
- Normal saves work without injection.
- Settings preserve valid old or new data.
- A fresh crash leftover remains present and byte-identical after the next repository load and save.
- Eligible stale leftovers are cleaned up according to the agreed 24-hour policy, using test files aged to 48 hours.

## Results

Focused suite:
8 test cases, 206 assertions, all passed.

Focused test output:
RNG seed: 3849940829
All tests passed (206 assertions in 8 test cases)

Full configured Windows Release build:
Passed.

Full Windows CTest:
21 registered, 21 passed, 0 failed.

Full CTest elapsed time:
13.07 seconds.

## Test artifact SHA-256

Injector DLL:
AB26FEB4F79E29930E4511772094760F167B35DBCB8943F320426C55343C7F3B

Child writer:
10A435073CCE9C9DC7115D85AD36E885664808E595D5E0F8E63891B0EA02539B

Catch2 test executable:
5DF88D505389A163B66C2D5BCA7F93AC473B4E9DB0A3B3F2BFB6AC52126059AC

## Limitations

- This demonstrates native Windows user-mode process-termination and API-failure recovery on Windows 11.
- It does not simulate physical power loss, kernel crashes or hardware failures.
- The simulated ERROR_DISK_FULL result is not a real disk-full test.
- The desktop application was not built in this headless test run.
- The cleanup test advances timestamps on isolated test files; it does not wait 24 real hours.
- Windows 10 and macOS crash-injection coverage are not established by this run.
- The local run used MSVC. The MinGW-specific -municode correction still requires hosted MinGW CI verification.
- Independent owner and team review are still required.

## Privacy

Published logs are redacted copies. Raw machine-specific logs remain local in the ignored build-win-crash directory.
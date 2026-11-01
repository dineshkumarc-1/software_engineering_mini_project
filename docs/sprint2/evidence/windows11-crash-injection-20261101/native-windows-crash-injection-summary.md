# Native Windows Crash-Injection Evidence

Date: 1 November 2026

## Result

Native Windows TC-SEC-04 process-crash injection completed successfully.

- Focused Windows fault-injection suite: 7/7 test cases passed.
- Assertions: 196/196 passed.
- Full Windows CTest suite: 21/21 passed.
- Full CTest elapsed time: 9.68 seconds.
- Clean Release rebuild completed successfully.

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

Base commit:
aef168a2fb679ab86b1c1ca401c4e51cf460f633

Base tree:
4a6e6cb666789c7bfec0be71f9f18b77eb0d1665

The Git blob hashes of the four changed source/build files are recorded in run-identity.txt.

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

The tests verify:
- The old complete leaderboard remains valid before replacement.
- The new complete leaderboard is valid after replacement.
- Subsequent saves work after injected process termination.
- Failed saves preserve the old data.
- Failed first saves do not leave a destination or temporary files.
- Normal saves work without injection.
- Settings preserve valid old or new data.
- Crash leftovers are cleaned up according to the agreed 24-hour policy, using test files aged to 48 hours.

## Results

Focused suite:
7 test cases, 196 assertions, all passed.

Full Windows CTest:
21 registered, 21 passed, 0 failed.

Full CTest elapsed time:
9.68 seconds.

## Test artifact SHA-256

Injector DLL:
142E4EB26250C76D4C64E462A65393627F558B6CF0BA84B02F6C3F867B671586

Child writer:
10A435073CCE9C9DC7115D85AD36E885664808E595D5E0F8E63891B0EA02539B

Catch2 test executable:
411E307A45D444321620F9F8565A4C174F60BB6F81EA5181FCA26BBD292880E8

## Limitations

- This demonstrates native Windows user-mode process-termination and API-failure recovery.
- It does not simulate physical power loss, kernel crashes or hardware failures.
- The desktop application was not built in this headless test run.
- The cleanup test advances timestamps on isolated test files; it does not wait 24 real hours.
- macOS crash-injection coverage remains untested.
- Independent owner and team review are still required.

## Privacy

Published logs are redacted copies. Raw machine-specific logs remain local in the ignored build-win-crash directory.
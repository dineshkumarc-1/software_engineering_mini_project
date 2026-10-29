# Issue #24 — Fuzz Testing and Privacy Audit

Date: 2026-10-28

## 1. Parser fuzz-regression testing

Added deterministic malformed-input tests to
`tests/SecureFileStoreTests.cpp`.

Coverage includes:
- Every strict prefix of a valid leaderboard JSON payload.
- Control-character mutations at every byte position.
- A corpus of malformed and truncated JSON payloads.
- Existing tests for duplicate keys, excessive nesting,
  oversized input, and empty or malformed files.

Validation:
- Windows MSVC headless build succeeded.
- Headless CTest: 10/10 test suites passed.
- Desktop application build succeeded.
- Desktop CTest: 10/10 test suites passed.

This is deterministic fuzz-style regression coverage, not a
coverage-guided fuzzing campaign.

## 2. Static network and telemetry audit

Searched C++ source files under `src` and `include` for common
networking and telemetry API names. No matches were returned.

The CMake configuration disables SFML's network module when SFML
is fetched. Dependency downloads through CMake FetchContent occur
during build/configuration and are separate from runtime behavior.

A source search is useful evidence but is not, by itself, proof
that all possible network activity is absent.

## 3. Runtime offline verification

- Confirmed the SnakeXtreme process was running.
- A 60-second PowerShell poll reported no TCP connections owned
  by the game process.
- Tested the desktop executable with a Windows Firewall outbound
  block rule scoped to SnakeXtreme.exe.
- The user reported that the application continued to work properly
  with the outbound block rule enabled.
- The temporary firewall rule was removed after testing.

The recorded TCP observation is time-limited. No separate UDP
endpoint observation was recorded.

## Conclusion

The parser regression tests pass, and the application operated
normally during the reported outbound-blocked test. These results
support the local offline execution requirement. They do not
constitute a coverage-guided fuzzing campaign or an exhaustive
proof against every possible network call.

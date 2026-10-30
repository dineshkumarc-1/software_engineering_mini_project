# Issue #24 — Fuzz Testing and Offline Privacy Evidence

Date: 2026-10-29

## 1. Parser fuzz-regression testing

Added deterministic malformed-input tests in tests/SecureFileStoreTests.cpp.

Coverage includes every strict prefix of a valid JSON payload, byte-position control-character mutations, a malformed/truncated payload set, and the existing duplicate-key, nesting-depth, file-size and empty/malformed-input cases.

This is deterministic fuzz-style regression testing, not a coverage-guided fuzzing campaign.

## 2. Static network/telemetry audit

The earlier source scan under src and include reported no matches for the common networking/telemetry APIs searched. The CMake configuration disables SFML's network module when SFML is fetched. A source scan alone is not exhaustive proof of the absence of all network activity.

## 3. Previously reported runtime observations

- An earlier 60-second PowerShell poll reported no TCP connections owned by the game process; this is separate from the fuller capture documented below.
- During the 2026-10-29 capture, the user confirmed the game remained usable while an executable-scoped outbound Windows Firewall block rule was enabled.
- The committed redacted JSON (`issue-24-offline-evidence.redacted.json`) preserves all 60 observations; all 60 report the game process still running, with zero TCP connection records and zero UDP endpoint records across the samples.
- The collector reports ActiveStore verification `true` at both start and end. The rule was configured for outbound blocking, enabled, and scoped to the game executable (Profile `Any`).
- Requested duration was 60 seconds, but the recorded timing shows the loop took longer: capture start `19:17:30.338 +05:30`, final JSON timestamp `19:19:15.041 +05:30` (about 104.70 seconds elapsed); sample timestamps span about 98.35 seconds, at intervals averaging 1.667 seconds (range 1.521–1.999 seconds). The collector performs endpoint queries plus a one-second sleep per iteration, so `DurationSeconds=60` did not result in a strict 60-second wall-clock capture. The collected observation window is therefore longer than requested; this limitation is recorded explicitly.

## 4. Repeatable TCP/UDP and firewall capture

The companion collector Collect-Issue24-OfflineEvidence.ps1 launches the specified executable, creates a temporary outbound block rule scoped to that executable, verifies the rule in Windows Firewall ActiveStore at the start and end, samples process-owned TCP connections and UDP endpoints once per loop with a one-second sleep (actual intervals also include query overhead), writes JSON evidence, and removes only the temporary rule it created.

Run from an elevated PowerShell session, using the actual local executable path:

    Set-ExecutionPolicy -Scope Process Bypass
    .\Collect-Issue24-OfflineEvidence.ps1 -ExecutablePath 'C:\path\to\SnakeXtreme.exe' -DurationSeconds 60 -OutputPath '.\issue-24-offline-evidence.json'

During the capture, interact with the game and separately note whether normal menu navigation/gameplay remains functional while the rule is active. Keep the generated JSON with the test artifact and record the exact executable build/commit SHA.

**Execution update (2026-10-29):** The uploaded Windows JSON was reviewed. Across 60 samples, no process-owned TCP connections and no process-owned UDP endpoints were observed. The process was reported running in all 60 samples. ActiveStore verification was `true` before launch and at the end; the configured rule was enabled, outbound, block, Profile `Any`, and scoped to the SnakeXtreme executable. The user confirmed that gameplay remained usable while the rule was active.

Original unredacted upload SHA-256 (not committed): `8e64b38706d1dbbbd549f71f73325998edfbcdcff2b2b89841a47c7fdf50e34e`.

**Public repository copy:** `docs/sprint2/evidence/issue-24-offline-evidence.redacted.json` preserves all 60 sample records and firewall-verification fields. It redacts the local computer name, full executable paths, and random suffix of the firewall-rule display name.

Committed redacted evidence SHA-256: `0a2ac44068c9fecf7c288d2f1fbc9023026bb06598f35b7700681ed439c47dc4`. This digest identifies the committed redacted JSON, not `SnakeXtreme.exe`.

**Timing limitation:** although `DurationSeconds` was set to 60, the capture loop took approximately 104.70 seconds from `CaptureStartedAt` to `CapturedAt`; sample timestamps span 98.35 seconds. Endpoint queries and the one-second sleep per iteration caused actual intervals to average 1.667 seconds (range 1.521–1.999 seconds). These are point-in-time process-owned connection/endpoint snapshots, not packet capture. No TCP connection or UDP endpoint was observed at the samples, but brief connections or activity between samples are not ruled out, and the results do not prove that no possible network attempt can occur.

## 5. Tested build identity and final-candidate follow-up

The committed redacted JSON artifact has SHA-256
\`0a2ac44068c9fecf7c288d2f1fbc9023026bb06598f35b7700681ed439c47dc4\`.
This digest identifies the committed redacted JSON, not \`SnakeXtreme.exe\`. The original unredacted upload has a separate SHA-256 of
\`8e64b38706d1dbbbd549f71f73325998edfbcdcff2b2b89841a47c7fdf50e34e\` and is not committed.

The committed redacted JSON records the OS caption as Windows 11 Home Single Language, all 60 observations, process-running state, endpoint results, and ActiveStore verification. Machine and local path identifiers are explicitly redacted. It does **not** record the repository commit SHA, compiler/build configuration, executable size, or executable SHA-256. The earlier review notes do not preserve those values either. Therefore the exact source revision and binary used for this historical capture cannot be identified from the retained evidence, and the observations must not be attributed to commit \`3032c5a\` or a newer candidate.

A final-candidate privacy capture is **Pending** coordination with Dinesh after the D6 fix from PR #48 is incorporated into the selected candidate. For that run, record, alongside the evidence artifact:

- \`git rev-parse HEAD\` from the exact source checkout used to build.
- Compiler/toolchain and build configuration.
- Executable byte size and \`Get-FileHash .\\build\\Release\\SnakeXtreme.exe -Algorithm SHA256\` output from the exact binary launched by the collector.
- The JSON evidence artifact SHA-256 separately from the executable SHA-256.
- Whether the menu/gameplay remained usable with the executable-scoped outbound block active.

Keep the original unredacted JSON out of the repository because it includes a local computer name and executable path. The redacted 60-sample copy is committed for review; the source/build identity gap remains unresolved. The existing observation remains historical evidence, not an exhaustive test of possible network activity and not proof about the final candidate.

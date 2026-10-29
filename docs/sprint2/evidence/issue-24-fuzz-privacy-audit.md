# Issue #24 — Fuzz Testing and Offline Privacy Evidence

Date: 2026-10-29

## 1. Parser fuzz-regression testing

Added deterministic malformed-input tests in tests/SecureFileStoreTests.cpp.

Coverage includes every strict prefix of a valid JSON payload, byte-position control-character mutations, a malformed/truncated payload set, and the existing duplicate-key, nesting-depth, file-size and empty/malformed-input cases.

This is deterministic fuzz-style regression testing, not a coverage-guided fuzzing campaign.

## 2. Static network/telemetry audit

The earlier source scan under src and include reported no matches for the common networking/telemetry APIs searched. The CMake configuration disables SFML's network module when SFML is fetched. A source scan alone is not exhaustive proof of the absence of all network activity.

## 3. Previously reported runtime observations

- A 60-second PowerShell poll previously reported no TCP connections owned by the game process.
- The user reported that the game remained usable with an app-specific outbound Windows Firewall block rule enabled; the temporary rule was subsequently removed.
- Those prior notes did not record a separate UDP endpoint snapshot or an ActiveStore verification result during the same capture window.

## 4. Repeatable TCP/UDP and firewall capture

The companion collector Collect-Issue24-OfflineEvidence.ps1 launches the specified executable, creates a temporary outbound block rule scoped to that executable, verifies the rule in Windows Firewall ActiveStore at the start and end, samples process-owned TCP connections and UDP endpoints once per second, writes JSON evidence, and removes only the temporary rule it created.

Run from an elevated PowerShell session, using the actual local executable path:

    Set-ExecutionPolicy -Scope Process Bypass
    .\Collect-Issue24-OfflineEvidence.ps1 -ExecutablePath 'C:\path\to\SnakeXtreme.exe' -DurationSeconds 60 -OutputPath '.\issue-24-offline-evidence.json'

During the capture, interact with the game and separately note whether normal menu navigation/gameplay remains functional while the rule is active. Keep the generated JSON with the test artifact and record the exact executable build/commit SHA.

**Execution update (2026-10-29):** The collector completed successfully on the Windows machine for a 60-second capture. It reported process ID `32772`, `60` samples, and `rule active at end: True`. The script verifies the executable-scoped outbound block rule in ActiveStore before launching the game and again at the end; successful completion plus the displayed end result supports that the rule verification completed for this run. Bharath confirmed the game remained usable during the capture while the outbound rule was active. The output was saved to `C:\\WINDOWS\\system32\\issue-24-offline-evidence.json` and has not yet been attached here, so the per-sample TCP/UDP observations and the collector's exact JSON fields have not been independently inspected or summarized. Attach the JSON to the PR or QA record before marking the evidence packet fully reviewed. The collector's endpoint samples are not a packet capture and do not prove that all possible network activity is absent.
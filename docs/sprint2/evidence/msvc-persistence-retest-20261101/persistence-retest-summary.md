# Windows 11 MSVC — controlled persistence retest

Date: 2026-11-01
Candidate commit: 3fcc586d740d2b9b461193e482d6861ca8f905e5
Source tree: 41b54378c177d7d1e01ff17f2bf7b22663789ec6

## Executable identity

- Executable: build/Release/SnakeXtreme.exe
- Size: 1896960 bytes
- SHA-256: 3496FD8C9E8ED95FB886BEF272DBAF9D62A181237E3FDB39474FF7627C3E42B5
- Working directory for both launches: <local-evidence-root>/persistence-repro-20261101
- First launch PID: 25568
- Second launch PID: 13504
- Both launches exited normally.

## Persistence results

- Leaderboard survived restart: True
- Settings survived restart: True
- Player: Bharath
- Score: 20
- Mode: Timed
- Difficulty: Medium
- Theme before change: Classic
- Theme after save: Neon
- Theme after restart: Neon

## Before/after JSON hashes

- leaderboard.json: before = 57CAB33B258A3236DCC1BE1BABAB8FAD1D450D2224CB21F47792221590567B47
- leaderboard.json: after restart = 57CAB33B258A3236DCC1BE1BABAB8FAD1D450D2224CB21F47792221590567B47
- config.json: before = 79428BEF2B5DFED806FF22A1FAE22A4DA292F65EA664AAA7267622DCC22172D7
- config.json: after save = AAF0B2D53E7862DDC108C7CA5339B1731F6C795AE63F0AC6B0C85EC9CD0949D5
- config.json: after restart = AAF0B2D53E7862DDC108C7CA5339B1731F6C795AE63F0AC6B0C85EC9CD0949D5

## Standard error

First launch:
[WARN] SettingsRepository: E_FILE_MISSING: defaults loaded

Second launch:
(empty)

## Interpretation

The controlled retest used the same executable and identical working directory before and after restart. Both the leaderboard entry and Neon setting survived.

The earlier manual failure is preserved separately. A different working directory during that earlier relaunch is a likely explanation, but not a proven root cause.

A dedicated screenshot of the Game Over "Score saved" message was not captured in this retest. The first-run leaderboard screenshot and persisted JSON show the saved entry.

See persistence-retest-hashes.txt for hashes of the snapshots, logs and screenshots.

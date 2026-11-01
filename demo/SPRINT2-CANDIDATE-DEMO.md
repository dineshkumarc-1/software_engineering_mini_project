# Sprint 2 candidate working-product demo

[Watch or download the two-minute actual-gameplay video](sprint2-candidate.mp4).

**Candidate footage, not final release/QA acceptance.** Recorded from merged product source `3fcc586d740d2b9b461193e482d6861ca8f905e5` on Linux. Issue #28 also requires recording after all Sprint 2 pull requests are merged; pending product PRs must be resolved and this footage revalidated/re-recorded as appropriate before final closure.

## What the video actually shows

| Time | Demonstration |
|---|---|
| 0:00–0:06 | Main menu > Name entry > Mode selection |
| 0:06–0:15 | Classic mode - food and growth |
| 0:15–0:24 | Multiplier power-up |
| 0:24–0:32 | Shrink power-up |
| 0:32–0:41 | Speed power-up |
| 0:41–0:47 | Pause and resume |
| 0:47–0:54 | Collision and Game Over |
| 0:54–1:00 | Leaderboard - six columns |
| 1:00–1:07 | Settings - independent audio controls |
| 1:07–1:15 | Clean relaunch - configuration persists |
| 1:15–1:21 | Leaderboard persists across relaunch |
| 1:21–1:36 | Timed Challenge mode |
| 1:36–2:00 | Obstacle mode |

## Capture and provenance

- Actual 900×600 SFML client footage, framed at 1280×800 with explanatory captions outside the game area; approximately 120 seconds, 30 fps, H.264/AAC.
- Real Multiplier pickup yields +20 food scoring; Shrink changes length 11→10; Speed pickup shows its active countdown. No fabricated HUDs, planted items, altered clocks, invincibility or gameplay changes. Normal keyboard/mouse input was automated.
- Three normal application processes were used. The second process loads the first process's saved score (260) and settings (Music 60 / SFX 80) from the same isolated runtime directory. No real player saves were modified.
- Audio is actual application output captured through OpenAL's wave backend and muxed with the corresponding footage. It is not a physical-speaker/microphone observation or Windows audio regression result.
- Excerpts are cut at normal playback speed. Original captures, event logs and failed capture attempts are preserved in the recording workspace.
- Executable SHA-256: `4abc22684ff77b433f7a900194ff2dc6a2f8c8fa1811b00030ff831c558f8b34`.
- Video identity and source-cut mapping: [sprint2-candidate-manifest.json](sprint2-candidate-manifest.json).
- This demo is not proof of #22 timing/FPS acceptance, a resolution of Windows persistence failures, a release tag, or final Sprint 2 sign-off.

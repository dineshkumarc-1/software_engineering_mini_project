# Sprint 1 working-product demo

[Watch or download the one-minute gameplay video](sprint1.mp4)

Recorded from merged `main` commit `61d7663` (PR #41, after PRs #34 and #35).

## Demonstrated in the actual application

| Video time | Gameplay |
| --- | --- |
| 0:00–0:02 | Main menu: Classic / Easy selection |
| 0:02–0:14 | Grid, movement, food collection, snake growth and live score HUD |
| 0:14–0:17 | Pause; gameplay then resumes |
| 0:17–0:21 | Wall collision and game-over reason |
| 0:21–0:24 | Main menu: Timed / Medium selection |
| 0:24–0:29 | Timed gameplay and live countdown |
| 0:29–0:34 | Later in the same Timed session: real countdown reaches zero and ends the game |
| 0:34–0:37 | Main menu: Obstacle / Hard selection |
| 0:37–0:52 | Real obstacle gameplay, food and 15-point scoring |
| 0:52–0:57 | Obstacle collision and game-over reason |
| 0:57–1:00 | Restart: score resets, session high score retained; paused for inspection |

## Recording and validation

- **Actual game footage**, edited from real application sessions at normal playback speed. The Timed session ran for its full 60-second limit; the middle is omitted to keep the deliverable one minute long.
- Linux, SFML 2.5.1, software OpenGL on an Xvfb display. Keyboard input was automated through X11 events. No game source, rules, scores, clock or random seed was changed for recording.
- The Obstacle footage is from a separate application run and includes normal restarts. High scores are in-memory/session-only, not persistent across application launches.
- No simulated game screens or test-result slides. Explanatory captions are outside the captured 900×600 game area.
- 60 seconds, 1280×800, 30 fps, H.264 MP4, no audio.
- Exact merged source built successfully; all **7 local CTest suites passed** before recording.
- This is Linux runtime evidence, **not Windows/macOS runtime or performance validation**.

Completes the recording/upload deliverable for issue #27; platform/performance validation remains separate.

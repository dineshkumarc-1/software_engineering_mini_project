# Sprint 2 final working-product demo

[Watch or download the final two-minute video](sprint2.mp4).

Fresh actual-gameplay capture recorded on 1 November 2026, **after** the owner selected and published `v1.0.0-final`, merged the accepted code/QA PRs, and explicitly deferred #69/#70. This completion is late against the original October 30 deadline; it is not backdated. The video is not the earlier candidate relabeled as final.

## Demonstration timeline

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

## Source, execution and review

- Frozen release source: `e4c6f029e9531d3b066a6098e86fab6bf098f56a` ([release](https://github.com/dineshkumarc-1/software_engineering_mini_project/releases/tag/v1.0.0-final)). Actual Linux executable built from `3a286625ceb7ed15c24a2a892d6984ba173a529a`; only seven QA documentation files differ from the release source, with product/build/test inputs unchanged. Do not call it a new binary rebuilt from the documentation-only tag.
- Executable SHA-256: `095f13b5032dee16d25032b303e909cc670943a8fac1ee0601f229cf5656c32b`.
- Fresh captured normal sessions show all three modes, real Speed/Multiplier/Shrink pickups, +20 multiplier food scoring, actual shrink length reduction, pause/resume, natural collision/Game Over, the six-column leaderboard, and both settings and leaderboard persistence across a clean process relaunch. All three processes exited normally with code 0.
- Actual SFML frames captured at 900×600, framed at 1280×800 with captions outside the game client. Editing uses excerpts at normal playback speed. Normal keyboard/mouse steering only; no fabricated screens, planted power-ups, edited scores, invincibility, seeded RNG, clock or gameplay changes.
- Audio is actual application output captured via OpenAL's wave backend and aligned with the recording. It is not a physical-speaker or Windows audio regression observation.
- Video: 120.021 seconds, H.264 30 fps / AAC stereo, 3,691,321 bytes. SHA-256: `2861d1e42bcf086cc8d35a3f75289a89ffe773e280104faac2c7d0392f5ffc18`. [Exact capture/edit manifest](sprint2-manifest.json).
- Original captures, events, screenshots and an incomplete fresh setup attempt are preserved in the recording workspace. The [earlier candidate](SPRINT2-CANDIDATE-DEMO.md) remains separately identified.

## Scope

This media/documentation-only addition does not change frozen product code or move the release tag. It completes the working-product demonstration deliverable; it does not turn owner-accepted test limitations into measured passes or grant a new QA signature. No Windows/macOS, cold-start, sustained-FPS or coverage acceptance is claimed.

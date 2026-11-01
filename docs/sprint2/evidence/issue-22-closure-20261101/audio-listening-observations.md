# Owner-reported audio listening observations — 2026-11-01

The following observations were provided by Dinesh Kumar during interactive
manual checks. They are owner-reported, not audio recordings or independent
observations.

## Earlier PR #69 build

Source `841ee1dccac37236a4bdcf92ec3dbf17e7a18c80`; EXE SHA-256
`341bd6394a93741137866c88d95c3b00ae9ccb6d160e19d9044a0fddde68b57b`.

- Main Menu music was audible and looping.
- Settings showed Music 63, SFX 71, and no audio-unavailable warning.
- F5 preview tone was audible at SFX 71.
- Music 63 → 58 became quieter while remaining audible; restoring 63 returned
  the previous level.
- F5 was audible at SFX 71 and silent after lowering SFX to 0; music continued.
- Esc Cancel discarded the SFX=0 draft and restored the prior music level.

## Latest local PR #69 candidate

Source `4b1d21420ff20a8e06751b675bd20d73e9a13764`; EXE SHA-256
`47e43387ecc4d6407de2a29022e8b9884027fea8e3adb09c24d8a9aa8f25a915`.

- Main Menu music was audible and looping.
- Game Over sound was audible.
- Owner reported hearing the food-pickup cue during gameplay.
- Music stopped while paused and resumed after Enter.

No recorded audio file or independent listener is claimed. The latest-candidate
manual run used normal audio; audio was not disabled by the check.

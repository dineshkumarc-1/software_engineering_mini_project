# Game Over collision danger alert

Owner-requested follow-up after acceptance of game candidate `7c6733d`.

## Change

The existing Game Over cue was a single low 164.81Hz tone. Replace it with a distinct descending pair: 880Hz for150ms,50ms silence,then440Hz for150ms (350ms total). PCM is generated deterministically in code,with click-free envelopes and the same6000 amplitude cap. No downloaded sound files,new dependencies or schema/API changes.

The existing AppManager Game Over transition still triggers the cue once,including wall/body/obstacle collision and timed expiry. That routing is unchanged; this is not an application-crash handler. Existing independent SFX volume,mute and pause/fallback behavior are retained. SFX0 intentionally produces no audible alert. Background music and food/preview waveforms are unchanged.

## Verification

Linux GCC11.5 warnings-as-errors Release desktop build passes. Final results:
- CTest21/21.
- Focused audio tests161assertions/5cases,including deterministic bounded350ms audio,two audible signal segments,silent gap,descending pitch and silent pulse endpoints.
- Focused changed audio/settings/test code compiled with GCC ASan+UBSan/leak detection:161assertions pass,no reported findings. Cached Catch2 framework itself was not rebuilt with sanitizer instrumentation; this is focused coverage,not a fresh full-project sanitizer claim.
- cppcheck2.13:zero findings,unchanged gate.
- Real SFML/OpenAL backend7checks pass using null output; not a listening test.
- Linux/Xvfb desktop navigation23checks pass,including real wall-collision Game Over and restart; not a native Windows audio test.

Sanitized final outputs: `evidence/collision-danger-alert-checks.txt`.
An initial redundant constant-order assertion was removed after cppcheck flagged it; no suppressions/gate changes. Missing local desktop/sanitizer dependencies were restored before final passing runs.

## Windows handoff

This changes product audio code. The existing Windows EXE for7c6733d DOES NOT contain the new waveform. Preserve its QA evidence as historical. Build the final reviewed PR commit in a separate checkout/runtime,record its new SHA and EXE hash,then listen with SFX enabled: collide once and confirm the two-part alert,confirm no repeating alert while remaining on Game Over,restart and collide again,then confirm SFX0 is silent. Confirm music/food/preview,Save/Cancel and relaunch gain behavior remain correct. Do not claim those new native listening checks passed from Linux/null output.

No final release,scope waiver or automatic merge is authorized by this note.

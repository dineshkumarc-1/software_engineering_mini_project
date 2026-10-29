# Desktop smoke synchronization follow-up

On tooling-fix head `79e0d82be494cb662eeebadd6ea1c78d12f5666f`, all four
headless matrix jobs and the Windows app build passed. Linux app configure,
build, CTest and audio smoke passed, but the existing navigation smoke failed
at the immediate `Dinesh@` title assertion. This is not a green CI result:
https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/40710416987

`xdotool` completion is not an application-event-consumption barrier. The
assertion previously read the title immediately after typing. An unchanged
local rerun passed all 23 checks. A repeated local check also reproduced the
same premature-read problem at the preceding 16th-character assertion.
Both checks now use the existing bounded predicate wait to require the exact
retained name and the specific feedback produced by the final rejected event
(`Z` at capacity or invalid `@`), before keeping the original assertions.
No production code, accepted-name rules, timeout length or fixed sleep changed.

After this test-only synchronization fix, three consecutive local desktop
navigation runs pass **23 checks each**, using the existing app, Xvfb,
software GL and null audio with isolated temporary configuration. The complete
Python suite still passes **33 tests**. Local runs do not erase the failed CI;
the subsequent published head must be checked separately. Prior source hashes
remain historical; `owner-tooling-navigation-sync-sha256.txt` identifies this
changed test input. No Windows/manual or target acceptance is claimed.

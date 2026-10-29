# Owner-only acceptance preparation (#19 / #20 / #22)

This workflow adds no QA sign-off. It addresses build-identification gaps without
pretending that manual testing occurred. The original #46 work is now consolidated
in owner #53 through #56; see SPRINT2-INTEGRATION-STATUS.md for current history.
Historical scope descriptions below do not override that reconciliation status.

## Fixed-capacity name entry (#19)

`NameEntry` now owns `std::array<char, 15>` and an explicit length. Editing never
resizes a string or relies on a null terminator. Backspace clears the removed
slot. Rendering receives a separate `std::string` snapshot. C10's existing
`isAllowedPlayerNameCharacter`, `isValidPlayerName` and `sanitizePlayerName`
remain unchanged and own the validation/normalization rules. Confirmation
validates before normalization, checks the bounded result, then copies it back
into the array. Temporary display/normalization/error strings are not the input
backing buffer; no claim is made that all UI strings are allocation-free.

Regression adds 40 cycles of 10,000-character injection and 10,000 backspaces,
normalization/reset, non-ASCII and out-of-Unicode-range codepoints, independent
copies and snapshot lifetime checks. This is scoped SEC-006 input-buffer evidence,
not final security or full-app sanitizer sign-off.

## Failed Save transaction (#20)

The existing isolated Linux Settings desktop script now replaces its temporary
config fixture with a directory, forcing atomic replacement to fail regardless
of root/chmod permissions. It checks visible failure/staying on Settings,
preservation of the target directory, owned-temporary cleanup, Cancel restoring
previously active values, and recovery of the saved fixture. It does not touch
player saves, the real working directory, C13 implementation or shared APIs.
Windows error-path acceptance and C13 integration remain pending.

## Identify the exact Windows build before manual testing (#22)

After the required integrations, use the Windows package from that exact CI run.
Keep executable, OpenAL runtime, fonts and notices together. Do not relabel an
older tested artifact as a newer commit or ask QA to repeat unrelated tests
before the final build is identified.

```
python tools/prepare_acceptance.py PATH_TO_SnakeXtreme.exe NEW_ACCEPTANCE_DIRECTORY --commit ACTUAL_40_CHARACTER_SHA --artifact-url https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/ACTUAL_RUN_ID
```

Replace both placeholders with actual values. An Actions artifact URL ending
`/artifacts/ACTUAL_ARTIFACT_ID` is also accepted. Output directory must be new.
The tool does not download or execute the artifact, query the run, verify its
source relationship, clear caches, measure performance, access player settings,
or mark any test passed. It:
- SHA256-hashes the executable and known packaged dependencies when present;
- records the operator-supplied source SHA and run/artifact URL separately;
- omits usernames, hostnames and absolute file paths;
- creates `build-manifest.json` with `acceptance: not_assessed`;
- creates an **unexecuted**, entirely Pending `manual-results.md` template.

**Before testing:** independently open the Actions run and verify its head SHA
matches the manifest's supplied commit. Record that check and the actual Windows
hardware, date/time/timezone, audio output and driver configuration. The hashes
identify files, not source provenance. Missing DLL/fonts remain a packaging
check, not automatically a tool failure on a non-Windows developer machine.

**During testing:** fill expected/actual results and evidence/defect references
for name limits and mouse flow, changed key bindings/duplicates/WASD/old primary
keys, F5 at current SFX volume (including zero), independent audible channels,
Save/Cancel/relaunch, failed saves, themes and corrupt recovery. Do not mark a
pending row passed because CI or another build passed. Coordinate those results
with the QA owner; owner merge approval and QA execution have separate roles.

**Performance:** use `performance-probes.md` and its explicit limitations. Three
continuous 300-s Hard/Obstacle runs with power-ups, actual target cold launch,
acknowledgement timing with OS/SFML queue limitations documented, and three
independent volunteers are still real
acceptance work. The generated template is not a substitute.

## Scope boundaries

The current reconciliation imports reviewed #48 through 4793a7a, including D6,
and preserves the owner Settings adapter and six-column leaderboard. The separate
source/test evidence covers that integration; this preparation tool itself never
merges PRs, verifies source provenance, executes tests, closes issues or grants
release acceptance. Final Windows/privacy/performance/manual evidence is Pending.

# Owner follow-up execution evidence

These local Linux results correspond to the source manifest, not a final release
or target Windows acceptance. See ../../SPRINT2-INTEGRATION-STATUS.md for exact
input revisions, qualifications and remaining blockers. Headless/sanitizer
CTest runs used an unprivileged account. Desktop used software GL and null audio.
The final screenshot is 900x600 from the final tested desktop executable.

The D6 known-defect log intentionally contains expected failures: an overall
CTest pass does not mean the empty/whitespace warning issue is resolved.
Failed harness attempts are retained separately and are not counted as passes.
The source manifest excludes documentation/evidence to avoid self-reference.
Published-head hosted CI remains a separate verification step.

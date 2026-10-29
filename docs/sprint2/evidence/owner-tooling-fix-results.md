# PR #46 owner tooling regression fixes

AI-assisted C Dinesh Kumar owner follow-up to source
`683ccf45ed944df12f66b06b762c5beab2f64d54`. Results describe the changed
working-tree inputs hashed in `owner-tooling-fix-source-sha256.txt`, not the
parent SHA alone. Earlier evidence remains historical for its own snapshot.

## Fixed review findings

- A zero-exit child printing the first-frame marker without creating its raw
  report no longer counts as a collected launch. The collector validates a
  regular, non-symlink report through the shared analyzer and requires a
  first-frame timestamp. It preserves per-run failure reasons and exits 1 for
  incomplete runs. Existing report schema, size, timestamp and sample checks
  apply rather than relying on file existence alone.
- FPS summaries reject overlapping, duplicate, reversed, pre-first-frame and
  off-grid windows. Windows must lie on the one-second grid anchored to the
  actual first-frame timestamp. Valid zero-FPS windows, exact boundaries and
  aligned gaps are retained; gaps break the continuous-session count.

## Executed checks

Environment: Amazon Linux 2023 x64, Python 3.13.15, GCC 11.5.0, CMake 4.4.4;
existing headless build and desktop app from the parent snapshot. No C++ source
changed in this follow-up. Desktop smoke uses Xvfb/software GL/null audio and
isolated temporary player settings, not target Windows hardware.

- `python -m unittest discover -s tests -p 'test_*.py' -v`: **33 tests pass**:
  14 analyzer, 10 collector and 9 acceptance-preparation tests. The collector
  cases simulate a subprocess across platforms but exercise the real report
  validator and CLI return decision. Regressions cover missing/malformed/raw
  schema/first-frame/overlapping reports, marker absence, nonzero exit,
  timeout, valid success and output-directory preservation.
- `ctest --preset headless --output-on-failure`: **11/11 suites pass**.
- `tools/desktop_probe_smoke.py`: **100 synthetic randomized requests and
  3 real isolated launch traces pass**, now asserting validated collection
  status for every launch. This exercises the changed collector against the
  existing actual desktop executable, not just simulated children.

Logs: `owner-tooling-fix-python.txt`, `owner-tooling-fix-ctest.txt`,
`owner-tooling-fix-probe.txt`. CI for the published head must be observed
separately; local success is not a claim of new-head CI success.

## Still pending / unchanged scope

`acceptance: not_assessed` is unchanged. A valid collected report does not mean
there are no dropped samples or grant performance acceptance. Target Windows,
cold-cache, end-to-end input, power-up-active performance and volunteer checks
remain pending. C13/persistence integration depends on the reviewed PR #48
fixes. No teammate branch, shared validator, C++ API, QA document, original
staged workspace, final release/demo, merge or issue closure is changed.
PR #46 remains draft and issues remain open.

# Persistence temporary-file cleanup policy

## Matching and deletion rules

Cleanup only considers files in the destination's sibling directory whose names
match the exact destination prefix and a non-empty ASCII decimal nonce:

- Temporary file: `<destination>.tmp.<nonce>`
- Lock sidecar: `<destination>.tmp.<nonce>.lock`

Candidates must be regular files, not symbolic links, and must be at least
24 hours old. Orphan lock sidecars are eligible only when the corresponding
temporary path is absent (checked without following a symlink). Cleanup takes
a non-blocking exclusive lock and rechecks age and temporary-path absence
before removing a sidecar. It never performs blanket `.lock` deletion.

Active lock holders are skipped. Fresh temp files, fresh sidecars, unrelated
destination names, non-decimal suffixes, and symbolic links are preserved.
A successful write removes its own temp file after atomic replacement or
failure and releases its own lock sidecar.

## Timing limitation

The 24-hour threshold intentionally prevents eager deletion of recently
created files. Therefore this policy **does not guarantee that a crash-created
temp or orphan lock will be removed on the very next startup**. Cleanup is
best-effort when a read or write occurs after the candidate exceeds the
threshold. The policy favors preserving fresh or potentially active writer
state over immediate cleanup.

## Verification

Regression tests in `tests/SecureFileStoreTests.cpp` cover stale temp removal,
orphan sidecar removal, active-lock preservation, fresh-file preservation,
symlink preservation, and unrelated/non-decimal-name preservation. The
cross-platform behavior remains subject to the Windows and POSIX CI results
for the exact commit under review.

# Evidence checksum and line-ending convention

`SHA256SUMS.txt` records SHA-256 digests of the exact Git blob bytes staged for the evidence files. It does not record hashes of Windows working-copy text files.

Git line-ending normalization may store LF bytes in the repository while a Windows checkout presents CRLF bytes in the working tree. Consequently, `Get-FileHash` on a Windows working-copy file can differ from the digest in `SHA256SUMS.txt`, even when the text is logically identical.

To verify a manifest entry, hash the bytes returned by `git cat-file blob` for the corresponding staged/index blob. Do not regenerate this manifest from `Get-FileHash` over Windows working-copy text files.

The original `build-release.log`, `ctest-release.log` and `ctest-inventory.log` remain the historical seven-case baseline artifacts. The updated review-fix result is recorded separately in the summary, run identity and focused test result. The handoff labels those earlier logs as baseline evidence.
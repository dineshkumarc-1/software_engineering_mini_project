#!/usr/bin/env python3
"""Create an unfilled owner acceptance bundle tied to an exact executable hash.
No tests are executed, no results are invented, and no real settings are edited.
"""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import re
from urllib.parse import urlsplit

MAX_BINARY_BYTES = 256 * 1024 * 1024

def hash_file(path):
    if path.is_symlink() or not path.is_file():
        raise ValueError("A regular non-symlink file is required")
    if not 0 < path.stat().st_size <= MAX_BINARY_BYTES:
        raise ValueError("File must contain 1 byte to 256 MiB")
    digest = hashlib.sha256()
    total = 0
    with path.open("rb") as source:
        while True:
            chunk = source.read(1024 * 1024)
            if not chunk:
                break
            total += len(chunk)
            if total > MAX_BINARY_BYTES:
                raise ValueError("File grew beyond 256 MiB while hashing")
            digest.update(chunk)
    return digest.hexdigest()

def prepare(executable, output, commit, artifact_url):
    if not re.fullmatch(r"[0-9a-fA-F]{40}", commit):
        raise ValueError("Supply the exact 40-character source commit SHA")
    if any(ord(character) < 32 for character in artifact_url):
        raise ValueError("Artifact URL cannot contain control characters")
    url = urlsplit(artifact_url)
    if url.scheme != "https" or url.netloc != "github.com" or url.query or url.fragment or not re.fullmatch(
        r"/dineshkumarc-1/software_engineering_mini_project/actions/runs/[0-9]+(?:/artifacts/[0-9]+)?", url.path
    ):
        raise ValueError("Supply this repository's HTTPS Actions run/artifact URL")
    executable = Path(executable)
    files = [{"file": executable.name, "sha256": hash_file(executable)}]
    for relative in ("openal32.dll", "assets/fonts/DejaVuSans.ttf", "THIRD_PARTY_NOTICES.md"):
        dependency = executable.parent / relative
        if dependency.exists():
            files.append({"file": relative, "sha256": hash_file(dependency)})
    manifest = {
        "schema_version": 1,
        "acceptance": "not_assessed",
        "source_commit_operator_supplied": commit.lower(),
        "artifact_url_operator_supplied": artifact_url,
        "source_artifact_relationship_independently_verified": False,
        "collection_platform": {"system": platform.system(), "release": platform.release(), "architecture": platform.machine()},
        "package_files": files,
        "warnings": [
            "Hashes identify files, not their source commit. Verify the Actions run head SHA matches the supplied commit.",
            "No runtime, audible, target-hardware, performance or usability check has been executed by this tool.",
            "Do not record personal usernames, hostnames, absolute paths, credentials or player data in shared notes.",
        ],
    }
    output = Path(output)
    output.mkdir(exist_ok=False)
    (output / "build-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    (output / "manual-results.md").write_text("""# Owner acceptance record — UNEXECUTED TEMPLATE

Acceptance: NOT ASSESSED. Every row starts Pending. Complete only after actually
running the identified build. This template is NOT QA sign-off or evidence of a pass.

## Build identification and environment
- Actions run head SHA checked against build-manifest.json: Pending
- Executable/package hashes checked: Pending
- Actual test date/time and timezone: Pending
- Windows version, CPU, RAM, GPU/driver, display and audio output: Pending
- Power-ups integrated/active for performance campaign: Pending

| Scope | Expected behavior | Actual observation | Status | Evidence / defect |
| --- | --- | --- | --- | --- |
| TC-UI / name entry | Blank/invalid rejected, 15 accepted, 16th rejected; Back/Continue work | Not executed | Pending | |
| TC-UI / launch | Three confirmation clicks plus typing; all nine mode/difficulty combinations start | Not executed | Pending | |
| TC-UI / pause/restart | Keyboard and mouse Resume/Menu/Restart work | Not executed | Pending | |
| TC-SET-01 | I/K/J/L remap works; duplicates rejected; WASD remains; old primary arrows no longer steer | Not executed | Pending | |
| TC-SET-02 / audio | Music 0/SFX 100 then Music 100/SFX 0 are audibly independent | Not executed | Pending | |
| F5 preview | F5 plays preview SFX at current SFX level; muted at zero; independent of Music | Not executed | Pending | |
| TC-SET / transaction | Save persists on relaunch; Cancel restores prior values; row gaps inactive | Not executed | Pending | |
| TC-SET / save failure | Visible error, remain in Settings, old target preserved, Cancel restores active values | Not executed | Pending | |
| TC-SET-03/04 | Classic/Neon/Retro render and persist correctly; defaults/Cancel/Save work | Not executed | Pending | |
| TC-REL-01 | Missing/corrupt config recovers without crashing; warning recorded without sensitive contents | Not executed | Pending | |
| Cold launch | Independently established cold condition on target hardware; external readiness <2 s | Not executed | Pending | |
| TC-PERF-01 | 3 continuous 300-s Hard/Obstacle runs with power-ups; each 1-s FPS >=30 | Not executed | Pending | |
| TC-PERF-02 | 100 randomized requests; accepted/rejected separated; account for OS/SFML queue wait for end-to-end claims | Not executed | Pending | |
| TC-USE-01 | Three independent first-time volunteers; record actual observations without inventing feedback | Not executed | Pending | |

Use tools/collect_launch.py and tools/analyze_performance.py as documented in
performance-probes.md. Internal input timing is a lower bound; display() return
is not photon timing. Warm Linux traces do not prove cold Windows performance.
QA owner independently reconciles results and signs off only on the integrated build.
""", encoding="utf-8")
    return manifest

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("new_output_directory", type=Path)
    parser.add_argument("--commit", required=True)
    parser.add_argument("--artifact-url", required=True)
    args = parser.parse_args()
    try:
        result = prepare(args.executable, args.new_output_directory, args.commit, args.artifact_url)
    except (OSError, ValueError) as error:
        parser.exit(2, f"Acceptance bundle not created: {error}\n")
    print("Created unexecuted acceptance template; executable SHA256=" + result["package_files"][0]["sha256"])

if __name__ == "__main__":
    main()

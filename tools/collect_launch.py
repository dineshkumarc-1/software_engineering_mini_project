#!/usr/bin/env python3
"""External process-launch to observed first-frame-marker upper bound.
Does not clear OS caches, assert hardware suitability, or grant acceptance.
Every run uses isolated temporary player settings; no real saves are modified.
"""
import argparse
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import threading
import time

MARKER = "SNAKEXTREME_PROBE_FIRST_FRAME"

def collect(executable, output_dir, runs, snapshot):
    output_dir.mkdir(parents=False, exist_ok=False)
    records = []
    for index in range(runs):
        report = output_dir / f"launch-{index + 1}.json"
        observed = []
        with tempfile.TemporaryDirectory(prefix="snake-launch-") as work:
            with (output_dir / f"launch-{index + 1}-stderr.txt").open("x", encoding="utf-8") as error_log:
                started = time.perf_counter_ns()
                process = subprocess.Popen([str(executable), "--perf-probe", str(report), "--probe-seconds", "1"],
                                           cwd=work, stdout=subprocess.PIPE, stderr=error_log, text=True)
                def read_marker():
                    for line in process.stdout:
                        if line.strip() == MARKER and not observed:
                            observed.append((time.perf_counter_ns() - started) / 1000)
                reader = threading.Thread(target=read_marker, daemon=True)
                reader.start()
                timed_out = False
                try:
                    code = process.wait(timeout=30)
                except subprocess.TimeoutExpired:
                    timed_out = True
                    process.kill()
                    code = process.wait()
                reader.join(timeout=5)
                process.stdout.close()
        records.append({"run": index + 1, "process_exit": code, "timed_out": timed_out,
                        "launch_to_observed_first_frame_marker_us": observed[0] if observed else None,
                        "report": report.name, "config_fixture": "missing_temporary_config"})
    result = {"acceptance": "not_assessed", "source_snapshot_operator_supplied": snapshot,
              "platform": platform.system(), "audio_null_output": os.getenv("ALSOFT_DRIVERS") == "null",
              "cache_condition": "unverified; no OS cache was cleared", "target_hardware_verified": False,
              "measurement_scope": "process creation request to observed stdout marker after first display; IPC/scheduling adds delay",
              "warnings": ["Not proof of cold-cache launch or the specified Windows hardware.",
                           "Use the full executable/DLL/assets package. First-run config regeneration is included.",
                           "No personal player input is recorded; inspect local stderr logs before sharing."], "runs": records}
    with (output_dir / "launch-summary.json").open("x", encoding="utf-8") as output:
        json.dump(result, output, indent=2)
    return result

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("output_directory", type=Path, help="Must not already exist")
    parser.add_argument("--runs", type=int, choices=range(1, 11), default=3)
    parser.add_argument("--source-snapshot", required=True, help="Tested commit SHA or honest working-tree label; not inferred")
    args = parser.parse_args()
    if not args.executable.is_file() or not 1 <= len(args.source_snapshot) <= 160 or any(ord(c) < 32 for c in args.source_snapshot):
        parser.error("Valid executable and source-snapshot label required")
    try:
        result = collect(args.executable.resolve(), args.output_directory.resolve(), args.runs, args.source_snapshot)
    except (OSError, ValueError) as error:
        parser.exit(2, f"Launch collection failed: {error}\n")
    print(json.dumps(result, indent=2))
    return 0 if all(r["process_exit"] == 0 and r["launch_to_observed_first_frame_marker_us"] is not None for r in result["runs"]) else 1

if __name__ == "__main__":
    raise SystemExit(main())

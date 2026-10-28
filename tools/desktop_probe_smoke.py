#!/usr/bin/env python3
"""Linux/Xvfb instrumentation smoke only, NOT performance acceptance.
Usage: python tools/desktop_probe_smoke.py /path/to/SnakeXtreme NEW_OUTPUT_DIR
Produces 100 synthetic randomized gameplay key requests, plus 3 launch traces.
Needs Xvfb and xdotool; accepts normal LD_LIBRARY_PATH/OpenAL environment.
"""
import json
import os
from pathlib import Path
import random
import subprocess
import sys
import tempfile
import time
from collect_launch import collect
from analyze_performance import summarize

exe = Path(sys.argv[1]).resolve()
output = Path(sys.argv[2]).resolve()
output.mkdir(exist_ok=False)
env = dict(os.environ, DISPLAY=":98", ALSOFT_DRIVERS="null", LIBGL_ALWAYS_SOFTWARE="1")
xvfb = process = None

def xdo(*args):
    return subprocess.check_output(["xdotool", *map(str, args)], env=env, text=True, stderr=subprocess.DEVNULL).strip()

def wait(predicate, message, seconds=15):
    end = time.monotonic() + seconds
    while time.monotonic() < end:
        try:
            if predicate(): return
        except subprocess.CalledProcessError: pass
        time.sleep(0.02)
    raise AssertionError(message)

try:
    assert not Path("/tmp/.X11-unix/X98").exists(), "Display :98 is already occupied"
    xvfb = subprocess.Popen(["Xvfb", ":98", "-screen", "0", "1024x768x24"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    wait(lambda: Path("/tmp/.X11-unix/X98").exists(), "Xvfb did not start")
    with tempfile.TemporaryDirectory(prefix="snake-probe-smoke-") as work:
        report = output / "gameplay.json"
        with (output / "gameplay-runtime.txt").open("x") as log:
            process = subprocess.Popen([str(exe), "--perf-probe", str(report), "--probe-seconds", "12"],
                                       cwd=work, env=env, stdout=log, stderr=log)
            ids = []
            def found():
                ids[:] = xdo("search", "--name", "^SnakeXtreme").splitlines()
                return bool(ids) and process.poll() is None
            wait(found, "application did not start")
            window = ids[0]; xdo("windowmove", window, 50, 50)
            def title(): return xdo("getwindowname", window)
            def key(*keys): xdo("key", "--window", window, "--clearmodifiers", "--delay", "20", *keys)
            wait(lambda: "Main menu" in title(), "no main menu")
            key("Return"); wait(lambda: "| Name:" in title(), "no name screen")
            # The report must not contain this private display-name sentinel.
            xdo("type", "--window", window, "--delay", "20", "ProbePrivate")
            key("Return"); wait(lambda: "| Menu |" in title(), "no mode screen")
            key("Right", "Right", "Up", "Return")
            wait(lambda: "Playing | Obstacle / Hard" in title(), "not Hard/Obstacle gameplay")
            randomizer = random.Random(22)
            for _ in range(100):
                if "Game over" in title():
                    key("Return")
                    wait(lambda: "Playing" in title(), "restart failed")
                key(randomizer.choice(("Up", "Down", "Left", "Right")))
                time.sleep(randomizer.uniform(0.005, 0.025))
            process.wait(timeout=20)
            assert process.returncode == 0, "instrumented app failed"
        raw = report.read_text(encoding="utf-8")
        assert "ProbePrivate" not in raw, "probe leaked display name"
        parsed = json.loads(raw)
        assert len(parsed["inputs"]) == 100, "not all 100 requested keys were recorded"
        assert any(i["accepted"] for i in parsed["inputs"]), "no accepted engine input"
        assert any(i["displayed_us"] is not None for i in parsed["inputs"]), "no application/display correlation"
        assert not any(parsed[k] for k in ("dropped_inputs", "dropped_windows", "invalid_samples")), "lost/invalid measurements"
        summary = summarize(report)
        (output / "gameplay-summary.json").write_text(json.dumps(summary, indent=2), encoding="utf-8")
        print("PASS: 100 synthetic randomized requests; accepted/rejected/consumed/displayed separated; no display-name leakage")
    # Preserve environment only for this script's child collector (no global OS settings).
    os.environ.update({k: env[k] for k in ("DISPLAY", "ALSOFT_DRIVERS", "LIBGL_ALWAYS_SOFTWARE")})
    launched = collect(exe, output / "launches", 3, "uncommitted probe smoke working-tree; not a release build")
    assert all(r["collection_ok"] and r["report_valid"] for r in launched["runs"])
    print("PASS: 3 isolated external launch-marker traces; cache condition and target hardware unverified")
    print("All instrumentation smoke checks pass; target performance acceptance NOT ASSESSED")
finally:
    if process is not None and process.poll() is None:
        process.terminate()
        try: process.wait(timeout=5)
        except subprocess.TimeoutExpired: process.kill(); process.wait()
    if xvfb is not None:
        xvfb.terminate()
        try: xvfb.wait(timeout=5)
        except subprocess.TimeoutExpired: xvfb.kill(); xvfb.wait()

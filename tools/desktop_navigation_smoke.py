#!/usr/bin/env python3
"""Real Linux/Xvfb UI regression, not Windows/human acceptance.
Usage: python tools/desktop_navigation_smoke.py /absolute/path/to/SnakeXtreme
Owns display :97 and temporary configuration only. Requires Xvfb/xdotool.
"""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time

exe = Path(sys.argv[1]).resolve()
env = dict(os.environ, DISPLAY=":97", ALSOFT_DRIVERS="null", LIBGL_ALWAYS_SOFTWARE="1")
process = xvfb = None
checks = 0

def xdo(*args):
    return subprocess.check_output(["xdotool", *map(str, args)], env=env, text=True, stderr=subprocess.DEVNULL).strip()

def wait(predicate, message, seconds=10):
    end = time.monotonic() + seconds
    while time.monotonic() < end:
        try:
            if predicate(): return
        except subprocess.CalledProcessError: pass
        time.sleep(0.02)
    raise AssertionError(message)

def check(condition, message):
    global checks
    assert condition, message
    checks += 1
    print("PASS:", message, flush=True)

try:
    assert not Path("/tmp/.X11-unix/X97").exists(), "Display :97 occupied"
    xvfb = subprocess.Popen(["Xvfb", ":97", "-screen", "0", "1024x768x24"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    wait(lambda: Path("/tmp/.X11-unix/X97").exists(), "Xvfb startup")
    with tempfile.TemporaryDirectory(prefix="snake-navigation-") as work:
        with open(Path(work) / "runtime.txt", "w") as log:
            process = subprocess.Popen([str(exe)], cwd=work, env=env, stdout=log, stderr=log)
            ids = []
            def found():
                ids[:] = xdo("search", "--name", "^SnakeXtreme").splitlines()
                return bool(ids) and process.poll() is None
            wait(found, "app window missing")
            window = ids[0]
            xdo("windowmove", window, 0, 0)
            def title(): return xdo("getwindowname", window)
            def screen(text): wait(lambda: text in title(), "screen missing " + text)
            def key(*keys): xdo("key", "--window", window, "--clearmodifiers", "--delay", "30", *keys)
            def click(x, y): xdo("mousemove", "--window", window, x, y); xdo("click", "1")
            def type_name(value): xdo("type", "--window", window, "--delay", "10", value)
            screen("Main menu")
            click(450, 214); screen("| Name:")
            click(550, 456)
            check("| Name:" in title(), "blank name cannot advance by mouse")
            type_name("               "); click(550, 456)
            check("| Name:" in title(), "whitespace-only name cannot advance")
            key(*(["BackSpace"] * 15)); type_name("ABCDEFGHIJKLMNOZ")
            screen("| Name: ABCDEFGHIJKLMNO |")
            check("15 characters" in title() or "15" in title(), "16th character is rejected at runtime")
            key(*(["BackSpace"] * 15)); type_name("Dinesh@");
            check("| Name: Dinesh |" in title(), "invalid character rejected without inserting it")
            click(350, 456); screen("Main menu")
            check(True, "mouse Back returns from name entry")
            # Three confirmation clicks; typed-name entry is additional text input.
            click(450, 214); screen("| Name:"); type_name("Dinesh")
            click(550, 456); screen("| Menu |")
            click(550, 456); screen("Playing | Classic / Medium")
            key("Escape"); screen("Paused")
            click(550, 566); screen("Playing")
            key("Escape"); screen("Paused"); click(350, 566); screen("Main menu")
            check(True, "three confirmation-click launch, mouse Resume and Main menu")
            click(450, 274); screen("Leaderboard integration pending")
            click(350, 456); screen("Main menu")
            check(True, "leaderboard placeholder has working mouse Back")
            for mode_index, mode in enumerate(("Classic", "Timed", "Obstacle")):
                for difficulty_index, difficulty in enumerate(("Easy", "Medium", "Hard")):
                    click(450, 214); screen("| Name:"); type_name("Dinesh")
                    click(550, 456); screen("| Menu |")
                    click(250 + mode_index * 200, 224)
                    click(250 + difficulty_index * 200, 362)
                    screen("| Menu | " + mode + " / " + difficulty)
                    click(550, 456); screen("Playing | " + mode + " / " + difficulty)
                    key("Escape"); screen("Paused"); click(350, 566); screen("Main menu")
                    check(True, "mouse launch/pause/menu: " + mode + "/" + difficulty)
            click(450, 214); screen("| Name:"); type_name("Dinesh")
            click(550, 456); screen("| Menu |"); click(250, 224); click(650, 362)
            click(550, 456); screen("Game over")
            click(550, 566); screen("Playing"); key("Escape"); screen("Paused")
            click(350, 566); screen("Main menu")
            check(True, "mouse restart after actual wall collision")
            click(450, 334); screen("Settings | row 1")
            click(200, 153)
            key("Down"); screen("Settings | row 2")
            check(True, "gap click does not activate key capture")
            click(200, 125); key("Down"); screen("duplicate")
            check(True, "duplicate movement binding rejected in real UI")
            key("Escape"); click(400, 276)
            screen("Music 0 SFX 70")
            xdo("mousedown", "1"); xdo("mousemove", "--window", window, 820, 276); xdo("mouseup", "1")
            screen("Music 100 SFX 70")
            check(True, "drag beyond slider track clamps without changing other channel")
            click(200, 417); screen("Main menu")
            saved = json.loads((Path(work) / "config.json").read_text())
            check(saved["music_volume"] == 100 and saved["sfx_volume"] == 70, "mouse Save persists clamped independent volumes")
            try: key("Escape")
            except subprocess.CalledProcessError: pass
            process.wait(timeout=10)
            check(process.returncode == 0, "clean default-mode exit")
            check(not list(Path(work).glob("*probe*.json")), "default mode produces no probe files")
    print(f"{checks} desktop navigation/settings checks passed; target Windows/manual acceptance pending")
finally:
    for owned in (process, xvfb):
        if owned is not None and owned.poll() is None:
            owned.terminate()
            try: owned.wait(timeout=5)
            except subprocess.TimeoutExpired: owned.kill(); owned.wait()

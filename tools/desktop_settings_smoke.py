#!/usr/bin/env python3
"""Linux/Xvfb owner-task smoke test. Null audio is NOT audible acceptance.
Usage: python tools/desktop_settings_smoke.py /absolute/path/to/SnakeXtreme
Needs Xvfb, xdotool; use LD_LIBRARY_PATH for non-system OpenAL if necessary.
"""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time

exe = Path(sys.argv[1]).resolve()
assert exe.is_file(), exe
env = dict(os.environ, DISPLAY=":99", ALSOFT_DRIVERS="null", LIBGL_ALWAYS_SOFTWARE="1")
checks = 0
process = None
xvfb = None

def check(condition, message):
    global checks
    if not condition:
        raise AssertionError(message)
    checks += 1
    print("PASS:", message, flush=True)

def xdo(*args):
    return subprocess.check_output(["xdotool", *map(str, args)], env=env, text=True, stderr=subprocess.DEVNULL).strip()

def wait_for(predicate, message, seconds=12):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        try:
            if predicate():
                return
        except (subprocess.CalledProcessError, FileNotFoundError, json.JSONDecodeError):
            pass
        time.sleep(0.05)
    raise AssertionError(message)

try:
    # Do not attach to or kill someone else's display/session.
    assert not Path("/tmp/.X11-unix/X99").exists(), "Display :99 is already in use"
    xvfb = subprocess.Popen(["Xvfb", ":99", "-screen", "0", "1024x768x24"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    wait_for(lambda: Path("/tmp/.X11-unix/X99").exists(), "Xvfb did not start")
    with tempfile.TemporaryDirectory(prefix="snake-desktop-settings-") as directory:
        root = Path(directory)
        config = root / "config.json"
        log = root / "runtime.log"
        def launch(driver="null"):
            global process
            active_env = dict(env, ALSOFT_DRIVERS=driver)
            with log.open("ab") as output:
                process = subprocess.Popen([str(exe)], cwd=root, env=active_env, stdout=output, stderr=output)
            ids = []
            def found():
                ids[:] = xdo("search", "--name", "^SnakeXtreme").splitlines()
                return bool(ids) and process.poll() is None
            wait_for(found, "application window did not appear")
            xdo("windowmove", ids[0], 50, 50)
            return ids[0]
        def title():
            return xdo("getwindowname", window)
        def wait_title(fragment):
            wait_for(lambda: fragment in title(), "title missing " + fragment)
        def keys(*values):
            xdo("key", "--window", window, "--clearmodifiers", "--delay", "50", *values)
        def repeated(key, count):
            for _ in range(count):
                keys(key)
        def stop():
            # The window may disappear after key-down, before xdotool sends
            # key-up. Only accept that race if the application exits cleanly.
            try: keys("Escape")
            except subprocess.CalledProcessError: pass
            process.wait(timeout=10)
            check(process.returncode == 0, "normal menu exit returns 0")
        window = launch()
        wait_title("Main menu")
        wait_for(config.exists, "default config not regenerated")
        initial = json.loads(config.read_text())
        check(initial["music_volume"] == initial["sfx_volume"] == 70, "missing config generates 70/70 defaults")
        keys("Down", "Down", "Return")
        wait_title("Settings | row 1 | Music 70 SFX 70 | Classic")
        check(True, "Settings opens without changing a row")
        repeated("Down", 4); repeated("Left", 14)
        keys("Down"); repeated("Right", 6); keys("F5")
        wait_title("Music 0 SFX 100")
        check(True, "desktop independent music 0 / SFX 100 preview")
        repeated("Down", 3); keys("Return"); wait_title("Main menu")
        saved = json.loads(config.read_text())
        check(saved["music_volume"] == 0 and saved["sfx_volume"] == 100, "Save persists independent channel volumes")
        keys("Return"); wait_title("Settings | row 1")
        repeated("Down", 4); repeated("Right", 20); keys("Down"); repeated("Left", 20)
        wait_title("Music 100 SFX 0")
        check(True, "desktop independent music 100 / SFX 0 preview")
        keys("Escape"); wait_title("Main menu")
        check(json.loads(config.read_text()) == saved, "Cancel does not overwrite saved configuration")
        keys("Return"); wait_title("Music 0 SFX 100")
        check(True, "reopen restores saved volume draft after Cancel")
        repeated("Down", 6); keys("Return"); wait_title("Neon")
        repeated("Down", 2); keys("Return"); wait_title("Main menu")
        check(json.loads(config.read_text())["theme"] == "Neon", "theme selection persists")
        keys("Return"); wait_title("Settings | row 1")
        for row, key in enumerate(("i", "k", "j", "l")):
            keys("Return", key)
            if row < 3: keys("Down")
        repeated("Down", 5); keys("Return"); wait_title("Main menu")
        expected = json.loads(config.read_text())
        check([expected[k] for k in ("key_up", "key_down", "key_left", "key_right")] == [8, 10, 9, 11],
              "desktop remap to I/K/J/L persists SFML key codes")
        # Force deterministic replacement failure without chmod/root assumptions:
        # a directory cannot be replaced by the settings file. Real saves are
        # isolated in this test's TemporaryDirectory.
        keys("Return"); wait_title("Settings | row 1")
        repeated("Down", 4); keys("Right"); wait_title("Music 5 SFX 100")
        config.unlink(); config.mkdir()
        repeated("Down", 4); keys("Return"); wait_title("E_WRITE_FAILED")
        check("Settings | row 9" in title(), "failed Save stays on Settings with visible error")
        check(config.is_dir() and not list(config.iterdir()), "failed Save preserves existing target directory")
        check(not list(root.glob("config.json.tmp-*")), "failed Save removes only its owned temporary file")
        keys("Escape"); wait_title("Main menu")
        config.rmdir(); config.write_text(json.dumps(expected))
        keys("Return"); wait_title("Music 0 SFX 100 | Neon")
        check(True, "Cancel after failed Save restores previously active values")
        check(json.loads(config.read_text()) == expected, "recovered fixture retains all saved settings")
        keys("Escape"); wait_title("Main menu")
        stop()
        window = launch(); wait_title("Main menu")
        keys("Down", "Down", "Return"); wait_title("Music 0 SFX 100 | Neon")
        check(json.loads(config.read_text()) == expected, "real application relaunch restores settings")
        if os.environ.get("SNAKE_SMOKE_SCREENSHOT"):
            from PIL import ImageGrab
            time.sleep(0.2)  # allow the next rendered frame, not an input acknowledgement measurement
            ImageGrab.grab(xdisplay=env["DISPLAY"]).save(os.environ["SNAKE_SMOKE_SCREENSHOT"])
        keys("Escape"); wait_title("Main menu"); stop()
        config.write_text("{broken")
        window = launch(); wait_title("Main menu")
        wait_for(lambda: json.loads(config.read_text()).get("theme") == "Classic", "corrupt config not repaired")
        recovered = json.loads(config.read_text())
        check(recovered["music_volume"] == recovered["sfx_volume"] == 70, "real startup regenerates corrupt config defaults")
        check("E_PARSE: defaults loaded" in log.read_text(), "corrupt-file warning emitted")
        stop()
        window = launch("invalid-driver-for-smoke-test"); wait_title("Main menu")
        wait_for(lambda: "playback unavailable; silent fallback" in log.read_text(), "silent fallback warning absent")
        keys("Down", "Down", "Return"); wait_title("Music 70 SFX 70 | Classic")
        check(True, "no audio backend leaves Settings usable (silent fallback)")
        keys("Escape"); wait_title("Main menu"); stop()
        print(f"{checks} Linux/Xvfb desktop Settings checks passed; null output, no audible or Windows acceptance.", flush=True)
finally:
    if process is not None and process.poll() is None:
        process.terminate()
        try: process.wait(timeout=5)
        except subprocess.TimeoutExpired: process.kill(); process.wait()
    if xvfb is not None:
        xvfb.terminate()
        try: xvfb.wait(timeout=5)
        except subprocess.TimeoutExpired: xvfb.kill(); xvfb.wait()

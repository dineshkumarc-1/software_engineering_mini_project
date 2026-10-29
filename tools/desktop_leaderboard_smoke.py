#!/usr/bin/env python3
"""Isolated Linux/Xvfb #21 integration checks; not Windows/manual acceptance."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time

exe = Path(sys.argv[1]).resolve()
env = dict(os.environ, DISPLAY=":95", ALSOFT_DRIVERS="null", LIBGL_ALWAYS_SOFTWARE="1")
checks = 0
process = xvfb = None


def xdo(*args):
    return subprocess.check_output(["xdotool", *map(str, args)], env=env, text=True,
                                   stderr=subprocess.DEVNULL).strip()


def wait(predicate, message, seconds=12):
    end = time.monotonic() + seconds
    while time.monotonic() < end:
        try:
            if predicate():
                return
        except (subprocess.CalledProcessError, OSError, json.JSONDecodeError):
            pass
        time.sleep(0.02)
    raise AssertionError(message)


def check(condition, message):
    global checks
    assert condition, message
    checks += 1
    print("PASS:", message, flush=True)


try:
    assert not Path("/tmp/.X11-unix/X95").exists(), "Display :95 occupied"
    xvfb = subprocess.Popen(["Xvfb", ":95", "-screen", "0", "1024x768x24", "-nolisten", "tcp"],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    wait(lambda: Path("/tmp/.X11-unix/X95").exists(), "Xvfb startup")
    with tempfile.TemporaryDirectory(prefix="snake-leaderboard-") as work:
        board = Path(work) / "leaderboard.json"
        def entries(count):
            return {"version": 1, "entries": [
                {"player_name": f"Player{i}", "score": 1000 - i * 10,
                 "game_mode": "Classic", "difficulty": "Medium", "date_achieved": "2026-10-28"}
                for i in range(count)]}
        with (Path(work) / "runtime.txt").open("w") as log:
            def launch():
                global process
                process = subprocess.Popen([str(exe)], cwd=work, env=env, stdout=log, stderr=log)
                ids = []
                def found():
                    ids[:] = xdo("search", "--name", "^SnakeXtreme").splitlines()
                    return bool(ids) and process.poll() is None
                wait(found, "app window missing")
                return ids[0]
            def stop(window):
                # Close through the app's menu, not X server forced destruction.
                xdo("key", "--window", window, "Escape")
                process.wait(timeout=10)
                check(process.returncode == 0, "clean app exit")
            board.write_text(json.dumps(entries(10)), encoding="utf-8")
            window = launch()
            title = lambda: xdo("getwindowname", window)
            key = lambda *keys: xdo("key", "--window", window, "--clearmodifiers", "--delay", "30", *keys)
            def screen(value):
                wait(lambda: value in title(), "screen missing " + value)
            def open_board():
                key("Down", "Return"); screen("| Leaderboard |")
            screen("Main menu"); open_board(); screen("| 10 entries")
            check(all(f"| {i+1}. Player{i} {1000-i*10}" in title() for i in range(10)),
                  "Top-10 table exposes rank name and descending score for every row")
            key("Escape"); screen("Main menu")
            check(True, "Escape returns to menu")
            key("Return"); screen("| Leaderboard |")
            xdo("mousemove", "--window", window, 350, 456); xdo("click", "1"); screen("Main menu")
            check(True, "mouse Back returns to menu")
            stop(window)

            # Make room for an actual completed game's score.
            board.write_text(json.dumps(entries(2)), encoding="utf-8")
            window = launch(); screen("Main menu")
            key("Return"); screen("| Name:")
            xdo("type", "--window", window, "--delay", "20", "Dinesh")
            screen("| Name: Dinesh |")
            key("Return"); screen("| Menu |")
            key("Return"); screen("Playing"); screen("Game over")
            screen("Score saved: rank")
            wait(lambda: len(json.loads(board.read_text())["entries"]) == 3, "score not persisted")
            saved = json.loads(board.read_text())
            check(saved["entries"][2]["player_name"] == "Dinesh", "completed session saves normalized player name")
            time.sleep(0.2)  # Observe several render frames after the one-shot save.
            check(json.loads(board.read_text()) == saved, "Game Over frames do not resubmit score")
            key("m"); screen("Main menu"); open_board(); screen("| 3 entries")
            check("| 3. Dinesh " in title(), "new score appears in the real UI table")
            key("Escape"); screen("Main menu"); stop(window)
            window = launch(); screen("Main menu"); open_board(); screen("| 3 entries")
            check("| 3. Dinesh " in title(), "saved leaderboard survives process restart")
            key("Escape"); screen("Main menu"); stop(window)

            board.write_text("{broken", encoding="utf-8")
            window = launch(); screen("Main menu"); open_board(); screen("| 0 entries")
            check("E_LEADERBOARD_READ" in title() and process.poll() is None,
                  "corrupt board displays empty table and warning without crashing")
            key("Escape"); screen("Main menu"); stop(window)
    print(f"{checks} leaderboard integration checks passed; Windows/manual acceptance pending")
finally:
    for owned in (process, xvfb):
        if owned is not None and owned.poll() is None:
            owned.terminate()
            try:
                owned.wait(timeout=5)
            except subprocess.TimeoutExpired:
                owned.kill(); owned.wait()

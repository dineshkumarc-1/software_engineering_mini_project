#!/usr/bin/env python3
"""App-level leaderboard checks on the real game (issue #18).

STP TC-LB-01 (5 real games, leaderboard.json inspected, restart), TC-LB-02
(10-entry and empty boards on the Leaderboard screen with all six columns,
Esc and the Back button), TC-LB-04
(missing / corrupt boards, then a real game's score) and TC-SEC-04 at app
level (a real game's score cannot be saved because the folder is read-only;
needs a normal user). Linux only: Xvfb, xdotool (XTEST key events) and Pillow
for screenshots.

    python3 tools/leaderboard_ui_check.py BUILD/SnakeXtreme OUTPUT_DIR [01,02,04,sec04]

Every run uses fresh temporary working directories, so nothing outside them
is read or written. Screenshots and copies of every leaderboard.json go to
OUTPUT_DIR. One line per check; a known product defect is shown next to the
check it breaks. Exit code 0 only if every check passes.

The game is driven only through its window: keys, the window title (which
mirrors the screen state) and screenshots of the board, which a small pilot
reads to steer the snake to food and so produce different scores.
"""
import collections
import datetime
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import time

from PIL import ImageGrab

EXE = Path(sys.argv[1]).resolve()
OUT = Path(sys.argv[2]).resolve()
CORPUS = Path(__file__).resolve().parent.parent / "tests" / "fixtures" / "json-corpus" / "leaderboard"
DISPLAY = ":94"
ENV = dict(os.environ, DISPLAY=DISPLAY, ALSOFT_DRIVERS="null", LIBGL_ALWAYS_SOFTWARE="1")
FIELDS = {"player_name", "score", "game_mode", "difficulty", "date_achieved"}
results = []


def check(name, ok, detail="", defect=None):
    results.append(bool(ok))
    tag = "PASS " if ok else ("FAIL (%s) " % defect if defect else "FAIL ")
    print(tag + name + (" - " + detail if detail else ""), flush=True)


def wait(predicate, seconds=10.0):
    end = time.monotonic() + seconds
    while time.monotonic() < end:
        try:
            if predicate():
                return True
        except (subprocess.CalledProcessError, OSError):
            pass
        time.sleep(0.03)
    return False


def xdo(*args):
    return subprocess.run(["xdotool", *map(str, args)], env=ENV, capture_output=True,
                          text=True).stdout.strip()


# --- Board reading and a small pilot (Classic theme colours) ----------------
COLS, ROWS, CELL = 30, 20, 30
HEAD, BODY, FOOD, OBSTACLE = (63, 215, 155), (35, 160, 115), (255, 90, 90), (190, 135, 70)
ITEMS = ((70, 170, 255), (255, 205, 60), (200, 120, 255))
BACKGROUND = (15, 23, 32)
DIRS = {"Right": (1, 0), "Left": (-1, 0), "Up": (0, -1), "Down": (0, 1)}
OPPOSITE = {"Right": "Left", "Left": "Right", "Up": "Down", "Down": "Up"}


def near(p, c, tol=14):
    return all(abs(p[i] - c[i]) <= tol for i in range(3))


def read_board(img):
    px = img.load()
    board = {"head": [], "blocked": set(), "food": [], "open_top": set()}
    for y in range(ROWS):
        for x in range(COLS):
            cx, cy = x * CELL + CELL // 2, y * CELL + CELL // 2
            p = px[cx, cy]
            # A cell counts only if its centre and points 6 px to the side and
            # below have the colour, so thin HUD outlines and text are never
            # taken for it, and a cell just under a HUD panel is still read.
            solid = lambda c: all(near(px[cx + dx, cy + dy], c) for dx in (-6, 0, 6) for dy in (0, 6))
            if solid(HEAD):
                board["head"].append((x, y))
            elif solid(BODY) or solid(OBSTACLE):
                board["blocked"].add((x, y))
            elif solid(FOOD):
                board["food"].append((x, y))
            elif y == 0 and near(p, BACKGROUND, 2):
                # Row 0 is free only where no HUD panel covers it.
                board["open_top"].add((x, y))
            if any(near(px[cx - 9, cy], c) for c in ITEMS):
                board["blocked"].add((x, y))  # avoid power-ups: keep scores at +10
    return board


def free_cells(board):
    blocked = board["blocked"] | set(board["head"])
    return lambda c: (0 <= c[0] < COLS and 0 <= c[1] < ROWS and c not in blocked
                      and (c[1] > 0 or c in board["open_top"]))


def next_step(start, goal, is_free):
    prev = {start: None}
    queue = collections.deque([start])
    while queue:
        c = queue.popleft()
        if c == goal:
            while prev[c] != start:
                c = prev[c]
            return c
        for d in DIRS.values():
            n = (c[0] + d[0], c[1] + d[1])
            if n not in prev and (is_free(n) or n == goal):
                prev[n] = c
                queue.append(n)
    return None


def room(start, is_free, limit=250):
    seen, queue = {start}, collections.deque([start])
    while queue and len(seen) < limit:
        c = queue.popleft()
        for d in DIRS.values():
            n = (c[0] + d[0], c[1] + d[1])
            if n not in seen and is_free(n):
                seen.add(n)
                queue.append(n)
    return len(seen)


# --- The application ----------------------------------------------------------
class App:
    def __init__(self, work, label):
        ensure_display()
        self.work, self.label = Path(work), label
        self.log = open(self.work / "runtime.log", "a")
        self.proc = subprocess.Popen([str(EXE)], cwd=self.work, env=ENV, stdout=self.log, stderr=self.log)
        self.menu = 0  # main-menu index; the game keeps it while it runs
        ids = []
        assert wait(lambda: ids.extend(xdo("search", "--name", "^SnakeXtreme").split()) or ids, 15), "no window"
        self.win = ids[0]
        xdo("windowmove", self.win, 0, 0)
        xdo("windowfocus", "--sync", self.win)
        assert wait(lambda: "Main menu" in self.title(), 10), "main menu missing"

    def title(self):
        return xdo("getwindowname", self.win)

    def key(self, *names):
        # XTEST events to the focused game window (the only window on the
        # display), like a real keyboard.
        xdo("key", "--clearmodifiers", "--delay", "40", *names)

    def press(self, name, text, tries=3):
        """Press `name` until the title shows `text`. A repeat is reported:
        it means a key event was lost (seen now and then on Xvfb)."""
        for attempt in range(tries):
            self.key(name)
            if self.screen(text, 3):
                if attempt:
                    print("  note: %s needed %d presses to reach %r" % (name, attempt + 1, text))
                return True
        return False

    def screen(self, text, seconds=10.0):
        return wait(lambda: text in self.title(), seconds)

    def shot(self, name):
        out = subprocess.run(["xwininfo", "-id", self.win], env=ENV, capture_output=True, text=True).stdout
        geo = {k: int(v) for k, v in re.findall(r"(Absolute upper-left X|Absolute upper-left Y|Width|Height):\s+(-?\d+)", out)}
        x, y = geo["Absolute upper-left X"], geo["Absolute upper-left Y"]
        img = ImageGrab.grab(bbox=(x, y, x + geo["Width"], y + geo["Height"]), xdisplay=DISPLAY).convert("RGB")
        if name:
            img.save(OUT / name)
        return img

    def main_item(self, index):
        # A mouse click on the item (UiHitTest: 380 x 48 buttons from y 190,
        # 60 px apart) selects and confirms it in one step.
        xdo("mousemove", "--window", self.win, 450, 214 + 60 * index)
        xdo("click", "1")
        self.menu = index

    def open_leaderboard(self):
        self.main_item(1)
        return self.screen("| Leaderboard |")

    def configure(self, mode, difficulty):
        wanted = "| Menu | %s / %s" % (mode, difficulty)
        for _ in range(4):
            if mode in self.title().split("| Menu |")[1].split("/")[0]:
                break
            self.key("Right")
            time.sleep(0.1)
        for step in ("Up", "Up", "Down", "Down"):
            if wanted in self.title():
                break
            current = self.title().split("/")[1]
            order = ["Easy", "Medium", "Hard"]
            have = next((d for d in order if d in current), "Medium")
            self.key("Up" if order.index(difficulty) > order.index(have) else "Down")
            time.sleep(0.1)
        return wanted in self.title()

    def play(self, name, mode, difficulty, foods):
        """New Game -> name -> mode/difficulty -> play: eat `foods` foods, then
        go straight into a wall. Returns (score, game-over title)."""
        self.main_item(0)
        assert self.screen("| Name:"), self.title()
        xdo("type", "--delay", "30", name)
        assert self.screen("| Name: %s |" % name), self.title()
        assert self.press("Return", "| Menu |"), self.title()
        assert self.configure(mode, difficulty), self.title()
        assert self.press("Return", "Playing"), self.title()
        head, direction, score = None, "Right", 0
        trace = open(OUT / ("pilot-%s.log" % name.lower()), "a")
        trace.write("--- %s %s / %s, %d foods\n" % (name, mode, difficulty, foods))
        end = time.monotonic() + 120
        while time.monotonic() < end and "Game over" not in self.title():
            board = read_board(self.shot(None))
            found = re.search(r"Score (\d+)", self.title())
            score = int(found[1]) if found else score
            if len(board["head"]) != 1 or board["head"][0] == head:
                time.sleep(0.005)
                continue
            new = board["head"][0]
            if head is not None:
                direction = next((k for k, d in DIRS.items() if (head[0] + d[0], head[1] + d[1]) == new), direction)
            head = new
            is_free = free_cells(board)
            ahead = lambda k: (head[0] + DIRS[k][0], head[1] + DIRS[k][1])
            if score >= foods * (15 if mode == "Obstacle" else 10):
                # Done: head for the wall on a line with no food on it.
                def clear(k):
                    c = ahead(k)
                    while 0 <= c[0] < COLS and 0 <= c[1] < ROWS:
                        if c in board["food"]:
                            return False
                        c = (c[0] + DIRS[k][0], c[1] + DIRS[k][1])
                    return True
                lines = [k for k in (direction, *DIRS) if k != OPPOSITE[direction] and clear(k)]
                choice = lines[0] if lines else direction
            else:
                goal = board["food"][0] if board["food"] else None
                step = next_step(head, goal, is_free) if goal else None
                choice = None
                if step is not None and room(step, lambda c: is_free(c) and c != head) > len(board["blocked"]) // 2 + 4:
                    choice = next(k for k in DIRS if ahead(k) == step)
                if choice is None:
                    options = [(room(ahead(k), is_free), k) for k in DIRS
                               if k != OPPOSITE[direction] and is_free(ahead(k))]
                    choice = max(options)[1] if options else direction
            trace.write("%.2f head=%s dir=%s food=%s score=%d choice=%s\n" % (
                time.monotonic(), head, direction, board["food"], score, choice))
            if choice not in (direction, OPPOSITE[direction]):
                xdo("key", choice)
        trace.close()
        assert self.screen("Game over", 30), self.title()
        time.sleep(0.3)
        over = self.title()
        return int(re.search(r"Score (\d+)", over)[1]), over

    def to_main_menu(self):
        if "Main menu" in self.title():
            return True
        if any(state in self.title() for state in ("Game over", "| Paused", "| Playing")):
            return self.press("m", "Main menu")
        return self.press("Escape", "Main menu")

    def quit(self):
        self.to_main_menu()
        for _ in range(3):
            # Esc on the main menu exits; repeat it if a key was lost.
            self.key("Escape")
            try:
                self.proc.wait(5)
                break
            except subprocess.TimeoutExpired:
                print("  note: app still running after Esc on: %s" % self.title()[:80])
        if self.proc.poll() is None:
            self.proc.kill()
            self.proc.wait()
        self.log.close()
        return self.proc.returncode


def entries_in_title(title):
    return re.findall(r"\| (\d+)\. (.+?) (\d+)(?= \|)", title)


def fields_in_title(title):
    """Rows with all six columns, as the window title mirrors them once the
    Leaderboard screen shows mode, difficulty and date (D7):
    "| 1. Name 900 | Timed / Easy / 2026-10-11"."""
    return [(int(r), n, int(s), m, d, day) for r, n, s, m, d, day in re.findall(
        r"\| (\d+)\. (.+?) (\d+) \| (\w+) / (\w+) / (\d{4}-\d{2}-\d{2})(?= \|)", title)]


def temp_files(work):
    return sorted(p.name for p in Path(work).iterdir() if ".tmp." in p.name)


# --- Runs -------------------------------------------------------------------------
OUT.mkdir(parents=True, exist_ok=True)
assert not Path("/tmp/.X11-unix/X94").exists(), "display :94 is in use"
xvfb = None


def ensure_display():
    """(Re)start Xvfb on :94 if it is not running."""
    global xvfb
    if xvfb is not None and xvfb.poll() is None and subprocess.run(
            ["xdpyinfo"], env=ENV, capture_output=True).returncode == 0:
        return
    if xvfb is not None:
        print("  note: Xvfb exited (code %s); restarted" % xvfb.poll())
        if xvfb.poll() is None:
            xvfb.kill()
            xvfb.wait()
    xvfb = subprocess.Popen(["Xvfb", DISPLAY, "-screen", "0", "1024x768x24", "-nolisten", "tcp", "-noreset"],
                            stdout=subprocess.DEVNULL, stderr=open(OUT / "xvfb.log", "a"))
    assert wait(lambda: subprocess.run(["xdpyinfo"], env=ENV, capture_output=True).returncode == 0, 10), \
        "Xvfb did not start"


SECTIONS = sys.argv[3].split(",") if len(sys.argv) > 3 else ["01", "02", "04", "sec04"]
today = {datetime.date.today().isoformat(), datetime.datetime.now(datetime.timezone.utc).date().isoformat()}
try:
    if "01" in SECTIONS:
        # TC-LB-01: empty leaderboard, temporary data directory, 5 real games.
        print("== TC-LB-01 persist Top 10 with all fields")
        # Food is worth 10 points in Classic and Timed, 15 in Obstacle (FR-016),
        # so the targets give 30, 10, 15, 0 and 45 points.
        games = [("Ann", "Classic", "Easy", 3), ("Ben", "Timed", "Medium", 1), ("Cara", "Obstacle", "Hard", 1),
                 ("Dev", "Classic", "Hard", 0), ("Eli", "Obstacle", "Easy", 3)]
        for attempt in range(1, 4):
            # The pilot can crash early; if two games end on the same score,
            # the whole case is repeated in a new empty directory.
            work = Path(tempfile.mkdtemp(prefix="snake-lb01-"))
            played, overs = {}, {}
            app = App(work, "lb01")
            for name, mode, difficulty, foods in games:
                score, overs[name] = app.play(name, mode, difficulty, foods)
                played[name] = (score, mode, difficulty)
                print("  attempt %d, game %s: %s / %s, score %d - %s" % (
                    attempt, name, mode, difficulty, score, overs[name].split("Game over: ")[1][:90]))
                if name == "Eli":
                    app.shot("lb01-game-over-eli.png")
                app.to_main_menu()
            code = app.quit()
            if len({s for s, _, _ in played.values()}) == 5:
                break
        for name in played:
            check("TC-LB-01: %s's game over screen reports the save" % name, "Score saved: rank" in overs[name],
                  overs[name][-80:])
        check("TC-LB-01: the 5 games have different scores", len({s for s, _, _ in played.values()}) == 5,
              str({n: s for n, (s, _, _) in played.items()}))
        check("TC-LB-01: the app exits normally", code == 0, "exit code %s" % code)
        board_path = work / "leaderboard.json"
        raw = board_path.read_bytes() if board_path.exists() else b""
        shutil.copyfile(board_path, OUT / "lb01-leaderboard.json") if raw else None
        try:
            data = json.loads(raw)
        except ValueError:
            data = None
        check("TC-LB-01: leaderboard.json is valid JSON", data is not None, "%d bytes" % len(raw))
        entries = (data or {}).get("entries", [])
        check("TC-LB-01: version 1 and all 5 games stored", (data or {}).get("version") == 1 and len(entries) == 5,
              "%d entries" % len(entries))
        scores = [e.get("score") for e in entries]
        check("TC-LB-01: sorted by score, descending", scores == sorted(scores, reverse=True), str(scores))
        check("TC-LB-01: every entry has exactly the 5 Appendix B fields",
              entries and all(set(e) == FIELDS for e in entries), str(sorted({k for e in entries for k in e})))
        check("TC-LB-01: name, score, mode and difficulty match each game played",
              entries and all(played.get(e.get("player_name")) == (e.get("score"), e.get("game_mode"), e.get("difficulty"))
                              for e in entries) and len({e.get("player_name") for e in entries}) == 5,
              "; ".join("%s %s %s/%s" % (e.get("player_name"), e.get("score"), e.get("game_mode"), e.get("difficulty"))
                        for e in entries))
        dates = [e.get("date_achieved") for e in entries]
        check("TC-LB-01: date_achieved is today's date as YYYY-MM-DD",
              dates and all(isinstance(d, str) and re.fullmatch(r"\d{4}-\d{2}-\d{2}", d) and d in today for d in dates),
              str(sorted(set(dates))), defect="D4")
        check("TC-LB-01: no temporary file is left", not temp_files(work), str(temp_files(work)))
        app = App(work, "lb01-restart")
        app.open_leaderboard()
        shown = entries_in_title(app.title())
        app.shot("lb01-after-restart.png")
        check("TC-LB-01: after a restart the Leaderboard shows the 5 saved games in order",
              [(n, int(s)) for _, n, s in shown] == [(e.get("player_name"), e.get("score")) for e in entries],
              app.title()[:200])
        check("TC-LB-01: after a restart every row also shows its mode, difficulty and date (D7)",
              fields_in_title(app.title()) ==
              [(i + 1, e.get("player_name"), e.get("score"), e.get("game_mode"), e.get("difficulty"),
                e.get("date_achieved")) for i, e in enumerate(entries)], app.title()[:260], defect="D7")
        check("TC-LB-01: the app exits normally after the restart", app.quit() == 0)
        check("TC-LB-01: viewing the board does not rewrite the file", board_path.read_bytes() == raw)

    if "02" in SECTIONS:
        # TC-LB-02: 10-entry board (stored unsorted, all modes and difficulties) and empty board.
        print("== TC-LB-02 view leaderboard")
        work = Path(tempfile.mkdtemp(prefix="snake-lb02-"))
        modes, levels = ("Classic", "Timed", "Obstacle"), ("Easy", "Medium", "Hard")
        ten = [{"player_name": name, "score": score, "game_mode": modes[i % 3], "difficulty": levels[(i // 3) % 3],
                "date_achieved": "2026-10-%02d" % (10 + i)}
               for i, (name, score) in enumerate([("Mid_500", 500), ("Top-9999999", 9999999), ("Low 10", 10),
                                                  ("FifteenCharsXYZ", 750), ("B", 300), ("Tie_A", 200),
                                                  ("Tie_B", 200), ("Zero", 0), ("Nine00", 900), ("Hundred", 100)])]
        (work / "leaderboard.json").write_text(json.dumps({"version": 1, "entries": ten}, indent=2), encoding="utf-8")
        shutil.copyfile(work / "leaderboard.json", OUT / "lb02-ten-entries-fixture.json")
        app = App(work, "lb02")
        check("TC-LB-02: Leaderboard opens from the main menu", app.open_leaderboard(), app.title()[:120])
        time.sleep(0.3)
        app.shot("lb02-ten-entries.png")
        shown = entries_in_title(app.title())
        expected = sorted(ten, key=lambda e: -e["score"])
        check("TC-LB-02: 10 rows with rank, name and score in descending order",
              [(int(r), n, int(s)) for r, n, s in shown] ==
              [(i + 1, e["player_name"], e["score"]) for i, e in enumerate(expected)], app.title()[:260])
        check("TC-LB-02: equal scores keep their stored order (Tie_A before Tie_B)",
              [n for _, n, _ in shown].index("Tie_A") < [n for _, n, _ in shown].index("Tie_B"))
        check("TC-LB-02: all 10 rows show rank, name, score, mode, difficulty and date (D7)",
              fields_in_title(app.title()) ==
              [(i + 1, e["player_name"], e["score"], e["game_mode"], e["difficulty"], e["date_achieved"])
               for i, e in enumerate(expected)], app.title()[:400], defect="D7")
        check("TC-LB-02: Esc returns to the main menu", app.press("Escape", "Main menu"), app.title()[:80])
        # The Back button (UiHitTest: 260..440 x 434..478) returns by mouse too.
        app.main_item(1)
        app.screen("| Leaderboard |")
        xdo("mousemove", "--window", app.win, 350, 456)
        xdo("click", "1")
        check("TC-LB-02: the Back button returns to the main menu", app.screen("Main menu"), app.title()[:80])
        check("TC-LB-02: the app exits normally", app.quit() == 0)
        work = Path(tempfile.mkdtemp(prefix="snake-lb02-empty-"))
        shutil.copyfile(CORPUS / "valid" / "01-empty-board.json", work / "leaderboard.json")
        app = App(work, "lb02-empty")
        app.open_leaderboard()
        time.sleep(0.3)
        app.shot("lb02-empty-board.png")
        check("TC-LB-02: the empty board shows no rows and no warning",
              "| 0 entries |" in app.title() and "E_LEADERBOARD" not in app.title(), app.title()[:120])
        check("TC-LB-02: Esc returns to the main menu from the empty board", app.press("Escape", "Main menu"))
        check("TC-LB-02: the app exits normally", app.quit() == 0)
        print("  The screenshots lb02-*.png are inspected by eye for the columns and \"No scores yet\".")

    if "04" in SECTIONS:
        # TC-LB-04: missing and corrupt boards, then a real game's score.
        print("== TC-LB-04 missing/corrupt leaderboard fallback")
        fixtures = [("missing", None, None), ("empty", "01-empty-file.json", None),
                    ("truncated", "03-truncated.json", None), ("wrong type", "14-score-as-string.json", "D3"),
                    ("negative score", "15-negative-score.json", "D3"), ("bad enum", "18-unknown-game-mode.json", "D3"),
                    ("11 entries", "24-eleven-entries.json", "D2")]
        for label, fixture, defect in fixtures:
            work = Path(tempfile.mkdtemp(prefix="snake-lb04-"))
            board_path = work / "leaderboard.json"
            if fixture:
                shutil.copyfile(CORPUS / "malformed" / fixture, board_path)
            before = board_path.read_bytes() if fixture else None
            app = App(work, "lb04")
            app.open_leaderboard()
            time.sleep(0.3)
            title = app.title()
            app.shot("lb04-%s.png" % label.replace(" ", "-"))
            alive = app.proc.poll() is None
            check("TC-LB-04 %s: no crash and an empty leaderboard is shown" % label,
                  alive and "| 0 entries |" in title, title[:120], defect="D2" if defect == "D2" else None)
            if fixture:
                check("TC-LB-04 %s: a warning is shown on the Leaderboard screen" % label,
                      "E_LEADERBOARD_READ" in title, title[-90:], defect="D2" if defect == "D2" else None)
            assert app.press("Escape", "Main menu"), app.title()
            score, over = app.play("Valid", "Classic", "Medium", 1)
            check("TC-LB-04 %s: the next real game's score (%d) is saved" % (label, score),
                  "Score saved: rank 1" in over, over.split(" | ")[-2] if " | " in over else over, defect=defect)
            code = app.quit()
            check("TC-LB-04 %s: the app exits normally" % label, code == 0, "exit code %s" % code)
            after = board_path.read_bytes() if board_path.exists() else b""
            try:
                stored = json.loads(after).get("entries", [])
            except ValueError:
                stored = None
            check("TC-LB-04 %s: the bad file is replaced by a valid board holding only that score" % label,
                  stored is not None and [(e.get("player_name"), e.get("score")) for e in stored] == [("Valid", score)],
                  "file %s" % ("unchanged" if after == before else "now %d entries" % len(stored) if stored is not None
                               else "invalid"), defect=defect)
            check("TC-LB-04 %s: no temporary file is left" % label, not temp_files(work), str(temp_files(work)))
            log = (work / "runtime.log").read_text(errors="replace")
            warned = [line for line in log.splitlines() if "leaderboard" in line.lower()]
            print("  runtime log lines about the leaderboard: %s" % (warned or "none"))
    if "sec04" in SECTIONS:
        # TC-SEC-04 at app level: the save of a real game's score fails
        # because the data directory cannot be written (a real refusal, not
        # a simulation). Needs a normal user: root ignores the permission.
        print("== TC-SEC-04 app level: a refused save keeps the old board")
        if os.geteuid() == 0:
            print("SKIP TC-SEC-04 app level: run as a normal user (root ignores directory permissions)")
        else:
            work = Path(tempfile.mkdtemp(prefix="snake-sec04-"))
            board_path = work / "leaderboard.json"
            board_path.write_text(json.dumps({"version": 1, "entries": [
                {"player_name": "Old%d" % s, "score": s, "game_mode": "Classic", "difficulty": "Medium",
                 "date_achieved": "2026-10-28"} for s in (300, 200, 100)]}, indent=2), encoding="utf-8")
            app = App(work, "sec04-setup")  # creates config.json while the folder is writable
            app.quit()
            before = board_path.read_bytes()
            work.chmod(0o555)
            try:
                app = App(work, "sec04")
                score, over = app.play("Locked", "Classic", "Medium", 1)
                app.shot("sec04-refused-save.png")
                check("TC-SEC-04 app level: the game over screen reports that the score was not saved",
                      "E_LEADERBOARD_SAVE" in over, over.split(" | ")[-2] if " | " in over else over)
                app.to_main_menu()
                app.open_leaderboard()
                check("TC-SEC-04 app level: the Leaderboard still shows the old board",
                      [(n, int(v)) for _, n, v in entries_in_title(app.title())] ==
                      [("Old300", 300), ("Old200", 200), ("Old100", 100)], app.title()[:140])
                code = app.quit()
                check("TC-SEC-04 app level: the app exits normally", code == 0, "exit code %s" % code)
                check("TC-SEC-04 app level: leaderboard.json is unchanged, byte for byte",
                      board_path.read_bytes() == before)
                check("TC-SEC-04 app level: no temporary file is left", not temp_files(work), str(temp_files(work)))
                log = (work / "runtime.log").read_text(errors="replace")
                check("TC-SEC-04 app level: the failure is logged", "[WARN] E_LEADERBOARD_SAVE" in log,
                      "; ".join(line for line in log.splitlines() if "LEADERBOARD" in line)[:160])
            finally:
                work.chmod(0o755)

finally:
    if xvfb is not None:
        xvfb.terminate()
        xvfb.wait(5)

print("SUMMARY %d/%d passed" % (sum(results), len(results)))
sys.exit(0 if all(results) else 1)

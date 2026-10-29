#!/usr/bin/env python3
"""Runtime checks of the power-ups in the real game (issues #14 and #18).

STP TC-PWR-01 to TC-PWR-04 at system level: plays Classic / Medium on Xvfb with
real XTEST key presses, reads the board from screenshots and the state from
the window title, steers to every power-up it sees and checks what each one
does (Speed 72 ms for 5 s, then 120 ms; x2 gives +20 per food for 10 s and its
time stands still while paused; Shrink takes one segment, never below 3).
Prints one line per check and saves screenshots. Linux only: Xvfb, xdotool,
xwininfo and Pillow.

    python3 tools/powerup_runtime_check.py BUILD/SnakeXtreme OUTPUT_DIR [SECONDS]

The game runs in a new temporary directory, so no real settings or scores are
touched. Power-ups under the HUD panels in the top corners cannot be seen by
this harness; the 8-15 s spawn rule itself is covered by powerup_tests.
"""
import collections, os, re, subprocess, sys, tempfile, time
from PIL import ImageGrab

DISPLAY = os.environ.get("XDRV_DISPLAY", ":92")


class Game:
    def __init__(self, binary, display=DISPLAY, cwd=None):
        self.display = display
        self.env = dict(os.environ, DISPLAY=display, ALSOFT_DRIVERS="null", LIBGL_ALWAYS_SOFTWARE="1")
        self.xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", "1024x768x24", "-nolisten", "tcp", "-noreset"],
                                     stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(0.8)
        self.log = open(os.path.join(cwd or ".", "runtime.log"), "w")
        self.proc = subprocess.Popen([binary], cwd=cwd, env=self.env, stdout=self.log, stderr=self.log)
        self.wid = None
        for _ in range(100):
            r = subprocess.run(["xdotool", "search", "--name", "SnakeXtreme"], env=self.env, capture_output=True, text=True)
            if r.stdout.strip():
                self.wid = r.stdout.split()[0]
                break
            time.sleep(0.1)
        assert self.wid, "game window did not appear"
        subprocess.run(["xdotool", "windowmove", self.wid, "0", "0"], env=self.env)
        subprocess.run(["xdotool", "windowfocus", "--sync", self.wid], env=self.env)
        time.sleep(0.4)

    def key(self, *names, delay=0.06):
        for n in names:
            subprocess.run(["xdotool", "key", "--clearmodifiers", n], env=self.env)
            time.sleep(delay)

    def title(self):
        return subprocess.run(["xdotool", "getwindowname", self.wid], env=self.env,
                              capture_output=True, text=True).stdout.strip()

    def geometry(self):
        out = subprocess.run(["xwininfo", "-id", self.wid], env=self.env, capture_output=True, text=True).stdout
        vals = {}
        for line in out.splitlines():
            line = line.strip()
            for k in ("Absolute upper-left X:", "Absolute upper-left Y:", "Width:", "Height:"):
                if line.startswith(k):
                    vals[k] = int(line.split(":")[1])
        return vals["Absolute upper-left X:"], vals["Absolute upper-left Y:"], vals["Width:"], vals["Height:"]

    def grab(self):
        x, y, w, h = self.geometry()
        return ImageGrab.grab(bbox=(x, y, x + w, y + h), xdisplay=self.display).convert("RGB")

    def alive(self):
        return self.proc.poll() is None

    def close(self):
        if self.alive():
            self.proc.terminate()
            try:
                self.proc.wait(5)
            except subprocess.TimeoutExpired:
                self.proc.kill()
        self.log.close()
        self.xvfb.terminate()
        self.xvfb.wait(5)


BIN = sys.argv[1]
OUT = sys.argv[2] if len(sys.argv) > 2 else '.'
PLAY_SECONDS = float(sys.argv[3]) if len(sys.argv) > 3 else 150
COLS, ROWS, CELL = 30, 20, 30
HEAD, BODY, FOOD, OBST = (63, 215, 155), (35, 160, 115), (255, 90, 90), (190, 135, 70)
ITEM = {'Speed': (70, 170, 255), 'Multiplier': (255, 205, 60), 'Shrink': (200, 120, 255)}
results = []
log = []

def check(name, ok, detail=''):
    results.append((name, bool(ok), detail))
    print(('PASS ' if ok else 'FAIL ') + name + (' - ' + detail if detail else ''), flush=True)

def near(p, c, tol=14):
    return all(abs(p[i] - c[i]) <= tol for i in range(3))

def board(img):
    """Snake, food and power-up cells. Row 0 counts only where no HUD panel
    covers it. A cell needs its colour on a 13 x 13 area around its centre,
    so HUD outlines and text are never taken for the snake or food."""
    px = img.load()
    b = {'head': [], 'body': [], 'food': [], 'obst': [], 'item': [], 'open_top': set()}
    for y in range(0, ROWS):
        for x in range(COLS):
            cx, cy = x * CELL + CELL // 2, y * CELL + CELL // 2
            p = px[cx, cy]
            if y == 0 and near(p, (15, 23, 32), 2):
                b['open_top'].add((x, y))
            for k, c in (('head', HEAD), ('body', BODY), ('food', FOOD), ('obst', OBST)):
                if all(near(px[cx + dx, cy + dy], c) for dx in (-6, 0, 6) for dy in (-6, 0, 6)):
                    b[k].append((x, y))
            q = px[cx - 9, cy]
            for name, c in ITEM.items():
                if near(q, c):
                    b['item'].append(((x, y), name))
    return b

def parse_title(t):
    d = {'raw': t}
    m = re.search(r'Score (\d+) \| High (\d+)', t)
    if m:
        d['score'], d['high'] = int(m.group(1)), int(m.group(2))
    m = re.search(r'\| (Speed boost|Score x2) (\d+)\.(\d)s', t)
    if m:
        d['effect'] = 'Speed' if m.group(1) == 'Speed boost' else 'Multiplier'
        d['effect_left'] = int(m.group(2)) + int(m.group(3)) / 10
    for state in ('Menu', 'Playing', 'Paused'):
        if '| ' + state + ' |' in t:
            d['state'] = state
    if 'Game over' in t:
        d['state'] = 'GameOver'
    return d

DIRS = {'Right': (1, 0), 'Left': (-1, 0), 'Up': (0, -1), 'Down': (0, 1)}
OPP = {'Right': 'Left', 'Left': 'Right', 'Up': 'Down', 'Down': 'Up'}

def is_free_fn(b):
    blocked = set(b['obst']) | set(b['body']) | set(b['head'])
    return lambda c: (0 <= c[0] < COLS and 0 <= c[1] < ROWS and c not in blocked
                      and (c[1] >= 1 or c in b['open_top']))

def bfs(start, goal, is_free):
    prev = {start: None}
    q = collections.deque([start])
    while q:
        c = q.popleft()
        if c == goal:
            while prev[c] != start:
                c = prev[c]
            return c
        for d in DIRS.values():
            n = (c[0] + d[0], c[1] + d[1])
            if n not in prev and (is_free(n) or n == goal):
                prev[n] = c
                q.append(n)
    return None

def flood(start, is_free, limit=250):
    seen, q = {start}, collections.deque([start])
    while q and len(seen) < limit:
        c = q.popleft()
        for d in DIRS.values():
            n = (c[0] + d[0], c[1] + d[1])
            if n not in seen and is_free(n):
                seen.add(n)
                q.append(n)
    return len(seen)

def step_dir(a, b):
    for k, d in DIRS.items():
        if (a[0] + d[0], a[1] + d[1]) == b:
            return k
    return None

class Pilot:
    def __init__(self, g):
        self.g, self.head, self.dir, self.moves = g, None, 'Right', []
        self.last = None

    def wait_move(self, timeout=1.0):
        end = time.time() + timeout
        while time.time() < end:
            img = self.g.grab()
            b = board(img)
            if len(b['head']) == 1 and b['head'][0] != self.head:
                new = b['head'][0]
                if self.head is not None:
                    d = step_dir(self.head, new)
                    if d:
                        self.dir = d
                    else:
                        log.append('jump %s -> %s' % (self.head, new))
                self.head = new
                self.moves.append(time.time())
                self.last = (img, b)
                return b
            time.sleep(0.002)
        return None

    def choose(self, b, goal):
        if self.head is None:
            return None
        is_free = is_free_fn(b)
        nxt = bfs(self.head, goal, is_free) if goal else None
        if nxt is not None:
            d = step_dir(self.head, nxt)
            # Only if there is room after it.
            if flood(nxt, lambda c: is_free(c) and c != self.head) > len(b['body']) + 4:
                return d
        best, room = None, -1
        for k, d in DIRS.items():
            n = (self.head[0] + d[0], self.head[1] + d[1])
            if k != OPP[self.dir] and is_free(n):
                r = flood(n, is_free)
                if r > room:
                    best, room = k, r
        return best

    def turn(self, d):
        if d and d != self.dir and d != OPP[self.dir]:
            self.g.key(d, delay=0)

def intervals(times):
    gaps = [b - a for a, b in zip(times, times[1:])]
    return sum(gaps) / len(gaps) * 1000 if gaps else None

# ---------------------------------------------------------------------------
WORK = tempfile.mkdtemp(prefix='snake-pu-')
g = Game(BIN, cwd=WORK)
try:
    # Candidate flow (#53): Main menu -> name entry -> mode menu -> play.
    for _ in range(100):
        if 'Main menu' in g.title():
            break
        time.sleep(0.05)
    check('main menu ready', 'Main menu' in g.title(), g.title())
    g.key('Return'); time.sleep(0.3)
    subprocess.run(['xdotool', 'type', '--delay', '20', 'Tejas'], env=g.env)
    time.sleep(0.3)
    check('name entered', '| Name: Tejas |' in g.title(), g.title())
    g.key('Return'); time.sleep(0.3)
    t = parse_title(g.title())
    check('mode menu ready', t.get('state') == 'Menu', t['raw'])
    g.key('Return')
    time.sleep(0.2)
    t = parse_title(g.title())
    check('Classic / Medium game started', t.get('state') == 'Playing' and 'Classic / Medium' in t['raw'], t['raw'])

    pilot = Pilot(g)
    start = time.time()
    first_seen = {}          # power-up appearance -> time
    appearances = []         # (time, cell, type)
    collected = []           # dicts
    saved = set()
    current_item = None
    pending = None           # collection waiting for its follow-up checks
    paused_checked = False
    multiplier_food = []     # score deltas while x2 is shown in the title
    normal_food = []         # score deltas with no multiplier shown
    last_score = 0
    last_effect = None
    restarts = 0

    while time.time() - start < PLAY_SECONDS:
        b = pilot.wait_move()
        if b is None:
            break
        now = time.time()
        title = parse_title(g.title())
        if title.get('state') == 'GameOver':
            # The pilot crashed: note it and play again (Enter).
            log.append('game over at %.1f s: %s' % (now - start, title['raw']))
            restarts += 1
            g.key('Return')
            time.sleep(0.3)
            pilot = Pilot(g)
            current_item, pending, last_score, last_effect = None, None, 0, None
            continue
        if title.get('state') != 'Playing':
            log.append('state %s at %.1f s' % (title.get('state'), now - start))
            break

        # Score changes: food eaten with or without the x2 effect.
        score = title.get('score', last_score)
        if score != last_score:
            (multiplier_food if last_effect == 'Multiplier' or title.get('effect') == 'Multiplier'
             else normal_food).append(score - last_score)
            last_score = score
        last_effect = title.get('effect')

        items = b['item']
        item = items[0] if items else None
        blink = (item and appearances and appearances[-1][1] == item[0]
                 and now - start - appearances[-1][0] < 5.5)
        if item and (current_item is None or current_item[0] != item[0]) and not blink:
            appearances.append((now - start, item[0], item[1]))
            if item[1] not in saved:
                pilot.last[0].save(os.path.join(OUT, 'powerup-on-board-%s.png' % item[1].lower()))
                saved.add(item[1])
        if current_item and not item:
            # Gone: collected if the head is on its cell, otherwise expired.
            if pilot.head == current_item[0]:
                length_before = current_item[2]
                length_after = len(b['body']) + len(b['head'])
                collected.append({'type': current_item[1], 'time': now - start,
                                  'length_before': length_before, 'length_after': length_after,
                                  'title': title['raw']})
                pending = collected[-1]
                pending['move_index'] = len(pilot.moves)
            elif not (appearances and now - start - appearances[-1][0] < 4.0):
                log.append('%s at %s gone without pick-up at %.1f s' % (current_item[1], current_item[0], now - start))
        current_item = (item[0], item[1], len(b['body']) + len(b['head'])) if item else None

        # Follow-up of the last pick-up.
        if pending and pending['type'] == 'Speed':
            n = len(pilot.moves) - pending['move_index']
            if n == 1:
                pending['effect_title'] = title.get('effect'), title.get('effect_left')
                if 'speed.png' not in saved:
                    pilot.last[0].save(os.path.join(OUT, 'powerup-hud-speed.png'))
                    saved.add('speed.png')
            if n == 30:
                pending['boost_ms'] = intervals(pilot.moves[pending['move_index']:pending['move_index'] + 30])
            if 'boost_ms' in pending and title.get('effect') is None and 'end' not in pending:
                pending['end'] = now - start
                pending['end_index'] = len(pilot.moves)
            if 'end_index' in pending and len(pilot.moves) - pending['end_index'] == 12:
                pending['after_ms'] = intervals(pilot.moves[pending['end_index'] + 1:pending['end_index'] + 12])
                pending = None
        elif pending and pending['type'] == 'Multiplier':
            if 'effect_title' not in pending:
                pending['effect_title'] = title.get('effect'), title.get('effect_left')
                if 'mult.png' not in saved:
                    pilot.last[0].save(os.path.join(OUT, 'powerup-hud-multiplier.png'))
                    saved.add('mult.png')
                # Pause with the effect running: its time must not move.
                if not paused_checked:
                    before = parse_title(g.title()).get('effect_left')
                    g.key('Escape')
                    time.sleep(3)
                    paused = parse_title(g.title())
                    g.key('Escape')
                    paused_checked = True
                    pending['pause'] = (before, paused.get('state'), paused.get('effect_left'))
                    pilot.head = None
            elif title.get('effect') != 'Multiplier':
                pending = None
        elif pending:
            pending = None

        # Steer: power-up first, then food.
        goal = item[0] if item else (b['food'][0] if b['food'] else None)
        pilot.turn(pilot.choose(b, goal))

    final = parse_title(g.title())
    g.grab().save(os.path.join(OUT, 'powerup-final.png'))
finally:
    # Leave through the menu (M, then Esc) so the process exits normally.
    if g.alive():
        g.key('m'); time.sleep(0.5)
        if '| Menu |' in g.title():
            g.key('Escape'); time.sleep(0.5)
        g.key('Escape')
        for _ in range(50):
            if not g.alive():
                break
            time.sleep(0.1)
    exit_code = g.proc.poll()
    g.close()
    print('process exit code:', exit_code)
check('clean exit', exit_code == 0, 'exit code %s' % exit_code)
print('files written:', sorted(os.listdir(WORK)))

played = time.time() - start
print('played %.1f s, %d moves, state %s' % (played, len(pilot.moves), final.get('state')))
print('appearances:', [('%.1f' % t, c, k) for t, c, k in appearances])
for c in collected:
    print('collected:', {k: v for k, v in c.items() if k != 'title'})
print('food with x2:', multiplier_food, 'food without:', normal_food)
for line in log[:20]:
    print('log:', line)

types = {c['type'] for c in collected}
# One appearance at least every 15 s; a few can hide under the HUD in row 0,
# which this harness does not read, so allow two missed ones.
check('power-ups keep appearing during play', len(appearances) >= int(played // 15) - 2,
      '%d seen in %.0f s (%d restarts)' % (len(appearances), played, restarts))
gaps = [b[0] - a[0] for a, b in zip(appearances, appearances[1:])]
print('gaps between seen appearances (s):', ', '.join('%.1f' % x for x in gaps))
for c in collected:
    if c['type'] == 'Speed' and 'boost_ms' in c:
        check('Speed: title shows "Speed boost" with seconds left', c.get('effect_title', (None,))[0] == 'Speed',
              str(c.get('effect_title')))
        check('Speed: moves every ~72 ms during the boost', 62 <= c['boost_ms'] <= 82, '%.1f ms over 29 moves' % c['boost_ms'])
        if 'end' in c:
            check('Speed: boost ends after ~5 s', 4.6 <= c['end'] - c['time'] <= 5.6, '%.2f s' % (c['end'] - c['time']))
        if 'after_ms' in c:
            check('Speed: back to ~120 ms afterwards', 110 <= c['after_ms'] <= 130, '%.1f ms' % c['after_ms'])
        break
for c in collected:
    if c['type'] == 'Multiplier':
        check('Multiplier: title shows "Score x2" with seconds left', c.get('effect_title', (None,))[0] == 'Multiplier',
              str(c.get('effect_title')))
        if 'pause' in c:
            before, state, during = c['pause']
            check('Multiplier: pause freezes the effect time', state == 'Paused' and before == during,
                  'before %s, paused %s %s' % (before, state, during))
        break
if multiplier_food:
    check('food during x2 scores +20', all(d == 20 for d in multiplier_food), str(multiplier_food))
if normal_food:
    check('food without x2 scores +10', all(d == 10 for d in normal_food), str(normal_food))
for c in collected:
    if c['type'] == 'Shrink':
        expected = max(3, c['length_before'] - 1)
        check('Shrink: length %d -> %d' % (c['length_before'], expected), c['length_after'] == expected,
              'measured %d' % c['length_after'])
print('collected types:', sorted(types))
print('SUMMARY %d/%d passed' % (sum(ok for _, ok, _ in results), len(results)))

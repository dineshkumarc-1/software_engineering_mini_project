#!/usr/bin/env python3
"""Issue #18: TC-SEC-01, TC-SEC-06 (system part) and TC-SEC-02 on the real game.

Usage: python3 tools/security_ui_smoke.py /absolute/path/to/SnakeXtreme [:96]

Linux only (Xvfb, xdotool, /proc). Needs the name-entry screen (#19). Runs
the game in a fresh working directory, types into the name field with real
XTEST key events (10,000 characters, then each unsafe input one character at
a time), plays one game, exits, and inspects every file the game wrote and
its log for a user name, host name or absolute path. Prints one line per
check; the exit code is non-zero if any check fails.
"""
import getpass, os, re, shutil, socket, subprocess, sys, tempfile, time
from pathlib import Path

BIN = Path(sys.argv[1]).resolve()
DISPLAY = sys.argv[2] if len(sys.argv) > 2 else ":96"
env = dict(os.environ, DISPLAY=DISPLAY, ALSOFT_DRIVERS="null", LIBGL_ALWAYS_SOFTWARE="1")
results = []
ALLOWED = set('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 _-')

def check(name, ok, detail=""):
    results.append(bool(ok))
    print(("PASS " if ok else "FAIL ") + name + (" - " + detail if detail else ""), flush=True)

def xdo(*args):
    return subprocess.run(["xdotool", *map(str, args)], env=env, capture_output=True, text=True).stdout.strip()

def wait(pred, seconds=10):
    end = time.time() + seconds
    while time.time() < end:
        if pred():
            return True
        time.sleep(0.03)
    return False

xvfb = subprocess.Popen(["Xvfb", DISPLAY, "-screen", "0", "1024x768x24", "-nolisten", "tcp"],
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
time.sleep(0.8)
work = Path(tempfile.mkdtemp(prefix="snake-sec-"))
log_path = work / "runtime.log"
log = open(log_path, "w")
proc = subprocess.Popen([str(BIN)], cwd=work, env=env, stdout=log, stderr=log)
try:
    assert wait(lambda: xdo("search", "--name", "SnakeXtreme") != "", 15), "no window"
    win = xdo("search", "--name", "SnakeXtreme").split()[0]
    xdo("windowfocus", "--sync", win)
    title = lambda: xdo("getwindowname", win)
    key = lambda *k: xdo("key", "--window", win, "--clearmodifiers", "--delay", "30", *k)
    def typed(text, delay=8):
        xdo("type", "--window", win, "--delay", delay, text)
    def name():
        m = re.search(r"\| Name: (.*?) \|", title() + " |")
        return m.group(1) if m else None
    def clear():
        key(*(["BackSpace"] * 20))
        wait(lambda: name() == "", 3)

    check("main menu shown", wait(lambda: "Main menu" in title()), title())
    key("Return")
    check("New Game opens name entry", wait(lambda: "| Name:" in title()), title())

    # TC-SEC-06: 10,000 characters typed into the name field.
    def rss_kb():
        for line in open("/proc/%d/status" % proc.pid):
            if line.startswith("VmRSS:"):
                return int(line.split()[1])
        return 0
    typed("A" * 200, delay=1)  # warm up text rendering first
    wait(lambda: name() == "A" * 15, 10)
    rss_before = rss_kb()
    start = time.time()
    typed("A" * 10000, delay=1)
    ok = wait(lambda: name() == "A" * 15, 20)
    time.sleep(0.5)
    rss_after = rss_kb()
    check("TC-SEC-06: memory stays stable while 10,000 characters are typed",
          rss_after - rss_before < 2048, "RSS %d kB -> %d kB" % (rss_before, rss_after))
    check("TC-SEC-06: 10,000 typed characters leave exactly 15 in the field",
          ok and proc.poll() is None, "name=%r, %.1f s, title: %s" % (name(), time.time() - start, title()[:120]))
    check("TC-SEC-06: the 16th and later characters are refused with a message",
          "Maximum 15 characters" in title(), title()[:160])

    # TC-SEC-01: each unsafe input; the field keeps only allowed characters.
    for text, allowed in (("Ab,cd", "Abcd"), ('a"b', "ab"), ("../x", "x"), ('{"x":1}', "x1"),
                          ("C:\\x", "Cx"), ("a;b|c", "abc"), ("Espa\u00f1a", "Espaa"),
                          ("emoji\U0001F600", "emoji")):
        clear()
        # One character at a time: every refused character must show the
        # inline message at once (the next accepted character clears it).
        messages = []
        for ch in text:
            before = name() or ""
            if ord(ch) > 127:
                # A Unicode keysym; xdotool's "type" cannot send these on Xvfb.
                key("U%04X" % ord(ch))
            else:
                typed(ch)
            if ch in ALLOWED:
                wait(lambda: name() == before + ch, 3)
            else:
                shown = wait(lambda: "Use letters, numbers, spaces, underscore or hyphen" in title(), 3)
                messages.append(shown and name() == before)
        check("TC-SEC-01: %r: each refused character shows the message; field keeps %r" % (text, allowed),
              name() == allowed and messages and all(messages),
              "name=%r, refused %d/%d with message" % (name(), sum(messages), len(messages)))
    clear()
    key("Tab")
    time.sleep(0.3)
    check("TC-SEC-01: Tab inserts nothing", name() == "", "name=%r" % name())
    key("Return")
    time.sleep(0.3)
    check("TC-SEC-01: an empty name cannot be confirmed", "| Name:" in title(), title()[:120])

    clear()
    typed("A_b-1 x")
    wait(lambda: name() == "A_b-1 x", 4)
    key("Return")
    check("TC-SEC-01: the valid name A_b-1 x is accepted", wait(lambda: "| Menu |" in title(), 5), title()[:120])
    key("Return")
    check("game starts", wait(lambda: "Playing" in title(), 5), title()[:120])
    # Play until the snake hits the wall (Classic / Medium, straight right).
    check("game over reached", wait(lambda: "Game over" in title(), 15), title()[:120])
    key("m")
    wait(lambda: "Main menu" in title() or "| Menu |" in title(), 5)
    if "| Menu |" in title():
        key("Escape")
        wait(lambda: "Main menu" in title(), 5)
    key("Escape")
    check("clean exit", wait(lambda: proc.poll() is not None, 10), "exit code %s" % proc.poll())
finally:
    if proc.poll() is None:
        proc.terminate()
        proc.wait(5)
    log.close()
    xvfb.terminate()
    xvfb.wait(5)

# TC-SEC-02: only Appendix B fields; no user name, host name or absolute path.
files = sorted(p for p in work.rglob("*") if p.is_file())
print("files written:", [str(p.relative_to(work)) for p in files])
user, host = getpass.getuser(), socket.gethostname()
needles = [user, host, str(work), str(Path.home()), "/home/", "/root", "/tmp/", "C:\\Users"]
for path in files:
    text = path.read_text(errors="replace")
    hits = [n for n in needles if n and n in text]
    check("TC-SEC-02: %s has no user name, host name or absolute path" % path.name, not hits,
          "found %s" % hits if hits else "%d bytes" % len(text))
config = work / "config.json"
if config.exists():
    import json
    keys = set(json.loads(config.read_text()))
    expected = {"version", "key_up", "key_down", "key_left", "key_right", "music_volume", "sfx_volume", "theme"}
    check("TC-SEC-02: config.json holds only the Appendix B fields", keys == expected, str(sorted(keys)))
print("runtime log:", log_path.read_text().strip().replace("\n", " | ")[:300])
print("SUMMARY %d/%d passed" % (sum(results), len(results)))
shutil.rmtree(work, ignore_errors=True)
sys.exit(0 if results and all(results) else 1)

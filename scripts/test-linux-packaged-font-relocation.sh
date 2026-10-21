#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
executable="$repo_root/build/SnakeXtreme"
build_assets="$repo_root/build/assets"
source_font="$repo_root/assets/fonts/dejavu-sans-ttf-2.37/ttf/DejaVuSans.ttf"
disabled_font="${source_font}.relocation-test-disabled"
output_dir="$repo_root/build/relocation-test-output"
relocated_dir="$(mktemp -d /tmp/snakextreme-relocated.XXXXXX)"
work_dir="$(mktemp -d /tmp/snakextreme-cwd.XXXXXX)"
trace_log="$output_dir/font-open-trace.log"
screenshot="$output_dir/gameplay.png"
hud_crop="$output_dir/hud-crop.png"
trace_pid=""

mkdir -p "$output_dir" "$relocated_dir/assets" "$work_dir"

cleanup() {
    if [[ -n "$trace_pid" ]]; then
        # Kill the traced app and strace as well as timeout to avoid orphaned
        # GUI processes or an unbounded wait in a headless CI runner.
        pkill -TERM -f "$relocated_dir/SnakeXtreme" 2>/dev/null || true
        kill -TERM "$trace_pid" 2>/dev/null || true
        sleep 0.2
        kill -KILL "$trace_pid" 2>/dev/null || true
        wait "$trace_pid" 2>/dev/null || true
    fi
    if [[ -f "$disabled_font" && ! -f "$source_font" ]]; then
        mv "$disabled_font" "$source_font"
    fi
    rm -rf "$relocated_dir" "$work_dir"
}
trap cleanup EXIT

for tool in strace xdotool import tesseract timeout; do
    command -v "$tool" >/dev/null 2>&1 || {
        echo "Required test tool missing: $tool" >&2
        exit 1
    }
done
if command -v magick >/dev/null 2>&1; then
    image_tool=(magick)
elif command -v convert >/dev/null 2>&1; then
    image_tool=(convert)
else
    echo "Required ImageMagick command missing: magick/convert" >&2
    exit 1
fi

[[ -x "$executable" ]] || { echo "Built app missing: $executable" >&2; exit 1; }
[[ -f "$build_assets/fonts/DejaVuSans.ttf" ]] || {
    echo "Bundled build font missing: $build_assets/fonts/DejaVuSans.ttf" >&2
    exit 1
}
[[ -f "$source_font" ]] || { echo "Source-tree font missing before test: $source_font" >&2; exit 1; }
[[ ! -e "$disabled_font" ]] || { echo "Stale disabled font exists: $disabled_font" >&2; exit 1; }

cp "$executable" "$relocated_dir/SnakeXtreme"
cp -a "$build_assets/." "$relocated_dir/assets/"
expected_font="$relocated_dir/assets/fonts/DejaVuSans.ttf"
[[ -f "$expected_font" ]] || {
    echo "Relocated package does not contain the expected font: $expected_font" >&2
    exit 1
}

# The executable was compiled with SNAKE_ASSET_DIR pointing into the checkout.
# Temporarily hide that exact source font to prove the packaged copy is sufficient.
mv "$source_font" "$disabled_font"
[[ ! -e "$source_font" ]] || { echo "Failed to disable source-tree font" >&2; exit 1; }

cd "$work_dir"
echo "Launching relocated app from unrelated working directory: $PWD"
echo "Expected bundled font: $expected_font"

# Give the process a bound so a broken app cannot hang CI indefinitely.
LIBGL_ALWAYS_SOFTWARE=1 timeout 20s strace -f -e trace=openat \
    -o "$trace_log" "$relocated_dir/SnakeXtreme" &
trace_pid=$!

window=""
for attempt in $(seq 1 30); do
    window="$(timeout 1s xdotool search --onlyvisible --name 'SnakeXtreme' 2>/dev/null | head -n 1 || true)"
    if [[ -n "$window" ]]; then
        break
    fi
    if ! kill -0 "$trace_pid" 2>/dev/null; then
        wait "$trace_pid" || true
        echo "Application exited before creating a visible window" >&2
        exit 1
    fi
    sleep 0.25
done
[[ -n "$window" ]] || { echo "Timed out waiting for SnakeXtreme window" >&2; exit 1; }

# Xvfb has no desktop window manager; avoid waiting indefinitely for an
# active-window notification that a minimal display may never provide.
timeout 5s xdotool windowfocus "$window" || true
echo "[smoke] Sending Enter to open gameplay"
timeout 5s xdotool key --clearmodifiers Return
sleep 0.8

window_title="$(timeout 5s xdotool getwindowname "$window")"
echo "Window title after Enter: $window_title"
[[ "$window_title" == *"Gameplay placeholder"* ]] || {
    echo "App did not enter gameplay after Return" >&2
    exit 1
}

echo "[smoke] Capturing gameplay window"
timeout 10s import -window "$window" "$screenshot"
# Crop away the grid: full-screen OCR treats its repeated lines as text and
# can miss the HUD even when the screenshot visibly contains it.
"${image_tool[@]}" "$screenshot" -crop 390x42+10+8 +repage -resize 200% \
    -colorspace Gray -contrast-stretch 1%x1% "$hud_crop"
echo "[smoke] Checking cropped HUD text"
ocr_text="$(timeout 20s tesseract "$hud_crop" stdout --psm 7 2>/dev/null | tr '\n' ' ')"
echo "OCR of cropped HUD: $ocr_text"
grep -Eiq 'Score[ :]*0' <<< "$ocr_text" || {
    echo "Screenshot did not show the current score" >&2
    exit 1
}
grep -Eiq 'High[[:space:]]+Score[ :]*0' <<< "$ocr_text" || {
    echo "Screenshot did not show the high score" >&2
    exit 1
}

# Require a successful open (file descriptor returned), not merely an attempted path.
if ! grep -F "$expected_font" "$trace_log" | grep -Eq ' = [0-9]+$'; then
    echo "Trace did not show a successful open of the relocated bundled font" >&2
    exit 1
fi
if grep -F "$source_font" "$trace_log" >/dev/null; then
    echo "Trace shows an attempted source-tree font access despite the packaged copy" >&2
    exit 1
fi

echo "[smoke] Font trace confirms the relocated bundled font was successfully opened"
echo "PASS: relocated executable opened bundled font, entered gameplay, and rendered Score/High Score."
xdotool windowclose "$window" 2>/dev/null || true

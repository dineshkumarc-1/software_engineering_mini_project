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
trace_pid=""

mkdir -p "$output_dir" "$relocated_dir/assets" "$work_dir"

cleanup() {
    if [[ -n "$trace_pid" ]] && kill -0 "$trace_pid" 2>/dev/null; then
        # timeout forwards SIGTERM to the traced app as well.
        kill -TERM "$trace_pid" 2>/dev/null || true
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
[[ -x "$executable" ]] || { echo "Built app missing: $executable" >&2; exit 1; }
[[ -f "$build_assets/fonts/DejaVuSans.ttf" ]] || {
    echo "Bundled build font missing: $build_assets/fonts/DejaVuSans.ttf" >&2
    exit 1
}
[[ -f "$source_font" ]] || { echo "Source-tree font missing before test: $source_font" >&2; exit 1; }
[[ ! -e "$disabled_font" ]] || { echo "Stale disabled font exists: $disabled_font" >&2; exit 1; }

cp "$executable" "$relocated_dir/SnakeXtreme"
cp -a "$build_assets/." "$relocated_dir/assets/"
[[ -f "$relocated_dir/assets/fonts/DejaVuSans.ttf" ]] || {
    echo "Relocated package does not contain the expected font" >&2
    exit 1
}

# The executable was compiled with SNAKE_ASSET_DIR pointing into the checkout.
# Temporarily hide that exact source font to prove the packaged copy is sufficient.
mv "$source_font" "$disabled_font"
[[ ! -e "$source_font" ]] || { echo "Failed to disable source-tree font" >&2; exit 1; }

cd "$work_dir"
echo "Launching relocated app from unrelated working directory: $PWD"
echo "Expected bundled font: $relocated_dir/assets/fonts/DejaVuSans.ttf"

# Give the process a bound so a broken app cannot hang CI indefinitely.
LIBGL_ALWAYS_SOFTWARE=1 timeout --foreground 20s strace -f -e trace=openat \
    -o "$trace_log" "$relocated_dir/SnakeXtreme" &
trace_pid=$!

window=""
for attempt in $(seq 1 60); do
    window="$(xdotool search --onlyvisible --name 'SnakeXtreme' 2>/dev/null | head -n 1 || true)"
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
timeout 5s xdotool key --clearmodifiers Return
sleep 0.8

window_title="$(timeout 5s xdotool getwindowname "$window")"
echo "Window title after Enter: $window_title"
[[ "$window_title" == *"Gameplay placeholder"* ]] || {
    echo "App did not enter gameplay after Return" >&2
    exit 1
}

timeout 10s import -window "$window" "$screenshot"
ocr_text="$(timeout 20s tesseract "$screenshot" stdout --psm 6 2>/dev/null | tr '\n' ' ')"
echo "OCR of gameplay screenshot: $ocr_text"
grep -Eiq 'Score[ :]*0' <<< "$ocr_text" || {
    echo "Screenshot did not show the current score" >&2
    exit 1
}
grep -Eiq 'High[[:space:]]+Score[ :]*0' <<< "$ocr_text" || {
    echo "Screenshot did not show the high score" >&2
    exit 1
}

grep -F "$relocated_dir/assets/fonts/DejaVuSans.ttf" "$trace_log" >/dev/null || {
    echo "Trace did not show the relocated bundled font being opened" >&2
    exit 1
}
if grep -F "$source_font" "$trace_log" >/dev/null; then
    echo "Trace shows an attempted source-tree font access despite the packaged copy" >&2
    exit 1
fi

echo "PASS: relocated executable opened bundled font, entered gameplay, and rendered Score/High Score."

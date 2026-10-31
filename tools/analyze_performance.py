#!/usr/bin/env python3
"""Summarize diagnostic JSON; never awards target-hardware acceptance."""
import argparse
import json
from pathlib import Path
import statistics

MAX_FILE_BYTES = 8 * 1024 * 1024

def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("duplicate JSON key")
        result[key] = value
    return result

def integer(value, low=0, high=86400 * 1000000):
    if type(value) is not int or not low <= value <= high:
        raise ValueError("out-of-range/non-integer measurement")
    return value

def distribution(values):
    if not values:
        return None
    return {"count": len(values), "min": min(values), "median": statistics.median(values),
            "mean": statistics.mean(values), "max": max(values)}

def startup_timeline(report, first_frame):
    """Optional named start-up phases (#22); absent in older reports."""
    phases = report.get("startup_phases_us")
    if phases is None:
        return None, None
    if not isinstance(phases, list) or len(phases) > 16:
        raise ValueError("invalid start-up phase list")
    integer(report.get("dropped_phases", 0))
    named, previous = {}, None
    for item in phases:
        if not isinstance(item, dict) or set(item) != {"phase", "us"}:
            raise ValueError("start-up phase must be an object with phase and us")
        name, at = item["phase"], integer(item["us"])
        if type(name) is not str or not 1 <= len(name) <= 64 or name in named or name == "first_frame":
            raise ValueError("invalid or duplicate start-up phase name")
        if previous is not None and at < previous:
            raise ValueError("start-up phases out of order")
        named[name], previous = at, at
    timeline = sorted(list(named.items()) + ([("first_frame", first_frame)] if first_frame is not None else []),
                      key=lambda pair: pair[1])
    steps, before = {}, 0
    for name, at in timeline:
        steps[name] = at - before
        before = at
    return named, steps

def summarize(path):
    path = Path(path)
    if path.stat().st_size > MAX_FILE_BYTES:
        raise ValueError("report exceeds 8 MiB")
    report = json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=unique_object)
    if not isinstance(report, dict):
        raise ValueError("report root must be an object")
    if report.get("schema_version") != 1 or type(report.get("schema_version")) is not int:
        raise ValueError("unsupported report schema")
    if report.get("clock") != "steady_clock_us_since_main" or report.get("input_scope") != "SFML_dequeue_to_engine_queue_registration_lower_bound":
        raise ValueError("unknown timestamp scope")
    windows, inputs = report.get("fps_windows"), report.get("inputs")
    if not isinstance(windows, list) or len(windows) > 3600 or not isinstance(inputs, list) or len(inputs) > 4096:
        raise ValueError("invalid/bounds-exceeding sample lists")
    for field in ("dropped_windows", "dropped_inputs", "invalid_samples"):
        integer(report[field])
    first_frame = report.get("first_frame_us")
    if first_frame is not None:
        integer(first_frame)
    if windows and first_frame is None:
        raise ValueError("FPS windows require a first-frame timestamp")
    phases, phase_steps = startup_timeline(report, first_frame)
    eligible, previous, longest, chain = [], None, 0, 0
    all_fps = []
    for window in windows:
        if not isinstance(window, dict):
            raise ValueError("FPS sample must be an object")
        start = integer(window["start_us"])
        if window.get("duration_us") != 1000000 or type(window.get("duration_us")) is not int:
            raise ValueError("FPS window is not exactly one second")
        fps = integer(window["frames"], high=1000000)
        session = integer(window["session"])
        if type(window.get("uniform_context")) is not bool:
            raise ValueError("invalid context-uniformity flag")
        if previous is not None and start < previous[0] + 1000000:
            raise ValueError("overlapping or out-of-order FPS windows")
        if start < first_frame or (start - first_frame) % 1000000 != 0:
            raise ValueError("FPS window is off the first-frame one-second grid")
        all_fps.append(fps)
        valid = window["uniform_context"] and window.get("screen") == "Gameplay" and window.get("mode") == "Obstacle" and window.get("difficulty") == "Hard"
        if valid:
            eligible.append(fps)
            chain = chain + 1 if previous and previous == (start - 1000000, session, True) else 1
        else:
            chain = 0
        longest = max(longest, chain)
        previous = (start, session, valid)
    latencies, displayed, rejected = [], [], 0
    last_registered = -1
    for sample in inputs:
        if not isinstance(sample, dict):
            raise ValueError("input sample must be an object")
        if type(sample.get("accepted")) is not bool or sample.get("direction") not in ("Up", "Down", "Left", "Right"):
            raise ValueError("invalid input sample")
        dequeued, registered = integer(sample["dequeued_us"]), integer(sample["registered_us"])
        if registered < dequeued or dequeued < last_registered:
            raise ValueError("non-monotonic input timestamps")
        last_registered = registered
        applied, visible = sample.get("applied_us"), sample.get("displayed_us")
        if applied is not None and integer(applied) < registered:
            raise ValueError("input applied before registration")
        if visible is not None and (applied is None or integer(visible) < applied):
            raise ValueError("input displayed before application")
        if sample["accepted"]:
            latencies.append(registered - dequeued)
            if visible is not None:
                displayed.append(visible - dequeued)
        else:
            rejected += 1
            if applied is not None or visible is not None:
                raise ValueError("rejected input cannot have application/display timestamps")
    warnings = ["Diagnostic tooling only; target-hardware acceptance is not assessed.",
                "Registration latency excludes OS/SFML queue wait; it is a lower bound, not end-to-end input latency.",
                "First-frame time starts at main entry; use collect_launch.py for an external process-launch readiness upper bound.",
                "Display timestamps mean display() returned, not measured photons or GPU scan-out.",
                "No measured power-up activity or independent volunteer usability evidence is provided."]
    if phases is not None:
        warnings.append("Start-up phases are steady-clock marks after each named step; each step's time also includes "
                        "anything else that ran since the previous mark.")
    if any(report[f] for f in ("dropped_windows", "dropped_inputs", "invalid_samples")) or report.get("dropped_phases"):
        warnings.append("Dropped or invalid samples: this run is incomplete and unsuitable for acceptance.")
    return {"report": path.name, "acceptance": "not_assessed", "main_to_first_frame_us": first_frame,
            "startup_phases_us": phases, "startup_step_us": phase_steps,
            "all_complete_window_fps": distribution(all_fps), "hard_obstacle_uniform_gameplay_fps": distribution(eligible),
            "longest_same_session_eligible_seconds": longest,
            "fps_below_30_windows": sum(fps < 30 for fps in eligible),
            "accepted_input_registration_lower_bound_us": distribution(latencies),
            "accepted_input_registration_at_least_100ms": sum(value >= 100000 for value in latencies),
            "first_display_after_application_us": distribution(displayed), "rejected_inputs": rejected,
            "accepted_without_display": len(latencies) - len(displayed),
            "required_campaign": "3 x 300 consecutive seconds, Hard/Obstacle with power-ups; 100 randomized inputs on target Windows hardware",
            "warnings": warnings}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reports", nargs="+", type=Path)
    args = parser.parse_args()
    try:
        result = {"acceptance": "not_assessed", "runs": [summarize(path) for path in args.reports]}
    except (OSError, ValueError, KeyError, TypeError, RecursionError) as error:
        parser.exit(2, f"Invalid/incomplete probe report: {error}\n")
    print(json.dumps(result, indent=2))

if __name__ == "__main__":
    main()

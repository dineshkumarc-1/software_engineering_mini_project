"""Cross-platform collector regressions; real report validation, simulated child."""
import contextlib
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import collect_launch


class CollectLaunchTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.executable = self.root / "game"
        self.executable.touch()
        self.output = self.root / "output"
        self.report = {"schema_version": 1, "clock": "steady_clock_us_since_main",
                       "input_scope": "SFML_dequeue_to_engine_queue_registration_lower_bound",
                       "first_frame_us": 12345, "dropped_windows": 0, "dropped_inputs": 0,
                       "invalid_samples": 0, "fps_windows": [], "inputs": []}

    def run_cli(self, report="valid", marker=True, code=0, timeout=False):
        owner = self
        class Process:
            def __init__(self, args, **kwargs):
                self.stdout = io.StringIO(collect_launch.MARKER + "\n" if marker else "")
                self.waits = 0
                self.killed = False
                if report != "missing":
                    content = json.dumps(owner.report) if report == "valid" else report
                    Path(args[2]).write_text(content, encoding="utf-8")
            def wait(self, timeout=None):
                self.waits += 1
                if timeout is not None and self.waits == 1 and owner_timeout:
                    raise subprocess.TimeoutExpired("game", timeout)
                return -9 if self.killed else code
            def kill(self):
                self.killed = True
        owner_timeout = timeout
        args = ["collect_launch.py", str(self.executable), str(self.output),
                "--runs", "1", "--source-snapshot", "regression-fixture"]
        with patch.object(collect_launch.subprocess, "Popen", Process), patch.object(sys, "argv", args), contextlib.redirect_stdout(io.StringIO()):
            status = collect_launch.main()
        result = json.loads((self.output / "launch-summary.json").read_text(encoding="utf-8"))
        self.assertEqual(result["acceptance"], "not_assessed")
        return status, result["runs"][0]

    def test_marker_without_report_fails_cli_and_records_reason(self):
        status, run = self.run_cli(report="missing")
        self.assertEqual(status, 1)
        self.assertFalse(run["collection_ok"])
        self.assertFalse(run["report_valid"])
        self.assertEqual(run["report_error"], "missing_or_non_regular_report")

    def test_malformed_json_fails(self):
        status, run = self.run_cli(report="{broken")
        self.assertEqual(status, 1)
        self.assertEqual(run["report_error"], "invalid_or_unreadable_report")

    def test_invalid_schema_fails(self):
        self.report["schema_version"] = 2
        self.assertEqual(self.run_cli()[0], 1)

    def test_missing_first_frame_fails(self):
        self.report["first_frame_us"] = None
        status, run = self.run_cli()
        self.assertEqual(status, 1)
        self.assertEqual(run["report_error"], "missing_first_frame")

    def test_overlapping_windows_fail_shared_validation(self):
        self.report["fps_windows"] = [
            {"start_us": start, "duration_us": 1000000, "frames": 60, "session": 1,
             "uniform_context": True, "screen": "Gameplay", "mode": "Obstacle", "difficulty": "Hard"}
            for start in (12345, 12346)]
        self.assertEqual(self.run_cli()[0], 1)

    def test_valid_report_marker_and_exit_succeed_without_acceptance(self):
        status, run = self.run_cli()
        self.assertEqual(status, 0)
        self.assertTrue(run["collection_ok"])
        self.assertTrue(run["report_valid"])
        self.assertIsNone(run["report_error"])

    def test_valid_report_without_marker_fails(self):
        status, run = self.run_cli(marker=False)
        self.assertEqual(status, 1)
        self.assertTrue(run["report_valid"])
        self.assertFalse(run["collection_ok"])

    def test_nonzero_exit_fails_even_with_valid_report(self):
        self.assertEqual(self.run_cli(code=3)[0], 1)

    def test_timeout_fails_even_with_valid_report(self):
        status, run = self.run_cli(timeout=True)
        self.assertEqual(status, 1)
        self.assertTrue(run["timed_out"])
        self.assertFalse(run["collection_ok"])

    def test_existing_output_is_not_overwritten(self):
        self.output.mkdir()
        sentinel = self.output / "keep.txt"
        sentinel.write_text("keep", encoding="utf-8")
        with self.assertRaises(FileExistsError):
            collect_launch.collect(self.executable, self.output, 1, "fixture")
        self.assertEqual(sentinel.read_text(encoding="utf-8"), "keep")


if __name__ == "__main__":
    unittest.main()

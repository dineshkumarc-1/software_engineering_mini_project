import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from analyze_performance import summarize  # noqa: E402

class PerformanceToolsTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.path = Path(self.directory.name) / "report.json"
        self.report = {"schema_version": 1, "clock": "steady_clock_us_since_main",
                       "input_scope": "SFML_dequeue_to_engine_queue_registration_lower_bound",
                       "first_frame_us": 0, "dropped_windows": 0, "dropped_inputs": 0,
                       "invalid_samples": 0, "fps_windows": [], "inputs": []}
    def run_report(self, report=None):
        self.path.write_text(json.dumps(report if report is not None else self.report), encoding="utf-8")
        return summarize(self.path)
    def window(self, start, fps=60, session=1, screen="Gameplay", uniform=True):
        return {"start_us": start, "duration_us": 1000000, "frames": fps,
                "session": session, "screen": screen, "mode": "Obstacle", "difficulty": "Hard", "uniform_context": uniform}
    def test_zero_fps_is_not_dropped_and_partial_windows_are_not_counted(self):
        self.report["fps_windows"] = [self.window(0), self.window(1000000, 0)]
        self.report["partial_window"] = {"frames": 999}
        result = self.run_report()
        self.assertEqual(result["all_complete_window_fps"]["count"], 2)
        self.assertEqual(result["all_complete_window_fps"]["min"], 0)
        self.assertEqual(result["fps_below_30_windows"], 1)
    def test_mixed_context_and_session_gaps_cannot_form_a_300_second_run(self):
        self.report["fps_windows"] = [self.window(0), self.window(1000000, session=2),
                                      self.window(2000000, screen="MainMenu"), self.window(3000000, uniform=False)]
        self.assertEqual(self.run_report()["longest_same_session_eligible_seconds"], 1)
    def test_latency_boundary_and_rejections_are_separate(self):
        self.report["inputs"] = [
            {"accepted": True, "direction": "Up", "dequeued_us": 0, "registered_us": 100000, "applied_us": 120000, "displayed_us": 130000},
            {"accepted": True, "direction": "Left", "dequeued_us": 200000, "registered_us": 300001, "applied_us": None, "displayed_us": None},
            {"accepted": False, "direction": "Right", "dequeued_us": 400000, "registered_us": 400001, "applied_us": None, "displayed_us": None}]
        result = self.run_report()
        self.assertEqual(result["accepted_input_registration_at_least_100ms"], 2)
        self.assertEqual(result["accepted_without_display"], 1)
        self.assertEqual(result["rejected_inputs"], 1)
        self.assertEqual(result["acceptance"], "not_assessed")
    def test_invalid_timestamps_cannot_become_small_latencies(self):
        self.report["inputs"] = [{"accepted": True, "direction": "Up", "dequeued_us": 100, "registered_us": 99}]
        with self.assertRaises(ValueError): self.run_report()
    def test_rejected_input_with_display_timestamp_is_invalid(self):
        self.report["inputs"] = [{"accepted": False, "direction": "Up", "dequeued_us": 0, "registered_us": 1,
                                   "applied_us": 2, "displayed_us": 3}]
        with self.assertRaises(ValueError): self.run_report()
    def test_wrong_types_and_non_one_second_windows_are_invalid(self):
        for field, value in (("schema_version", True), ("invalid_samples", -1), ("dropped_inputs", True)):
            report = copy.deepcopy(self.report); report[field] = value
            with self.assertRaises(ValueError): self.run_report(report)
        self.report["fps_windows"] = [self.window(0)]; self.report["fps_windows"][0]["duration_us"] = 2000000
        with self.assertRaises(ValueError): self.run_report()
    def test_duplicate_json_keys_are_invalid(self):
        self.path.write_text('{"schema_version":1,"schema_version":1}', encoding="utf-8")
        with self.assertRaises(ValueError): summarize(self.path)
    def test_drops_are_flagged_and_no_report_grants_acceptance(self):
        self.report["dropped_inputs"] = 1
        result = self.run_report()
        self.assertEqual(result["acceptance"], "not_assessed")
        self.assertTrue(any("incomplete" in warning for warning in result["warnings"]))
    def test_malformed_root_and_sample_objects_are_rejected(self):
        self.path.write_text("[]", encoding="utf-8")
        with self.assertRaises(ValueError): summarize(self.path)
        for key in ("fps_windows", "inputs"):
            report = copy.deepcopy(self.report); report[key] = [42]
            with self.assertRaises(ValueError): self.run_report(report)
    def test_300_good_windows_still_require_target_hardware_and_powerups(self):
        self.report["fps_windows"] = [self.window(i * 1000000) for i in range(300)]
        result = self.run_report()
        self.assertEqual(result["longest_same_session_eligible_seconds"], 300)
        self.assertEqual(result["acceptance"], "not_assessed")
        self.assertTrue(any("power-up" in warning for warning in result["warnings"]))

if __name__ == "__main__":
    unittest.main()

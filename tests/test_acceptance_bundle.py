import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from prepare_acceptance import prepare, hash_file

SHA = "0123456789abcdef" * 2 + "01234567"
URL = "https://github.com/dineshkumarc-1/software_engineering_mini_project/actions/runs/40465075508"

class AcceptanceBundleTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.exe = self.root / "SnakeXtreme.exe"
        self.exe.write_bytes(b"test fixture, not an executable")
        self.output = self.root / "acceptance"
    def test_hashes_identify_bytes_without_claiming_tested_source(self):
        result = prepare(self.exe, self.output, SHA.upper(), URL)
        self.assertEqual(result["package_files"][0]["sha256"], hashlib.sha256(self.exe.read_bytes()).hexdigest())
        self.assertEqual(result["source_commit_operator_supplied"], SHA)
        self.assertFalse(result["source_artifact_relationship_independently_verified"])
        self.assertEqual(result["acceptance"], "not_assessed")
    def test_template_does_not_award_passes(self):
        prepare(self.exe, self.output, SHA, URL)
        text = (self.output / "manual-results.md").read_text(encoding="utf-8")
        self.assertIn("UNEXECUTED TEMPLATE", text)
        self.assertIn("F5 preview", text)
        self.assertGreater(text.count("Not executed | Pending"), 10)
        self.assertNotIn("| Passed |", text)
    def test_private_absolute_path_is_not_saved(self):
        prepare(self.exe, self.output, SHA, URL)
        data = (self.output / "build-manifest.json").read_text()
        self.assertNotIn(str(self.root), data)
        self.assertNotIn("hostname", json.loads(data)["collection_platform"])
    def test_existing_directory_is_not_overwritten(self):
        self.output.mkdir(); marker = self.output / "keep.txt"; marker.write_text("keep")
        with self.assertRaises(FileExistsError): prepare(self.exe, self.output, SHA, URL)
        self.assertEqual(marker.read_text(), "keep")
        self.assertFalse((self.output / "build-manifest.json").exists())
    def test_invalid_commit_never_creates_output(self):
        for value in ("main", "1234567", "x" * 40, "a" * 41):
            with self.assertRaises(ValueError): prepare(self.exe, self.output, value, URL)
        self.assertFalse(self.output.exists())
    def test_foreign_or_untrusted_artifact_url_is_rejected(self):
        for value in (URL.replace("https:", "http:"), URL.replace("github.com", "github.com.evil.test"), URL + "?token=secret", URL.replace("dineshkumarc-1", "other")):
            with self.assertRaises(ValueError): prepare(self.exe, self.output, SHA, value)
        self.assertFalse(self.output.exists())
    def test_packaged_dependency_hashes_are_included(self):
        (self.root / "openal32.dll").write_bytes(b"dll fixture")
        result = prepare(self.exe, self.output, SHA, URL + "/artifacts/123")
        self.assertEqual([f["file"] for f in result["package_files"]], ["SnakeXtreme.exe", "openal32.dll"])
    def test_size_cap_and_control_characters_are_rejected(self):
        with mock.patch("prepare_acceptance.MAX_BINARY_BYTES", 2):
            with self.assertRaises(ValueError): hash_file(self.exe)
        with self.assertRaises(ValueError): prepare(self.exe, self.output, SHA, "\n" + URL)
        self.assertFalse(self.output.exists())
    def test_missing_or_empty_file_is_rejected(self):
        self.exe.unlink()
        with self.assertRaises(ValueError): hash_file(self.exe)
        self.exe.touch()
        with self.assertRaises(ValueError): hash_file(self.exe)

if __name__ == "__main__":
    unittest.main()

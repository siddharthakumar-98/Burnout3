"""Scope regressions: vendor matching must never inflate game-source completion."""

import contextlib
import io
import json
import tempfile
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path
from unittest.mock import patch

import progress
import progress_map


def report():
    game = {"total_code": "100", "matched_code": "25", "total_functions": 4, "matched_functions": 1}
    vendor = {"total_code": "900", "matched_code": "900", "total_functions": 96, "matched_functions": 96}
    return {
        "measures": {"total_code": "1000", "matched_code": "925", "total_functions": 100,
                     "matched_functions": 97},
        "categories": [{"id": "game", "measures": game}, {"id": "rw", "measures": vendor}],
        "units": [
            {"name": "d4/example", "metadata": {"progress_categories": ["game"]}, "measures": game,
             "functions": [{"name": "gameExample"}, {"name": "func_00100010"},
                           {"name": "func_00100020"}, {"name": "func_00100030"}]},
            {"name": "rw/vendorExample", "metadata": {"progress_categories": ["rw"]}, "measures": vendor,
             "functions": [{"name": "vendorExample"}]},
        ],
    }


class ProgressScopeTests(unittest.TestCase):
    def test_vendor_matching_does_not_change_map_or_badge(self):
        original = report()
        changed = report()
        changed["measures"]["matched_code"] = "25"
        changed["measures"]["matched_functions"] = 1
        changed["categories"][1]["measures"]["matched_code"] = "0"
        changed["categories"][1]["measures"]["matched_functions"] = 0
        with patch.object(progress_map, "unit_order", return_value={}):
            svg = progress_map.render(original)
            self.assertEqual(svg, progress_map.render(changed))
        self.assertEqual(progress_map.badge(original), progress_map.badge(changed))
        self.assertEqual(progress_map.badge(original)["message"], "25.00% of game code")
        ET.fromstring(svg)
        self.assertIn("1 of 4 game functions", svg)
        self.assertIn("1 game functions named", svg)
        self.assertNotIn("vendorExample", svg)
        self.assertNotIn("whole executable", svg)

    def test_generated_table_uses_game_denominator(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "build").mkdir()
            (root / "build/report.json").write_text(json.dumps(report()))
            with patch.object(progress, "ROOT", root), patch.object(progress_map, "unit_order", return_value={}), \
                    patch("sys.argv", ["progress.py", "--no-report"]), contextlib.redirect_stdout(io.StringIO()):
                progress.main()
            md = (root / "PROGRESS.md").read_text()
            self.assertIn("| **Burnout 3 game code** | 1 / 4 | 25.00% | 25 / 100 | 25.00% | 1 |", md)
            self.assertNotIn("| **All**", md)
            self.assertIn("96 functions and 900 code bytes", md)
            self.assertEqual(json.loads((root / "progress.json").read_text())["message"], "25.00% of game code")

    def test_missing_game_category_cannot_publish_progress(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "build").mkdir()
            data = report()
            data["categories"] = data["categories"][1:]
            (root / "build/report.json").write_text(json.dumps(data))
            with patch.object(progress, "ROOT", root), patch("sys.argv", ["progress.py", "--no-report"]):
                with self.assertRaisesRegex(SystemExit, "no game category"):
                    progress.main()
            self.assertFalse((root / "PROGRESS.md").exists())


if __name__ == "__main__":
    unittest.main()

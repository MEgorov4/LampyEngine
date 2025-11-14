import json
import tempfile
import unittest
from pathlib import Path

from src.core.validation import create_project_scaffold, load_project_descriptor
from src.core.errors import JsonProjectError


class ProjectBrowserCoreTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp_dir = tempfile.TemporaryDirectory()
        self.base = Path(self.temp_dir.name)

    def tearDown(self) -> None:
        self.temp_dir.cleanup()

    def _make_valid_project(self) -> Path:
        project_dir = self.base / "ValidProject"
        for folder in ("Resources", "Build", "Config", "Saved"):
            (project_dir / folder).mkdir(parents=True, exist_ok=True)
        payload = {
            "projectPath": str(project_dir),
            "projectName": "ValidProject",
            "resourcesPath": str(project_dir / "Resources"),
            "buildPath": str(project_dir / "Build"),
            "configPath": str(project_dir / "Config"),
            "logsPath": str(project_dir / "Saved"),
            "editorStartWorld": "default",
            "gameStartWorld": "default",
            "result": "success",
        }
        lproj = project_dir / "ValidProject.lproj"
        lproj.write_text(json.dumps(payload), encoding="utf-8")
        return lproj

    def test_load_valid_project(self) -> None:
        lproj_path = self._make_valid_project()
        payload = load_project_descriptor(lproj_path)
        self.assertEqual(payload["projectName"], "ValidProject")
        self.assertEqual(payload["result"], "success")

    def test_create_project_scaffold(self) -> None:
        payload, lproj_path = create_project_scaffold(self.base, "MyGame", None)
        self.assertTrue(lproj_path.exists())
        for folder in ("Resources", "Build", "Config", "Saved"):
            self.assertTrue((self.base / "MyGame" / folder).exists())
        self.assertEqual(payload["projectName"], "MyGame")

    def test_load_corrupted_project(self) -> None:
        bad_file = self.base / "Broken.lproj"
        bad_file.write_text("{invalid json", encoding="utf-8")
        with self.assertRaises(JsonProjectError):
            load_project_descriptor(bad_file)


if __name__ == "__main__":
    unittest.main()

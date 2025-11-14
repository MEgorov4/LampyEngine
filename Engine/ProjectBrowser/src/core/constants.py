from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parents[2]
RECENT_FILE = ROOT_DIR / "recent.json"
TEMPLATES_DIR = ROOT_DIR / "Templates"
ICON_PATH = ROOT_DIR / "app_icon.png"
SUMMARY_DISPLAY_MS = 1000
REQUIRED_KEYS = (
    "projectPath",
    "projectName",
    "resourcesPath",
    "buildPath",
    "configPath",
    "logsPath",
    "editorStartWorld",
    "gameStartWorld",
)
REQUIRED_DIRS = ("Resources", "Build", "Config", "Saved")

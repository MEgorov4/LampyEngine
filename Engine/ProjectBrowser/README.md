# Project Browser v2

PySide6 desktop utility for managing `.lproj` game projects with strict JSON responses, modern UI, template-aware project creation, and a recent-projects sidebar.

## Highlights
- Open existing `.lproj` files with schema + directory validation, ANSI/UTF-8 detection, and granular error dialogs (JSON/structure/access/extension/I/O).
- Create projects with smart name validation, template selection (`Templates/`), permission checks, and automatic scaffolding (`Resources/`, `Build/`, `Config/`, `Saved/`).
- Summary toast (1 second) after successful open/create before emitting JSON and exiting.
- Recent-project list (persisted in `recent.json`), drag-and-drop opening, light/dark theme toggle, animated buttons, and exit confirmation.
- All terminal output remains JSON (`{"result":"success"|"fail"|"exit"}`) matching exit codes 0/1.

## Requirements
- Python 3.9+
- PySide6 (`pip install PySide6`)
- PyInstaller (`pip install pyinstaller`) — used by CMake to bundle the tool into a standalone `ProjectBrowser.exe`.

## Project Structure
- `main.py` — lightweight entrypoint calling `src.app.main()`.
- `src/core/` — validation, error types, constants, and recent-project storage.
- `src/ui/` — themed widgets and dialogs.
- `Templates/`, `SampleProject/`, `recent.json` — user-facing assets/data.

## Run
```bash
pip install PySide6
python main.py
```

## Tests
```bash
python -m unittest tests.test_core
```
Covers loading a valid project, scaffolding creation, and corrupted JSON handling.

## Standalone Build (PyInstaller)
The CMake target `ProjectBrowser` runs PyInstaller automatically and copies the resulting `ProjectBrowser.exe` into the engine's runtime output (`build/bin` by default). Ensure the required Python packages are installed, then trigger a build:

```bash
pip install PySide6 pyinstaller
cmake --build build --target ProjectBrowser --config Debug
```

PyInstaller caches go into `Engine/ProjectBrowser/pyinstaller/`. Delete that folder if you need a completely clean rebuild.

## Assets & Samples
- `Templates/Basic`, `Templates/FPS` — starter layouts copied when creating a project (use the "Template" dropdown).
- `SampleProject/SampleProject.lproj` — ready-made descriptor demonstrating the expected JSON format.
- `recent.json` — persists up to 10 recently successful `.lproj` paths.
- `app_icon.png`, `project-browser-dark.png`, `project-browser-light.png` — UI assets & screenshots.

## Usage Flow
1. **Open Project** — select a `.lproj`; upon success a summary toast appears, JSON is printed, exit code 0.
2. **Create Project** — choose a root, name, and template; scaffolding + `.lproj` written, summary toast, JSON, exit code 0.
3. **Recent buttons** — instantly reopen prior projects; missing entries auto-prune.
4. **Exit** — confirmation dialog; `{"result":"exit"}`, exit code 1.

Errors display categorized dialogs and either keep the app running (recoverable validation/permission issues) or emit `{ "result": "fail" }` before exiting for fatal problems (e.g., corrupted project files).

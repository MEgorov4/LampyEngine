from __future__ import annotations

import json
import locale
import os
import shutil
from pathlib import Path
from typing import Any, Dict, Optional, Tuple

from .constants import REQUIRED_DIRS, REQUIRED_KEYS
from .errors import (
    FileExtensionError,
    FileIOProjectError,
    JsonProjectError,
    PermissionDeniedError,
    ProjectNameError,
    ProjectError,
    StructureProjectError,
)


def validate_project_name(raw_name: str, base_dir: Optional[Path] = None) -> str:
    cleaned = raw_name.strip()
    if not cleaned:
        raise ProjectNameError("Project name cannot be empty.", fatal=False)
    if cleaned.startswith("."):
        raise ProjectNameError("Project name cannot start with a dot.", fatal=False)
    if len(cleaned) > 64:
        raise ProjectNameError("Project name must be under 64 characters.", fatal=False)
    invalid_chars = set('/\\:*?"<>|')
    if any(char in invalid_chars for char in cleaned):
        raise ProjectNameError("Project name contains invalid characters.", fatal=False)
    if base_dir is not None and (base_dir / cleaned).exists():
        raise ProjectNameError("Project already exists in the selected directory.", fatal=False)
    return cleaned


def validate_extension(path: Path) -> None:
    if path.suffix.lower() != ".lproj":
        raise FileExtensionError("Selected file must have a .lproj extension.")


def safe_read_json(path: Path) -> Dict[str, Any]:
    encodings = ["utf-8", locale.getpreferredencoding(False), "cp1251", "cp1252"]
    last_decode_error: Optional[Exception] = None
    for encoding in encodings:
        if not encoding:
            continue
        try:
            text = path.read_text(encoding=encoding)
            return json.loads(text)
        except UnicodeDecodeError as exc:
            last_decode_error = exc
            continue
        except json.JSONDecodeError as exc:
            raise JsonProjectError("Project file is corrupted.") from exc
    raise JsonProjectError("Project file is corrupted.") from last_decode_error


def ensure_structure(data: Dict[str, Any]) -> None:
    missing_keys = [key for key in REQUIRED_KEYS if key not in data]
    if missing_keys:
        raise StructureProjectError(f"Project structure missing keys: {', '.join(missing_keys)}")
    dir_mapping = {
        "Resources": data["resourcesPath"],
        "Build": data["buildPath"],
        "Config": data["configPath"],
        "Saved": data["logsPath"],
    }
    for label, path_str in dir_mapping.items():
        path = Path(path_str)
        if not path.exists() or not path.is_dir():
            raise StructureProjectError(f"Missing required directory: {label}")
    project_root = Path(data["projectPath"])
    for folder in REQUIRED_DIRS:
        candidate = project_root / folder
        if not candidate.exists() or not candidate.is_dir():
            raise StructureProjectError(f"Project folder missing: {folder}")


def build_payload(project_dir: Path, project_name: str) -> Dict[str, Any]:
    project_dir = project_dir.resolve()
    return {
        "projectPath": str(project_dir),
        "projectName": project_name,
        "resourcesPath": str(project_dir / "Resources"),
        "buildPath": str(project_dir / "Build"),
        "configPath": str(project_dir / "Config"),
        "logsPath": str(project_dir / "Saved"),
        "editorStartWorld": "default",
        "gameStartWorld": "default",
        "result": "success",
    }


def load_project_descriptor(path: Path) -> Dict[str, Any]:
    validate_extension(path)
    try:
        data = safe_read_json(path)
    except PermissionError as exc:
        raise PermissionDeniedError("Not enough permissions to read project file.", fatal=True) from exc
    except OSError as exc:
        raise FileIOProjectError("Unable to read project file.") from exc
    ensure_structure(data)
    data["result"] = "success"
    return data


def ensure_writable_directory(directory: Path) -> None:
    if not os.access(directory, os.W_OK):
        raise PermissionDeniedError("Not enough permissions to create project in this folder.", fatal=False)


def create_project_scaffold(base_dir: Path, project_name: str, template_path: Optional[Path]) -> Tuple[Dict[str, Any], Path]:
    if not base_dir.exists() or not base_dir.is_dir():
        raise FileIOProjectError("Base directory does not exist.")
    ensure_writable_directory(base_dir)
    validated_name = validate_project_name(project_name, base_dir)
    project_dir = base_dir / validated_name

    try:
        project_dir.mkdir(parents=False, exist_ok=False)
        if template_path and template_path.exists():
            for item in template_path.iterdir():
                destination = project_dir / item.name
                if item.is_dir():
                    shutil.copytree(item, destination, dirs_exist_ok=True)
                else:
                    shutil.copy2(item, destination)
        for folder in REQUIRED_DIRS:
            (project_dir / folder).mkdir(exist_ok=True)
        payload = build_payload(project_dir, validated_name)
        lproj_path = project_dir / f"{validated_name}.lproj"
        lproj_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
        return payload, lproj_path
    except PermissionError as exc:
        raise PermissionDeniedError("Not enough permissions to create project in this folder.", fatal=False) from exc
    except OSError as exc:
        raise FileIOProjectError("Failed to create project files.") from exc

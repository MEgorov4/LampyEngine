from __future__ import annotations

import json
from pathlib import Path
from typing import List


class RecentProjectsStore:
    def __init__(self, path: Path, limit: int = 10) -> None:
        self._path = path
        self._limit = limit
        self._recent = self._load()

    def _load(self) -> List[str]:
        if not self._path.exists():
            return []
        try:
            data = json.loads(self._path.read_text(encoding="utf-8"))
            recent = data.get("recent", [])
            return [str(Path(item)) for item in recent if isinstance(item, str)]
        except Exception:
            return []

    def _save(self) -> None:
        try:
            self._path.write_text(json.dumps({"recent": self._recent}, indent=2), encoding="utf-8")
        except Exception:
            pass

    def add(self, path: Path) -> None:
        string_path = str(path)
        items = [string_path] + [item for item in self._recent if item != string_path]
        self._recent = items[: self._limit]
        self._save()

    def remove(self, path: Path) -> None:
        string_path = str(path)
        self._recent = [item for item in self._recent if item != string_path]
        self._save()

    def items(self) -> List[Path]:
        return [Path(item) for item in self._recent]

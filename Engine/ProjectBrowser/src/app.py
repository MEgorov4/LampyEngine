from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Dict, Optional

from PySide6.QtCore import QMimeData, QTimer, Qt
from PySide6.QtGui import QIcon
from PySide6.QtWidgets import (
    QApplication,
    QFileDialog,
    QHBoxLayout,
    QLabel,
    QMessageBox,
    QPushButton,
    QScrollArea,
    QVBoxLayout,
    QWidget,
)

from .core.constants import ICON_PATH, RECENT_FILE, SUMMARY_DISPLAY_MS, TEMPLATES_DIR
from .core.errors import (
    FileExtensionError,
    FileIOProjectError,
    JsonProjectError,
    PermissionDeniedError,
    ProjectError,
    StructureProjectError,
)
from .core.recent import RecentProjectsStore
from .core.validation import create_project_scaffold, load_project_descriptor
from .ui.dialogs import CreateProjectDialog
from .ui.theme import THEMES
from .ui.widgets import AnimatedButton, SummaryWindow


class ProjectBrowser(QWidget):
    def __init__(self) -> None:
        super().__init__()
        self.recent_store = RecentProjectsStore(RECENT_FILE)
        self._template_names = self._discover_templates()
        self._current_theme_key = "dark"
        self._summary_window: Optional[SummaryWindow] = None
        self._animated_buttons: list[AnimatedButton] = []
        self._recent_buttons: list[AnimatedButton] = []
        self._setup_ui()
        self._apply_theme()
        self._refresh_recent_list()

    def _setup_ui(self) -> None:
        self.setWindowTitle("Project Browser")
        self.setMinimumSize(420, 260)
        self.setAcceptDrops(True)
        app = QApplication.instance()
        if ICON_PATH.exists():
            icon = QIcon(str(ICON_PATH))
            self.setWindowIcon(icon)
            if app is not None:
                app.setWindowIcon(icon)
        root = QVBoxLayout(self)
        root.setContentsMargins(24, 24, 24, 24)
        root.setSpacing(16)

        header = QHBoxLayout()
        title = QLabel("Project Browser", self)
        title.setStyleSheet("font-size: 18px; font-weight: 600;")
        header.addWidget(title)
        header.addStretch()
        self.theme_toggle = QPushButton("Light Mode", self)
        self.theme_toggle.setCursor(Qt.PointingHandCursor)
        self.theme_toggle.clicked.connect(self._toggle_theme)
        header.addWidget(self.theme_toggle)
        root.addLayout(header)

        self.open_button = AnimatedButton("Open Project")
        self.open_button.clicked.connect(self._handle_open_project)
        self.create_button = AnimatedButton("Create Project")
        self.create_button.clicked.connect(self._handle_create_project)
        self.exit_button = AnimatedButton("Exit")
        self.exit_button.clicked.connect(self._handle_exit)

        for button in (self.open_button, self.create_button, self.exit_button):
            self._animated_buttons.append(button)
            root.addWidget(button)

        recent_label = QLabel("Recent:", self)
        root.addWidget(recent_label)

        scroll = QScrollArea(self)
        scroll.setWidgetResizable(True)
        scroll.setFrameShape(QScrollArea.NoFrame)
        self.recent_container = QWidget()
        self.recent_layout = QVBoxLayout(self.recent_container)
        self.recent_layout.setSpacing(8)
        self.recent_layout.setContentsMargins(0, 0, 0, 0)
        scroll.setWidget(self.recent_container)
        root.addWidget(scroll, 1)

    def _discover_templates(self) -> list[str]:
        names = ["Empty"]
        if TEMPLATES_DIR.exists():
            for child in sorted(TEMPLATES_DIR.iterdir()):
                if child.is_dir():
                    names.append(child.name)
        return names

    def _toggle_theme(self) -> None:
        self._current_theme_key = "light" if self._current_theme_key == "dark" else "dark"
        self._apply_theme()

    def _apply_theme(self) -> None:
        theme = THEMES[self._current_theme_key]
        self.setStyleSheet(f"background-color: {theme.background}; color: {theme.text};")
        self.theme_toggle.setText("Dark Mode" if self._current_theme_key == "light" else "Light Mode")
        for button in self._animated_buttons:
            if button is self.exit_button:
                button.set_colors(
                    theme.button_secondary,
                    theme.button_secondary_hover,
                    theme.button_secondary_pressed,
                    theme.text,
                )
            else:
                text_color = "#ffffff" if self._current_theme_key == "dark" else theme.text
                button.set_colors(theme.button_primary, theme.button_primary_hover, theme.button_primary_pressed, text_color)
        for button in self._recent_buttons:
            button.set_colors(theme.recent_button, theme.recent_button_hover, theme.recent_button_pressed, theme.text)

    def _handle_open_project(self) -> None:
        file_path, _ = QFileDialog.getOpenFileName(self, "Select Project File", "", "Logic Project (*.lproj)")
        if not file_path:
            return
        self._open_project(Path(file_path))

    def _open_project(self, path: Path) -> None:
        try:
            payload = load_project_descriptor(path)
            self.recent_store.add(path)
            self._refresh_recent_list()
            self._show_summary_and_exit(payload, exit_code=0)
        except ProjectError as error:
            self._handle_project_error(error, path)

    def _handle_create_project(self) -> None:
        base_dir = QFileDialog.getExistingDirectory(self, "Select Project Root")
        if not base_dir:
            return
        dialog = CreateProjectDialog(self._template_names, self)
        if dialog.exec() != CreateProjectDialog.Accepted:
            return
        project_name, template_name = dialog.values()
        template_path = None
        if template_name != "Empty" and TEMPLATES_DIR.exists():
            candidate = TEMPLATES_DIR / template_name
            if candidate.exists():
                template_path = candidate
        try:
            payload, lproj_path = create_project_scaffold(Path(base_dir), project_name, template_path)
            self.recent_store.add(lproj_path)
            self._refresh_recent_list()
            self._show_summary_and_exit(payload, exit_code=0)
        except ProjectError as error:
            self._handle_project_error(error)

    def _handle_project_error(self, error: ProjectError, path: Optional[Path] = None) -> None:
        QMessageBox.critical(self, "Error", error.user_message)
        if isinstance(error, PermissionDeniedError) and not error.fatal:
            return
        if path is not None and isinstance(
            error,
            (FileExtensionError, StructureProjectError, JsonProjectError, FileIOProjectError),
        ):
            self.recent_store.remove(path)
            self._refresh_recent_list()
        if error.fatal:
            self._emit_fail_and_exit()

    def _handle_exit(self) -> None:
        reply = QMessageBox.question(
            self,
            "Exit",
            "Exit Project Browser?",
            QMessageBox.Yes | QMessageBox.No,
            QMessageBox.No,
        )
        if reply == QMessageBox.Yes:
            self._emit_and_exit({"result": "exit"}, exit_code=1)

    def _refresh_recent_list(self) -> None:
        while self.recent_layout.count():
            item = self.recent_layout.takeAt(0)
            widget = item.widget()
            if widget:
                widget.deleteLater()
        self._recent_buttons.clear()
        entries = self.recent_store.items()
        if not entries:
            placeholder = QLabel("No recent projects yet.", self.recent_container)
            self.recent_layout.addWidget(placeholder)
            return
        for path in entries:
            button = AnimatedButton(path.stem or path.name)
            button.setToolTip(str(path))
            button.clicked.connect(lambda checked=False, p=path: self._open_recent_path(p))
            self._recent_buttons.append(button)
            self.recent_layout.addWidget(button)
        self._apply_theme()

    def _open_recent_path(self, path: Path) -> None:
        if not path.exists():
            QMessageBox.warning(self, "Missing", "Recent project not found. Removing from list.")
            self.recent_store.remove(path)
            self._refresh_recent_list()
            return
        self._open_project(path)

    def dragEnterEvent(self, event) -> None:  # type: ignore[override]
        if self._is_valid_drag(event.mimeData()):
            event.acceptProposedAction()
        else:
            event.ignore()

    def dropEvent(self, event) -> None:  # type: ignore[override]
        for path in self._paths_from_mime(event.mimeData()):
            if path.suffix.lower() == ".lproj":
                self._open_project(path)
                event.acceptProposedAction()
                return
        event.ignore()

    def _is_valid_drag(self, mime: QMimeData) -> bool:
        return any(path.suffix.lower() == ".lproj" for path in self._paths_from_mime(mime))

    def _paths_from_mime(self, mime: QMimeData) -> list[Path]:
        if not mime.hasUrls():
            return []
        return [Path(url.toLocalFile()) for url in mime.urls() if url.isLocalFile()]

    def _show_summary_and_exit(self, payload: Dict[str, str], exit_code: int) -> None:
        self._summary_window = SummaryWindow(payload)
        self._summary_window.show()
        QTimer.singleShot(SUMMARY_DISPLAY_MS, lambda: self._emit_and_exit(payload, exit_code))

    def _emit_fail_and_exit(self) -> None:
        self._emit_and_exit({"result": "fail"}, exit_code=1)

    def _emit_and_exit(self, payload: Dict[str, str], exit_code: int) -> None:
        if self._summary_window is not None:
            self._summary_window.close()
            self._summary_window = None
        print(json.dumps(payload, ensure_ascii=False), flush=True)
        app = QApplication.instance()
        if app is not None:
            app.quit()
        sys.exit(exit_code)


def main() -> None:
    app = QApplication(sys.argv)
    browser = ProjectBrowser()
    browser.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()

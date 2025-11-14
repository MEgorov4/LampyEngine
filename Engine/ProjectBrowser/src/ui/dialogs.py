from __future__ import annotations

from typing import Sequence, Tuple

from PySide6.QtWidgets import QComboBox, QDialog, QDialogButtonBox, QLabel, QLineEdit, QVBoxLayout


class CreateProjectDialog(QDialog):
    def __init__(self, templates: Sequence[str], parent=None) -> None:
        super().__init__(parent)
        self.setWindowTitle("Create Project")
        layout = QVBoxLayout(self)
        self.name_edit = QLineEdit(self)
        self.name_edit.setPlaceholderText("Project name")
        layout.addWidget(QLabel("Project Name:"))
        layout.addWidget(self.name_edit)
        layout.addWidget(QLabel("Template:"))
        self.template_combo = QComboBox(self)
        for name in templates:
            self.template_combo.addItem(name)
        layout.addWidget(self.template_combo)
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

    def values(self) -> Tuple[str, str]:
        return self.name_edit.text(), self.template_combo.currentText()

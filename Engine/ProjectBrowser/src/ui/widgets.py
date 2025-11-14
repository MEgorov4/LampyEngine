from __future__ import annotations

from typing import Dict

from PySide6.QtCore import QEasingCurve, QVariantAnimation, Qt
from PySide6.QtGui import QColor
from PySide6.QtWidgets import QLabel, QPushButton, QWidget, QVBoxLayout


class AnimatedButton(QPushButton):
    def __init__(self, text: str) -> None:
        super().__init__(text)
        self._base_color = QColor("#2563eb")
        self._hover_color = QColor("#1d4ed8")
        self._pressed_color = QColor("#1e40af")
        self._text_color = "#ffffff"
        self._current_color = self._base_color
        self._animation = QVariantAnimation(self)
        self._animation.setDuration(180)
        self._animation.setEasingCurve(QEasingCurve.InOutQuad)
        self._animation.valueChanged.connect(self._apply_color)
        self.setCursor(Qt.PointingHandCursor)
        self.setMinimumHeight(40)
        self._apply_style()

    def set_colors(self, base: str, hover: str, pressed: str, text_color: str) -> None:
        self._base_color = QColor(base)
        self._hover_color = QColor(hover)
        self._pressed_color = QColor(pressed)
        self._text_color = text_color
        self._current_color = self._base_color
        self._apply_style()

    def enterEvent(self, event) -> None:  # type: ignore[override]
        self._animate_to(self._hover_color)
        super().enterEvent(event)

    def leaveEvent(self, event) -> None:  # type: ignore[override]
        self._animate_to(self._base_color)
        super().leaveEvent(event)

    def mousePressEvent(self, event) -> None:  # type: ignore[override]
        self._animate_to(self._pressed_color)
        super().mousePressEvent(event)

    def mouseReleaseEvent(self, event) -> None:  # type: ignore[override]
        target = self._hover_color if self.rect().contains(event.position().toPoint()) else self._base_color
        self._animate_to(target)
        super().mouseReleaseEvent(event)

    def _animate_to(self, color: QColor) -> None:
        self._animation.stop()
        self._animation.setStartValue(self._current_color)
        self._animation.setEndValue(color)
        self._animation.start()

    def _apply_color(self, color: QColor) -> None:
        self._current_color = color
        self._apply_style()

    def _apply_style(self) -> None:
        style = f"background-color: {self._current_color.name()}; color: {self._text_color}; border-radius: 10px; padding: 10px 16px; font-weight: 600; border: none;"
        self.setStyleSheet(style)


class SummaryWindow(QWidget):
    def __init__(self, payload: Dict[str, str]) -> None:
        super().__init__(None, Qt.Tool | Qt.FramelessWindowHint)
        self.setWindowTitle("Project Loaded Successfully")
        layout = QVBoxLayout(self)
        layout.setContentsMargins(20, 20, 20, 20)
        title = QLabel("Project Loaded Successfully", self)
        title.setStyleSheet("font-weight: 600; font-size: 16px;")
        layout.addWidget(title)
        layout.addSpacing(6)
        for label, key in (
            ("Project Path", "projectPath"),
            ("Resources", "resourcesPath"),
            ("Config", "configPath"),
            ("Build", "buildPath"),
            ("Saved", "logsPath"),
        ):
            row = QLabel(f"{label}: {payload.get(key, '')}", self)
            row.setWordWrap(True)
            layout.addWidget(row)
        self.adjustSize()

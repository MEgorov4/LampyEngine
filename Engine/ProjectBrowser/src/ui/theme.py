from __future__ import annotations

from dataclasses import dataclass


@dataclass
class ThemePalette:
    name: str
    background: str
    text: str
    button_primary: str
    button_primary_hover: str
    button_primary_pressed: str
    button_secondary: str
    button_secondary_hover: str
    button_secondary_pressed: str
    recent_button: str
    recent_button_hover: str
    recent_button_pressed: str


THEMES = {
    "dark": ThemePalette(
        name="Dark",
        background="#0f172a",
        text="#f8fafc",
        button_primary="#2563eb",
        button_primary_hover="#1d4ed8",
        button_primary_pressed="#1e40af",
        button_secondary="#475569",
        button_secondary_hover="#64748b",
        button_secondary_pressed="#475569",
        recent_button="#1e293b",
        recent_button_hover="#334155",
        recent_button_pressed="#0f172a",
    ),
    "light": ThemePalette(
        name="Light",
        background="#f8fafc",
        text="#0f172a",
        button_primary="#0f62fe",
        button_primary_hover="#0353c6",
        button_primary_pressed="#023c8a",
        button_secondary="#e2e8f0",
        button_secondary_hover="#cbd5f5",
        button_secondary_pressed="#94a3b8",
        recent_button="#e2e8f0",
        recent_button_hover="#cbd5f5",
        recent_button_pressed="#94a3b8",
    ),
}

from PyQt5.QtWidgets import QCheckBox


class LegendCheckBox(QCheckBox):
    def __init__(self, label, c_value, color='green', parent=None):
        super().__init__(label, parent)
        self.c_value = c_value
        self.setStyleSheet(f"QCheckBox {{ color: {color}; font-weight: bold; }}")

from PyQt5.QtWidgets import QMainWindow
from src.ui.gui.mainwindow import Ui_MainWindow


class MainWindowController(QMainWindow, Ui_MainWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        stylesheet = "./gui/resources/stylesheet.qss"
        with open(stylesheet, "r") as f:
            self.setStyleSheet(f.read())

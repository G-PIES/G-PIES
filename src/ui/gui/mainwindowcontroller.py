from PyQt5.QtWidgets import QMainWindow
#src.ui.gui not functioning correctly? when importing on line 4, Gave error : ModuleNotFoundError: No module named 'src'
#Solution : Added the folder this file is in "/gui" to the PYTHONPATH Env Variable. Removed leading "src.ui.gui." from the import statement
from mainwindow import Ui_MainWindow


class MainWindowController(QMainWindow, Ui_MainWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        stylesheet = "./gui/resources/stylesheet.qss"
        with open(stylesheet, "r") as f:
            self.setStyleSheet(f.read())

        self.setupUi(self)

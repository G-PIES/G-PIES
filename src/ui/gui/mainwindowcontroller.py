from PyQt5.QtWidgets import QMainWindow
from PyQt5.QtCore import QThread
#src.ui.gui not functioning correctly? when importing on line 4, Gave error : ModuleNotFoundError: No module named 'src'
#Solution : Added the folder this file is in "/gui" to the PYTHONPATH Env Variable. Removed leading "src.ui.gui." from the import statement
from mainwindow import Ui_MainWindow
from simulationworker import SimulationWorker


class MainWindowController(QMainWindow, Ui_MainWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)
        self.init_connections()

        stylesheet = "./gui/resources/stylesheet.qss"
        with open(stylesheet, "r") as f:
            self.setStyleSheet(f.read())

    def init_connections(self):
        self.pushButton_2.clicked.connect(self.start_simulation)

    def start_simulation(self):
        self.thread = QThread()
        self.worker = SimulationWorker(C=10, simulation_time=1.0, step=0.00001)
        self.worker.moveToThread(self.thread)
        self.thread.started.connect(self.worker.run)
        self.worker.finished.connect(self.text_browser_show_results)
        self.worker.finished.connect(self.thread.quit)
        self.worker.finished.connect(self.worker.deleteLater)
        self.thread.finished.connect(self.thread.deleteLater)
        self.thread.start()

    def text_browser_show_results(self, simulation_result):
        self.textBrowser.append(simulation_result)

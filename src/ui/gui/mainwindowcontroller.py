from PyQt5.QtWidgets import QMainWindow
from PyQt5.QtCore import QThread
from gui.mainwindow import Ui_MainWindow
from simulationworker import SimulationWorker
from mplcanvas import MplCanvas

import pandas as pd
import matplotlib.pyplot as plt


class MainWindowController(QMainWindow, Ui_MainWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)
        self.init_graph()
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
        self.worker.finished.connect(self.set_df)
        self.worker.finished.connect(self.thread.quit)
        self.worker.finished.connect(self.worker.deleteLater)
        self.thread.finished.connect(self.thread.deleteLater)
        self.thread.start()

    def init_graph(self):
        self.df = pd.DataFrame(columns =['Time (s)', 'Cluster Size', 'Interstitials / cm^3', 'Vacancies / cm^3'])
        self.sc = MplCanvas(self, width=5, height=4, dpi=100)
        self.df.plot.scatter(ax=self.sc.axes, x = 'Time (s)', y = 'Interstitials / cm^3')
        self.verticalLayout_2.addWidget(self.sc)

    def set_df(self, result):
        self.df = result

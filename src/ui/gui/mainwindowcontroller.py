from PyQt5.QtWidgets import QMainWindow
from PyQt5.QtCore import QThread
from gui.mainwindow import Ui_MainWindow
from simulationworker import SimulationWorker

from simulationprocess import SimulationProcess
from mplcanvas import MplCanvas
from multiprocessing import Process, Pipe

import pandas as pd
import matplotlib.pyplot as plt

import threading

import numpy as np

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
        self.pushButton_3.clicked.connect(self.stop_simulation)

    def start_simulation(self):
        self.parent_conn, child_conn = Pipe()
        self.sim_process = SimulationProcess(C=10, simulation_time=1.0, step=0.00001, pipe=child_conn)

        self.thread = threading.Thread(target=self.start_waiting_for_task)
        self.thread.start()

        print("simulation started")

    def stop_simulation(self):
        if(self.sim_process.is_alive()):
            self.sim_process.terminate()
            self.parent_conn.close()
            self.data = []
            self.data_x = []
            self.data_y = []
        print("simulation stopped")

    def init_graph(self):
        #plt.
        self._plot_ref = None
        self.data = []
        self.data_x = []
        self.data_y = []
        #self.df = pd.DataFrame(columns =['Time (s)', 'Cluster Size', 'Interstitials / cm^3', 'Vacancies / cm^3'])
        self.scatter_plot = MplCanvas(self, width=5, height=4, dpi=100)
        #self.df.plot.scatter(ax=self.sc.axes, x = 'Time (s)', y = 'Interstitials / cm^3')
        self.scatter_plot.axes.set_xlabel('Time (s)')
        self.scatter_plot.axes.set_ylabel('Interstitials / cm^3')
        #self.scatter_plot.axes.scatter()
        self.verticalLayout_2.addWidget(self.scatter_plot)
        self.scatter_plot.show()

    def start_waiting_for_task(self):
        self.sim_process.start()
        while(self.sim_process.is_alive()):
            self.update_graph()
        self.sim_process.join()
        print("simulation finished")

    def update_graph(self):
        self.data = self.parent_conn.recv()
        self.data_x.append(self.data[0])
        self.data_y.append(self.data[2])
        
        self.scatter_plot.axes.cla()
        self.scatter_plot.axes.scatter(x=self.data_x, y=self.data_y)
        self.scatter_plot.draw()

from PyQt5.QtCore import pyqtSignal, QObject
import sys
sys.path.append('../../lib')
import pyclusterdynamics as pycd

import pandas as pd


class SimulationWorker(QObject):
    finished = pyqtSignal(object)

    def __init__(self, C, simulation_time, step):
        super().__init__()
        self.C = C
        self.simulation_time = simulation_time
        self.step = step
        self.df = pd.DataFrame(columns =['Time (s)', 'Cluster Size', 'Interstitials / cm^3', 'Vacancies / cm^3'])

    def run(self):
        t = 0
        cd = pycd.Simulation(self.C, self.simulation_time, self.step)
        while t < self.simulation_time:
            cd.run()
            for i in range(1,self.C):
                self.df.loc[len(self.df)] = ([t, i, cd.get_int_idx(i), cd.get_vac_idx(i)])
            
            t = cd.get_state_time()

        self.finished.emit(self.df)

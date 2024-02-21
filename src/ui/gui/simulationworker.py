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

    def run(self):
        t = 0
        cd = pycd.Simulation(self.C, self.simulation_time, self.step)
        data = []
        while t < self.simulation_time:
            cd.run()
            for i in range(1, self.C):
                data.append([t, i, cd.get_int_idx(i), cd.get_vac_idx(i)])

            t = cd.get_state_time()

        df = pd.DataFrame(data, columns=['Time (s)', 'Cluster Size', 'Interstitials / cm^3', 'Vacancies / cm^3'])
        self.finished.emit(df)

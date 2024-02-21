from PyQt5.QtCore import pyqtSignal, QObject
import sys
sys.path.append('../../lib')
import pyclusterdynamics as pycd


class SimulationWorker(QObject):
    finished = pyqtSignal(str)

    def __init__(self, C, simulation_time, step):
        super().__init__()
        self.C = C
        self.simulation_time = simulation_time
        self.step = step

    def run(self):
        t = 0
        cd = pycd.Simulation(self.C, self.simulation_time, self.step)
        while t < self.simulation_time:
            cd.run()
            t = cd.get_state_time()
        string = cd.string_state()
        self.finished.emit(string)

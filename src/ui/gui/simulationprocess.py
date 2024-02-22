from multiprocessing import Process, Pipe

import sys
sys.path.append('../../lib')
import pyclusterdynamics as pycd

import pandas as pd

class SimulationProcess(Process):
    def __init__(self, C, simulation_time, step, pipe):
        super().__init__()
        self.C = C
        self.simulation_time = simulation_time
        self.step = step
        self.pipe = pipe

    def run(self):
        t = 0
        cd = pycd.Simulation(self.C, self.simulation_time, self.step)
        data = []
        while t < self.simulation_time:
            cd.run()
            for i in range(1, self.C):
                self.pipe.send([t, i, cd.get_int_idx(i), cd.get_vac_idx(i)])

            t = cd.get_state_time()
        
        self.pipe.close()
        
        print("Finished")

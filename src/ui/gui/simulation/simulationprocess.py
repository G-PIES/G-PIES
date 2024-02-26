from multiprocessing import Process, shared_memory
import numpy as np
import sys
sys.path.append('../../lib')
import pyclusterdynamics as pycd


class SimulationProcess(Process):
    def __init__(self, simulation_params, read_interval, shm_name, shm_ready):
        super().__init__()
        self.params = simulation_params
        # Number of time steps between state reads, limits usage of pycd getters and shared memory writes
        self.read_interval = read_interval
        # Shared memory
        self.shm_name = shm_name
        self.shm_ready = shm_ready

    def run(self):
        shm = shared_memory.SharedMemory(name=self.shm_name)
        data_block = np.ndarray((self.params.runner_block_size, self.params.C - 1, self.params.entry_size), dtype=np.float64, buffer=shm.buf)

        t = 0
        block_idx = 0
        cd = pycd.Simulation(self.params.C, self.params.simulation_time, self.params.step)

        # Run simulation, read / store data every read_interval number of iterations,
        # set data_ready_event every runner_block_size number of completed read_intervals
        while t < self.params.simulation_time:
            for _ in range(self.read_interval):
                cd.run()
            for i in range(1, self.params.C):
                data_block[block_idx, i - 1, :] = [t, i, cd.get_int_idx(i), cd.get_vac_idx(i)]

            block_idx += 1
            if block_idx >= self.params.runner_block_size:
                self.shm_ready.set()
                block_idx = 0

            t = cd.get_state_time()

        print("Finished")


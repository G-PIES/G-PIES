import numpy as np
from multiprocessing import shared_memory
from PyQt5.QtCore import QObject, pyqtSignal


class DataAccumulator(QObject):
    data_processed_signal = pyqtSignal()

    def __init__(self, simulation_params, shm_name, shm_ready, max_data_points=150):
        super().__init__()
        self.simulation_params = simulation_params
        self.shm_name = shm_name
        self.shm_ready = shm_ready
        # Threshold for downsampling to be applied
        self.max_data_points = max_data_points
        self.data = np.empty((0, self.simulation_params.C - 1, self.simulation_params.entry_size), dtype=np.float64)
        # Tracks downsampling depth
        self.index_distance = 1

    def run(self):
        block_size = self.simulation_params.runner_block_size
        data_point_size = self.simulation_params.entry_size
        shm = shared_memory.SharedMemory(name=self.shm_name)

        # Accumulates data from the shared memory buffer as fast as it can
        # Ensures total stored data doesn't exceed max_data_points, increasing downsampling
        # if this occurs. Incoming data is downsampled to the current depth of the stored data
        while True:
            self.shm_ready.wait()
            # Receive data block and downsample to current downsampling depth of stored data
            new_data = np.ndarray((block_size, self.simulation_params.C - 1, data_point_size), dtype=np.float64, buffer=shm.buf)
            downsampled_new_data = new_data[::self.index_distance]
            self.data = np.append(self.data, downsampled_new_data, axis=0)
            # Increase downsampling depth if length exceeds max_data_points
            if self.data.shape[0] >= self.max_data_points:
                self.data = self.data[::2]
                self.index_distance *= 2

            self.data_processed_signal.emit()
            self.shm_ready.clear()

class SimulationParams:
    def __init__(self, C, simulation_time, step, config_name, entry_size, runner_block_size=1000):
        # Simulation params
        self.C = C
        self.simulation_time = simulation_time
        self.step = step
        # Number of items per data point entry (per C value)
        self.config_name = config_name
        self.entry_size = entry_size
        # Data points per shared memory block
        self.runner_block_size = runner_block_size

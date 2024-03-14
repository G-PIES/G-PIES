from PyQt5.QtWidgets import QMainWindow
from PyQt5.QtCore import QThread, QDateTime

from multiprocessing import shared_memory, Event
import threading

from gui.mainwindow import Ui_MainWindow
from gui.simulation.simulationprocess import SimulationProcess
from gui.simulation.simulationparams import SimulationParams
from gui.visualization.dataaccumulator import DataAccumulator
from gui.visualization.graphcontroller import GraphController
from gui.mainwindow import InputDialog
import pyclusterdynamics as pycd
import sys
sys.path.append('../G-PIES/lib')


class MainWindowController(QMainWindow, Ui_MainWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)
        self.init_connections()
        self.reactor=0
        self.material= pycd.Sim_Material()
        self.reactor = pycd.Sim_Reactor()
        

        stylesheet = "./gui/resources/stylesheet.qss"
        with open(stylesheet, "r") as f:
            self.setStyleSheet(f.read())

        self.gc = GraphController(self.verticalLayout_2, width=5, height=4, dpi=100)
        self.gc.init_graph(1)
        self.sim_running = False

    def init_connections(self):
        self.pushButton_2.clicked.connect(self.start_simulation)
        self.pushButton_3.clicked.connect(self.stop_simulation)
        self.pushButton_4.clicked.connect(self.get_settings)
        
    def get_settings(self):
        input_dialog = InputDialog(self)
        result = input_dialog.exec_()

        inputs = input_dialog.getInputs()
        temperature=float(inputs[0])
        self.reactor.set_temperature(temperature)
        self.material.set_atomic_volume(int(inputs[1]))
        print("First text:", inputs[0])
        print("Second text:", inputs[1])
       
    
    def start_simulation(self):
        if self.sim_running:
            self.stop_simulation()

        # To be initialized by GUI parameter entry handling
        # I added in very simple error checking for the user param here
        try:
            userInputTime = float(self.lineEdit.text())
        except ValueError:
            userInputTime=1.0
        
        try:
            userInputC = int(self.lineEdit_2.text())
        except ValueError:
            userInputC=10
        
        try:
            userInputStep = float(self.lineEdit_3.text())
        except ValueError:
            userInputStep=0.00001
            
        try:
            userInputEntrySize = int(self.lineEdit_4.text())
        except ValueError:
            userInputEntrySize=4
       
        
        self.params = SimulationParams(userInputC, userInputTime, userInputStep, userInputEntrySize, runner_block_size=1000)
        self.gc.init_graph(self.params.C)

        # Init shared memory for SimulationProcess / DataAccumulator communication
        shm_size = self.params.runner_block_size * (self.params.C - 1) * self.params.entry_size * 8
        self.shm = shared_memory.SharedMemory(create=True, size=shm_size)
        self.shm_name = self.shm.name
        self.shm_ready = Event()

        # Init SimulationProcess and start
        self.sim_running = True
        self.start_time = QDateTime.currentDateTime()
        
        
        
        self.sim_process = SimulationProcess(simulation_params=self.params,
                                             read_interval=10, shm_name=self.shm_name, shm_ready=self.shm_ready,sim_material=self.material,sim_reactor=self.reactor)
        
        self.thread = threading.Thread(target=self.start_waiting_for_task)
        self.thread.start()
        print("simulation started")

    def stop_simulation(self):
        # ignore stop requests if sim is not running
        if self.sim_running:
            self.sim_running = False
            if self.sim_process.is_alive():
                self.sim_process.terminate()
            if self.accumulator_thread.isRunning():
                self.accumulator_thread.quit()
            if self.shm:
                self.shm.close()
                self.shm.unlink()
            print("simulation stopped")

    def start_waiting_for_task(self):
        self.data_accumulator = DataAccumulator(self.params, self.shm_name, self.shm_ready)
        self.data_accumulator.data_processed_signal.connect(self.update_graph)

        self.accumulator_thread = QThread()
        self.data_accumulator.moveToThread(self.accumulator_thread)
        self.accumulator_thread.started.connect(self.data_accumulator.run)
        self.accumulator_thread.start()

        self.sim_process.start()
        self.sim_process.join()
        print("simulation finished")

        if self.sim_running:
           self.stop_simulation()

    def update_graph(self):
        elapsed = self.start_time.msecsTo(QDateTime.currentDateTime()) / 1000.0
        # TODO: this changes the width of the parent layout ...
        self.label.setText(f"Time Elapsed: {elapsed:.2f}s                Simulation Time:")
        self.gc.update_graph(self.data_accumulator)

    def closeEvent(self, event):
        if self.sim_running:
            self.stop_simulation()

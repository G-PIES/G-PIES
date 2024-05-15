from PyQt5.QtWidgets import QMainWindow, QVBoxLayout
from PyQt5.QtCore import QThread, QDateTime, Qt, pyqtSignal
from PyQt5.QtGui import QIntValidator, QDoubleValidator
from PyQt5 import QtWidgets

from multiprocessing import shared_memory, Event
import threading

from gui.mainwindow import Ui_MainWindow
from gui.inputDialog import InputDialog

from gui.simulation.simulationprocess import SimulationProcess
from gui.simulation.simulationparams import SimulationParams
from gui.visualization.dataaccumulator import DataAccumulator
from gui.visualization.graphcontroller import GraphController
from gui.visualization.legendcheckbox import LegendCheckBox

import libpyclusterdynamics as pycd
import sys
import os
import yaml
sys.path.append('../G-PIES/out')


class MainWindowController(QMainWindow, Ui_MainWindow):
    simulationFinished = pyqtSignal()
    npcsvarry = [] 

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)
        self.input_dialog = InputDialog(self)
        #self.init_element_states() removed
        self.init_connections()
        self.reactor=0
        self.material= pycd.Sim_Material()
        self.reactor = pycd.Sim_Reactor()

        stylesheet = "./gui/resources/stylesheet.qss"
        with open(stylesheet, "r") as f:
            self.setStyleSheet(f.read())

        self.gc = GraphController(self.maingraph, width=5, height=4, dpi=100)
        self.gc.init_graph(1)
        self.sim_running = False

    def init_element_states(self):
        if self.groupBox.layout() is None:
            layout = QVBoxLayout()
            self.groupBox.setLayout(layout)
        self.simLenEntry.setText('1.0')
        self.clusterSizeEntry.setText('10')
        self.timeStepEntry.setText('0.00001')
        self.entrySizeEntry.setText('4')

        self.simLenEntry.setValidator(QDoubleValidator())
        self.clusterSizeEntry.setValidator(QIntValidator())
        self.timeStepEntry.setValidator(QDoubleValidator())
        self.entrySizeEntry.setValidator(QIntValidator())

        self.input_dialog.reactor_temp.setValidator(QDoubleValidator())
        self.input_dialog.atomic_volume.setValidator(QIntValidator())

    def init_connections(self):
        self.simulationFinished.connect(self.on_simulation_finished)

        self.startButton.clicked.connect(self.start_simulation)
        self.stopButton.clicked.connect(self.stop_simulation)
        self.actionReactor_Settings.triggered.connect(self.get_reactor_settings)
        self.actionMaterial_Settings.triggered.connect(self.get_material_settings)
        self.actionExportAs.triggered.connect(self.export_as)
        self.actionImport.triggered.connect(self.import_yaml)

    def export_as(self):
        options = QtWidgets.QFileDialog.Options()
        options |= QtWidgets.QFileDialog.DontUseNativeDialog
        fileName, _ = QtWidgets.QFileDialog.getSaveFileName(self, 
            "Save File", "", "CSV File(*.csv);;All Files(*)", options = options)
        if fileName:
            with open(fileName, 'w') as f:
                f.write(''.join(self.npcsvarry))
            self.fileName = fileName
            self.setWindowTitle(str(os.path.basename(fileName)) )

    def import_yaml(self):
        config_name = 'config.yaml'
        with open('../../'+config_name, 'r') as file:
            sim_config = yaml.safe_load(file)
        if (sim_config):
            print(config_name + " imported successfully")
            #print(sim_config)

        try:
            temp = sim_config['reactor']['temperature-kelvin']
            self.reactor.set_temperature(temp)
        except:
            self.reactor.set_temperature(603.15)

        try:
            temp = sim_config['reactor']['flux-dpa-s']
            self.reactor.set_flux(temp)
        except:
            self.reactor.set_flux(2.9e-7)
        
        #....
            

    def get_reactor_settings(self):
        
        
        self.input_dialog.getReactorSettings()
        self.input_dialog.exec_()
        inputs = self.input_dialog.getReactorInputs()
        if inputs[0] != '':  
            self.reactor.set_temperature(float(inputs[0]))
        if inputs[1] != '':  
            self.reactor.set_flux(float(inputs[1]))
        if inputs[2] != '':  
            self.reactor.set_recombination(float(inputs[2]))
        if inputs[3] != '':  
            self.reactor.set_i_bi(float(inputs[3]))
        if inputs[4] != '':  
            self.reactor.set_i_tri(float(inputs[4]))
        if inputs[5] != '':  
            self.reactor.set_i_quad(float(inputs[5]))
        if inputs[6] != '':  
            self.reactor.set_dislocation_density_evolution(float(inputs[6]))
        
            

    def get_material_settings(self):
        self.input_dialog.getMaterialSettings()
        self.input_dialog.exec_()
        inputs = self.input_dialog.getMaterialInputs()
        if inputs[0] != '':  
            self.material.set_atomic_volume(float(inputs[0]))
        if inputs[1] != '':  
            self.material.set_i_migration(float(inputs[1]))
        if inputs[2] != '':  
            self.material.set_i_diffusion_0(float(inputs[2]))
        if inputs[3] != '':  
            self.material.set_i_formation(float(inputs[3]))
        if inputs[4] != '':  
            self.material.set_i_binding(float(inputs[4]))
        if inputs[5] != '':  
            self.material.set_recombination_radius(float(inputs[5]))
        if inputs[6] != '':  
            self.material.set_i_loop_bias(float(inputs[6]))
        if inputs[7] != '':  
            self.material.set_i_dislocation_bias(float(inputs[7]))
        if inputs[8] != '':  
            self.material.set_i_dislocation_bias_param(float(inputs[8]))
        if inputs[9] != '':  
            self.material.set_dislocation_density_0(float(inputs[9]))
        if inputs[10] != '':  
            self.material.set_grain_size(float(inputs[10]))
        if inputs[11] != '':  
            self.material.set_lattice_param(float(inputs[11]))
        if inputs[12] != '':  
            self.material.set_burgers_vector(float(inputs[12]))
        
    def start_simulation(self):
        if self.sim_running:
            self.stop_simulation()

        userInputTime = float(self.simLenEntry.text())
        userInputC = int(self.clusterSizeEntry.text())
        userInputStep = float(self.timeStepEntry.text())
        userInputEntrySize = int(self.entrySizeEntry.text())

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
        self.data_accumulator.data_processed_signal.connect(self.update_csv)

        self.accumulator_thread = QThread()
        self.data_accumulator.moveToThread(self.accumulator_thread)
        self.accumulator_thread.started.connect(self.data_accumulator.run)
        self.accumulator_thread.start()

        self.sim_process.start()
        self.sim_process.join()
        print("simulation finished")

        if self.sim_running:
            self.simulationFinished.emit()

    def on_simulation_finished(self):
        self.stop_simulation()
        self.populate_legend()

    def update_csv(self):
        self.npcsvarry.append(self.data_accumulator.data)
        #print(self.npcsvarry)

    def update_graph(self):
        elapsed = self.start_time.msecsTo(QDateTime.currentDateTime()) / 1000.0
        # TODO: this changes the width of the parent layout ...
        self.timeLabel.setText(f"Time Elapsed: {elapsed:.2f}s                Simulation Time:")
        self.gc.update_graph(self.data_accumulator)

    def populate_legend(self):
        legend_items = self.gc.get_legend_items()

        # Remove any existing legend items
        for i in range(self.groupBox.layout().count()):
            widget = self.groupBox.layout().itemAt(i).widget()
            if widget:
                widget.deleteLater()

        # Create legend items and connect
        for c_value, (label, color) in legend_items.items():
            checkbox = LegendCheckBox(label, c_value, color=color, parent=self)
            checkbox.setChecked(True)
            checkbox.stateChanged.connect(self.checkbox_state_changed)
            self.groupBox.layout().addWidget(checkbox)

    def checkbox_state_changed(self, state):
        checkbox = self.sender()  # get emitter widget
        if isinstance(checkbox, LegendCheckBox):
            self.toggle_line_visibility(checkbox.c_value, state)

    def toggle_line_visibility(self, c_value, state):
        line = self.gc.lines.get(c_value)
        if line:
            line.set_visible(state == Qt.Checked)
            # Rescale / redraw visible lines only on any LegendCheckBox state change
            self.gc.scatter_plot.axes.relim(visible_only=True)
            self.gc.scatter_plot.axes.autoscale_view(True, True, True)
            self.gc.scatter_plot.figure.canvas.draw_idle()

    def closeEvent(self, event):
        if self.sim_running:
            self.stop_simulation()

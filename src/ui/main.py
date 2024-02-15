from PyQt5.Qt import QApplication
from gui.mainwindowcontroller import MainWindowController

import sys
sys.path.append('../../lib')
import pyclusterdynamics as pycd

simulation_time = 1.0
C = 10
step = 0.00001

app = QApplication([])
window = MainWindowController()

def startSimulation():
    t = 0
    cd = pycd.Simulation(C, simulation_time, step)
    
    while t < simulation_time:
        cd.run()
        t = cd.get_state_time()
    
    string = cd.string_state()
    window.textBrowser.append(string)
    #cd.print_state()

window.pushButton_2.clicked.connect(startSimulation)

window.show()
app.exec_()
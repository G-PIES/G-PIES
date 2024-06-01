import sys
import os
sys.path.append('../../out')
import libpyclusterdynamics as pycd

cd = pycd.Simulation("/home/ben/Dev/G-PIES/src/ui/config.yaml")

cd.run_full_simulation(0, 1)
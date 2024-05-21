import sys
import os
sys.path.append('../../out')
import libpyclusterdynamics as pycd

cd = pycd.Simulation("/home/ben/Dev/G-PIES/src/ui/config.yaml")

t = 0
print(" [DEBUG] simulation-time: ", cd.get_simulation_time())
iterations = 0
while t < cd.get_simulation_time():
  print(" [DEBUG] state-time: ", cd.get_state_time(), " : ", iterations)
  cd.run()
  t = cd.get_state_time()
  iterations += 1
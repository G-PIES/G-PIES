# Python 3.10.12
#
#
#
import sys
sys.path.append('../lib')
# Import sys and append the library location to the path of known python libraries
# Otherwise, we would have to have the shared libary in the same folder as the Python code.
# This can totally be changed, the problem with this implementation is that the code must be run 
# in the /example folder as path relies on relative paths. We can change this later :) 

# For now make sure to run "python3 .." from this /example folder.

# Import python cluster dynamics library shared library
import pyclusterdynamics as pycd

simulation_time = 1.0
C = 10
t = 0
step = 0.00001

# Simulation object from the wrapper code, given these launch paramters 
# C - Concentration boundary 
# simulation time - total simulation time
# 0.00001 - time step from each iteration
reactor = pycd.Sim_Reactor();
material = pycd.Sim_Material();

#print("Material Obj: " + str(material))
#print("Reactor Obj: " + str(reactor))

print(reactor.get_temperature())
reactor.set_temperature(623.15) #KELVIN
print(reactor.get_temperature())
print(material.get_i_migration())
print(reactor.get_dislocation_density_evolution())

#Default material SA304 (for now..)
#Deafult reactor OSIRIS (for now..)

#Overloaded args - Custom reactor and custom material
cd = pycd.Simulation(C, simulation_time, step, material, reactor);

#Default Constructor
#cd = pycd.Simulation(C, simulation_time, step);

# prints out the object so we know Python has a reference point in memory for the class
print("Cluster Dynamics Obj: " + str(cd))


# Main simulation loop
while t < simulation_time:
    # runs the simulation and updates the state
    cd.run()
    # gets the time at the current state
    t = cd.get_state_time()

# print out the end state
cd.print_state()
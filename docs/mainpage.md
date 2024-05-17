G-PIES Documentation Main Page                         {#mainpage}
============

# G-PIES GPU Accelerated Cluster Dynamics Simulation

Code Documentation:  
  [Classes](annotated.html)


## Obtaining Binary gpies

### Method 1: Build From Source
Clone the repository:
>git clone https://github.com/G-PIES/G-PIES.git

Build gpies:
>./build.sh gpies

If this fails, try:
>./build.sh gpies --no-sanitizer

gpies binary will be at:
>./out/gpies

### Method 2: Pre-built Binary
Download a release at: 
>https://github.com/G-PIES/G-PIES/releases

### Help Command
For more information about running the binary, run (ensure you are in the same directory as the G-PIES executable):
>./gpies --help

## Purpose

When designing a nuclear reactor, it is necessary to understand how the material will change over the decades of radiation exposure it will endure. It is rarely feasible to determine this by simply irradiating a material sample for decades, so we must model the material’s evolution with a computer. However, existing techniques have been restricted by the amount of time it takes to simulate these models. 

Unlike existing models, our project takes advantage of GPU parallelism to simulate a cluster dynamics model of radiation damage. The GPU (graphics processing unit) allows for several thousands of computations to be done at the same time, or in parallel. GPU parallelism can speed up the simulation by orders of magnitude. Our model can track concentrations of clusters of various sizes, generate clusters in the initial collision cascade, demonstrate evolution of clusters by exchange of single vacancies and interstitials, and demonstrate evolution of the dislocation network (including the effect of cluster growth on the dislocation network). Additionally, the model is augmented by modeling of the spatial dimension and spatial diffusion of defects.

## Author

Tianyi Chen (Product Partner/Advisor)

## Contributors

If you have any questions about our project, you can email one of us at the emails below:

George Connor Hutchinson
hutchgeo@oregonstate.edu

Ella Riis
riise@oregonstate.edu

Dylan Oldham
oldhamd@oregonstate.edu

Benjamin Reed
reedbe@oregonstate.edu

Austin Nakamatsu
nakamata@oregonstate.edu

Matthew Mills
millsmat@oregonstate.edu

Connor Oliver
olivecon@oregonstate.edu

Kyle Werstlein
werstlky@oregonstate.edu

Sean Siders
siderss@oregonstate.edu

Mykola Balakin
balakinm@oregonstate.edu

Keifer Snedeker
snedekek@oregonstate.edu

## Repository

https://github.com/G-PIES/G-PIES
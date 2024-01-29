# G-PIES

### GPU-Parallelized Irradiation Environment Simulator

## CD CUDA
**Usage:** `make`

Program will return a `ClusterDynamicsState` which reflects the result of the simulation.

### Variables

`interstitials`: This array at index 1 is the concentration of interstitial clusters of size one, index 2 is the concentration of interstitial clusters of size two, and so on. Zeroeth element does not hold a meaningful value and serves only to help the indices match the cluster sizes.

`vacancies`: This array at index 1 is the concentration of vacancy clusters of size one, index 2 is the concentration of vacancy clusters of size two, and so on. Zeroeth element does not hold a meaningful value and serves only to help the indices match the cluster sizes.

`delta_time`: Size of the time step between samples

`time`: Total time for the simulation to run. Returns the sum of all runs.

`valid`: Returns false if the simulation ended in an invalid state, otherwise returns true

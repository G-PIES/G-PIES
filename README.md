![G-PIES Logo](https://github.com/G-PIES/G-PIES/blob/f6de57ae159a27f9892ebe5cb9fd6fd60ae078e5/assets/logos/G-PIES%20Banner.png)

# G-PIES

### GPU-Parallelized Irradiation Environment Simulator

Our cluster dynamics simulation can utilize CUDA for GPU acceleration to
model the evolution of defects in materials that are exposed to high 
levels of neutron radiation inside of a nuclear reactor.  

Our model can be run on Linux, MacOS, or Windows. However the CUDA-enabled
version will only work on a Linux or Windows machine which has a compatible 
Nvidia graphics card.  

[User Guide / Documentation](https://g-pies.github.io/G-PIES/)  

**Doxygen**  

We've currently got Doxygen set up to serve most of our documentation.

To view our full documentation, do the following:
   1. Install [doxygen](https://www.doxygen.nl/download.html)
   2. Run either `doxygen InterfaceDoxyfile` or `doxygen BackendDoxyfile`.
      The former only documents the interface of the cluster dynamics library.
      The latter includes documentation for the backend.
   3. Open `docs/html/index.html` in a browser.

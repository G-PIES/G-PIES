# Using Our Makefile

In the present state, our Makefile is a list of named build scripts. This is different
from how `make` is normally used, since it doesn't take advantage of `make`'s dependency
graph skills, but it works.

To run a make command, go to the root directory of the project (The one with `Makefile` in it)
and execute `make [COMMAND]`, where `[COMMAND]` is replaced by one of the commands listed further
down in this document. Optionally, if you include `R=1` at the end of a make command for a test 
or executable target, the executable will run immediately after being built (`make [COMMAND] R=1`). 
The CI targets cannot be run automatically like this.

The following commands are present in our Makefile:

Library Targets
  - **cdlib** Builds the cluster dynamics library. Outputs `lib/libclusterdynamics.a`.  
  - **cdcudalib** Builds the CUDA implementation of the cluster dynamics library. Outputs `lib/libcudaclusterdynamics.a`.  
  - **dblib** Builds the database library. Outputs `lib/libclientdb.a`.  

Test Targets  
  - **cdtests** Builds the cluster dynamics test target. Outputs `bin/cd_tests.out`.  
  - **cdcudatests** Builds the cluster dynamics test target with the CUDA backend implementation.
                 Outputs `bin/cd_cuda_tests.out`.  
  - **dbtests** Builds the database test target. Outputs `bin/db_tests.out`.  

Example Executable Targets  
  - **cdex** Builds the example frontend for the cluster dynamics backend library. Outputs `bin/cd_example.out`.  
  - **cdv** Same as **cdex** but with verbose printing enabled, which will tell the example frontend to show
         each step of the simulation instead of just the last one.  
  - **cdcsv** Same as **cdex** but with csv output enabled, which will disable normal output and only print out csv rows
           of the simulation data.  
  - **cdcudaex** Similar to **cdex** except using the CUDA cluster dynamics backend library. Outputs `bin/cd_cuda_example.out`.
  - **dbex** Builds an example usage of the database library. Outputs `bin/db_example.out`.  

CI Targets
  - **cluster_dynamics** Same as **cdex**, except it outputs `bin/cluster_dynamics.out`, and the resulting executable
                      cannot be run automatically with `R=1`.  
  - **client_db** Same as **dbex**, except it outputs `bin/client_db.out`, and the resulting executable cannot be 
               run automatically with `R=1`.

If you build from Windows, the output filetypes should be automatically changed to match. For example, all of the
`.out` outputs should give you `.exe` files instead.
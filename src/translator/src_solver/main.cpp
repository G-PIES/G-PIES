#include <iostream>

#include <cvodes/cvodes.h>
#include <nvector/nvector_serial.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sunmatrix/sunmatrix_dense.h> 

#include "../generated/system.h"

void handleError(int sunerr)
{
    if (sunerr) std::cout << SUNGetErrMsg(sunerr) << "\n";
}

int main()
{
    SUNContext sun_context;
    N_Vector state;
    SUNMatrix jacobian_matrix;
    SUNLinearSolver linear_solver;
    void* cvodes_memory_block;
    size_t state_size = STATE_SIZE;

    handleError( SUNContext_Create(SUN_COMM_NULL, &sun_context) );

    state = N_VNew_Serial(state_size, sun_context);
    get_initial_state(state);
    
    cvodes_memory_block = CVodeCreate(CV_BDF, sun_context);
    handleError( CVodeInit(cvodes_memory_block, system, 0, state) );
    handleError( CVodeSStolerances(cvodes_memory_block, 1e-6, 1e-1) );
    jacobian_matrix = SUNDenseMatrix(state_size, state_size, sun_context);
    linear_solver = SUNLinSol_Dense(state, jacobian_matrix, sun_context);
    CVodeSetMaxNumSteps(cvodes_memory_block, 5000);
    CVodeSetMinStep(cvodes_memory_block, 1e-20);
    CVodeSetMaxStep(cvodes_memory_block, 1e30);
    CVodeSetInitStep(cvodes_memory_block, 1e-5);
    handleError( CVodeSetLinearSolver(cvodes_memory_block, linear_solver, jacobian_matrix) );

    double tout = 1e2;
    double sample_interval = 1e-1;

    std::cout << get_state_csv_label() << std::endl;
    for (double t = 0; t <= tout;)
    {
        int sunerr = CVode(cvodes_memory_block, t + sample_interval, state, &t, CV_NORMAL);
        if (sunerr) break;
        std::cout << t;
        std::cout << get_csv_line(state) << "\n";
        std::cout << "\n";
    }

    N_VDestroy_Serial(state);
    SUNMatDestroy(jacobian_matrix);
    SUNLinSolFree(linear_solver);
    CVodeFree(&cvodes_memory_block);
    SUNContext_Free(&sun_context);

    return 0;
}
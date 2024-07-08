#include <iostream>
#include "config.h"
#include "instance.h"
#include "solver.h"
int main(int argc, char* argv[])
{

    // Omega_CVaR_Cardi -i "C:/Users/ErisGrey/Downloads/output_1/output_1_1.csv" -p 0.97
    Config config(argc, argv);

    Instance instance(config.input, config.panel);

    Solver solver(&instance, instance.instanceName, instance.instanceName, config.time_limit, config.mem_limit);
    solver.Solve();

    return 0;
}
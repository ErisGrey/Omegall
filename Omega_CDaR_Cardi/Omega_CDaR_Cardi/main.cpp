#include <iostream>
#include "config.h"
#include "instance.h"
#include "solver.h"
int main(int argc, char* argv[])
{

    // Omega_CDaR_Cardi -i "C:/Users/maiti/Downloads/output_4/output_4_0.csv" -p 0.90
    Config config(argc, argv);

    Instance instance(config.input, config.panel);

    Solver solver(&instance, instance.instanceName, instance.instanceName, config.time_limit, config.mem_limit);
    solver.Solve();

    return 0;
}
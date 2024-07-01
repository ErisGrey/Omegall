#include "instance.h"
#include <ilcplex/ilocplex.h>
#include <unordered_map>
#include <unordered_set>

#include <chrono>
ILOSTLBEGIN

typedef IloArray<IloArray<IloNumVarArray>> NumVar3D;
typedef IloArray<IloArray<IloNumArray>> Num3D;
typedef IloArray<IloNumVarArray> NumVar2D;
typedef IloArray<IloNumArray> Num2D;

typedef unordered_map<int, int> MapSol2D;
typedef vector<unordered_map<int, int>> MapSol3D;
typedef vector<tuple<int, int>> Sol2D;
typedef vector<tuple<int, int, int>> Sol3D;


using namespace std;


class Solver {
public:
    Instance* instance;

    IloNumVarArray n;
    IloNumVarArray m;
    IloNumVarArray Q;
    IloNumVarArray x;
    IloNumVarArray h;
    IloNumVarArray u;
    IloNumVarArray d;
    IloNumVar lam;
    IloNumVar L;
    IloNumVar V;
    //IloNumVarArray v;
    IloNumVarArray k;

   
    float alpha;
    vector<float> p;
    vector<float> q; // proportional_cost
    
    vector<int> N_st;
    vector<int> N_sc;
    
    vector<vector<float>> B;

    int N; //num_stock_benchmark;
    int T; //num_scenario;
    

    IloObjective subObj;
    IloEnv masterEnv;
    IloCplex masterCplex;
    IloModel masterModel;
    double UB;
    double UB_tsp;
    int numNode;
    int numUAV;



    Solver(Instance*, string input, string output, double time_limit, double mem_limit);
    void Solve();
    bool isTruckOnly(int i);


    ~Solver();

    bool isFree(int i);
private:
    chrono::time_point<std::chrono::high_resolution_clock> startTime;
    double gap;
    double runtime;
    string status;
    string inputFile;
    string outputFile;
    double time_limit;
    double mem_limit;
    void createModel();
    void dispay_solution();
    void write_output();



};



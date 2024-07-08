#include "instance.h"
#include <ilcplex/ilocplex.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <iostream>
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

    IloNumVarArray x;
    IloNumVarArray u;
    IloNumVarArray z;
    IloNumVarArray d;
    IloNumVarArray v;
    IloNumVarArray k;

    IloNumVar V;
    IloNumVar lam;
    IloNumVar L;
    float alpha;

    vector<float> p;
    vector<float> q; // proportional_cost

    vector<int> N_st;
    vector<int> N_sc;
    vector<int> n_st;
    vector<int> exotic;
    vector<vector<float>> b;
    vector<vector<float>> B;

    int N; //num_stock_benchmark;
    int T; //num_scenario;
    int n; //num_stock_selection

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

    vector<int> sub(const vector<int>& vec1, const vector<int>& vec2) {
        vector<int> result;

        // Sắp xếp vector 2 để có thể sử dụng std::binary_search
        vector<int> sortedVec2 = vec2;
        sort(sortedVec2.begin(), sortedVec2.end());

        // Lặp qua từng phần tử của vector 1
        for (int num : vec1) {
            // Kiểm tra xem phần tử có tồn tại trong vector 2 hay không
            if (!std::binary_search(sortedVec2.begin(), sortedVec2.end(), num)) {
                result.push_back(num); // Nếu không tồn tại, thêm vào kết quả
            }
        }

        return result;
    }

};



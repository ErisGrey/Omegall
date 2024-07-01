
#ifndef INSTANCE_H
#define INSTANCE_H

#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include <unordered_map>


using namespace std;


class Instance {
public:
    string instanceName;

    int num_stock;
    int num_scenario;
    int frame;
    Instance(string inputFile, float panel);
    vector<vector<float>> realization;
    vector<vector<float>> y;
    vector<float> out_sample;
    vector<int> sto_sel;
    float alpha;
    int index;
    void read_input(const string& inputFile);
    void initialize();
    template <class Container>
    void split(const std::string& str, Container& cont, char delim);
};

#endif


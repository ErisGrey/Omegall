﻿#include "solver.h"
#include <algorithm>
#include "instance.h"
#include <tuple>

ILOSTLBEGIN
#define pos(u,v) min(u,v)][max(u,v)


Solver::Solver(Instance* instance, string input, string output, double time_limit, double mem_limit)
	:instance(instance), inputFile(input), outputFile(output), time_limit(time_limit), mem_limit(mem_limit) {
	cerr << "-- Solving \n";
	startTime = chrono::high_resolution_clock::now();
	outputFile = instance->instanceName;
	gap = 1e5;
	status = "-";

	/*  SET -------------------------------- */
	//n = 50;
	N = instance->num_stock;
	T = instance->num_scenario;
	n = instance->sto_sel.size();

	/*n_st = instance->sto_sel;

	for (auto i : n_st)
	{
		cout << i << " ";
	}
	cout << endl;*/
	for (int i = 0; i < N; i++)
	{
		N_st.push_back(i);
	}

	for (int i = 0; i < T; i++)
	{
		N_sc.push_back(i);
	}

	exotic = sub(N_st, n_st);
	for (int i : exotic)
	{
		cout << i << " ";
	}
	cout << endl;

	p.resize(T);
	q.resize(T);
	for (int i = 0; i < T; i++)
	{
		p[i] = 1.0 / T;
		q[i] = 1.0 / T;
	}

	B = instance->realization;
	b.resize(N);
	for (int i : n_st)
	{
		b[i].resize(T);
		for (int j = 0; j < T; j++)
		{
			b[i][j] = B[i][j];
		}
	}
	/*for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < T; j++)
		{
			cout << b[i][j] << " ";
		}
		cout << endl;
	}*/
	alpha = instance->alpha;
	cout << "alpha = " << alpha << endl;
}

Solver::~Solver() {
	//    cerr << "Runtime = " << (double)(clock() - startTime) / CLOCKS_PER_SEC << "s\n";
}

void Solver::Solve() {
	try {
		createModel();

		masterCplex.exportModel("lpex.lp");
		masterCplex.setParam(IloCplex::Param::Parallel, 1);
		masterCplex.setParam(IloCplex::Param::MIP::Tolerances::Integrality, 0);
		masterCplex.setParam(IloCplex::Param::Threads, 16);
		masterCplex.setParam(IloCplex::TiLim, time_limit);
		masterCplex.setParam(IloCplex::TreLim, mem_limit);
		masterCplex.setParam(IloCplex::Param::MIP::Strategy::RINSHeur, 10);

		masterCplex.solve();
		if (masterCplex.getStatus() == IloAlgorithm::Infeasible) {
			cout << UB << endl;
			cout << "Infeasible" << endl;
		}
		else if (masterCplex.getStatus() == IloAlgorithm::Unbounded) {
			cout << "Unbounded" << endl;
		}
		else if (masterCplex.getStatus() == IloAlgorithm::Unknown) {
			cout << "Unknown" << endl;
		}
		else {
			cout << "DONE ..." << endl;
			cout << masterCplex.getObjValue() << endl;
			dispay_solution();
		}


	}
	catch (IloException& e) {
		cerr << "Conver exception caught: " << e << endl;
	}
	catch (...) {
		cerr << "Unknown exception caught" << endl;
	}


	auto endTime = chrono::high_resolution_clock::now();
	runtime = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
	runtime = runtime / 1000;

	write_output();
	cout << "Environment cleared \n";
	//        workerCplex.end();
	masterCplex.end();

	//        workerEnv.end();
	masterEnv.end();
}

void Solver::createModel() {

	masterModel = IloModel(masterEnv);
	masterCplex = IloCplex(masterEnv);



	x = IloNumVarArray(masterEnv, N); // x_i
	k = IloNumVarArray(masterEnv, N); //k_i
	u = IloNumVarArray(masterEnv, T); //u_t
	d = IloNumVarArray(masterEnv, T); //d_t
	z = IloNumVarArray(masterEnv, T); //z_t
	v = IloNumVarArray(masterEnv, T); //v_t
	V = IloNumVar(masterEnv); //V
	L = IloNumVar(masterEnv); //L
	lam = IloNumVar(masterEnv); //lam


	stringstream name;
	// x_i
	for (int i : N_st)
	{
		name << "x." << i;
		x[i] = IloNumVar(masterEnv, 0, IloInfinity, ILOFLOAT, name.str().c_str());
		name.str("");
	}

	// k_i
	for (int i : N_st)
	{
		name << "k." << i;
		k[i] = IloNumVar(masterEnv, 0, 1, ILOINT, name.str().c_str());
		name.str("");
	}

	//u_t
	for (int i : N_sc)
	{
		name << "u." << i;
		u[i] = IloNumVar(masterEnv, 0, IloInfinity, ILOFLOAT, name.str().c_str());
		name.str("");
	}

	//d_t
	for (int i : N_sc)
	{
		name << "d." << i;
		d[i] = IloNumVar(masterEnv, 0, IloInfinity, ILOFLOAT, name.str().c_str());
		name.str("");
	}

	//z_t
	for (int i : N_sc)
	{
		name << "z." << i;
		z[i] = IloNumVar(masterEnv, 0, 1, ILOINT, name.str().c_str());
		name.str("");
	}


	//v_t
	for (int i : N_sc)
	{
		name << "v." << i;
		v[i] = IloNumVar(masterEnv, 0, IloInfinity, ILOFLOAT, name.str().c_str());
		name.str("");
	}

	//V
	name << "V";
	V = IloNumVar(masterEnv, -IloInfinity, IloInfinity, ILOFLOAT, name.str().c_str());
	name.str("");

	//L
	name << "L";
	L = IloNumVar(masterEnv, -IloInfinity, IloInfinity, ILOFLOAT, name.str().c_str());
	name.str("");

	//lam
	name << "lam";
	lam = IloNumVar(masterEnv, 0, IloInfinity, ILOFLOAT, name.str().c_str());
	name.str("");

	int BIGM = 9999;

	// OBJ FUNCTION
	IloExpr exprSolution(masterEnv);

	for (int t : N_sc)
	{
		exprSolution += p[t] * u[t];
		//exprSolution += p * d[t];
	}

	masterModel.add(IloMaximize(masterEnv, exprSolution));
	//CONSTRAINT

	int inf = 0.005;

	for (int i : N_sc)
	{
		IloExpr exp(masterEnv);
		for (int j : N_st)
		{
			exp += B[j][i] * x[j];
		}

		masterModel.add(exp + u[i] - d[i] == L);
	}


	IloExpr exp1(masterEnv);
	for (int i : N_sc)
	{
		exp1 += p[i] * d[i];
	}
	masterModel.add(exp1 == 1);


	masterModel.add(L <= V);

	for (int i : N_sc)
	{

		masterModel.add(v[i] - lam * 1.0 / (1 - alpha) * q[i] <= 0);
	}

	for (int i : N_st)
	{
		IloExpr exp2(masterEnv);
		for (int j : N_sc)
		{
			exp2 += -B[i][j] * v[j];
		}
		masterModel.add(exp2 + V <= 0);
	}


	IloExpr exp3(masterEnv);
	for (int i : N_st)
	{
		exp3 += x[i];
	}
	masterModel.add(exp3 == lam);


	IloExpr exp4(masterEnv);
	for (int i : N_sc)
	{
		exp4 += v[i];
	}
	masterModel.add(exp4 == lam);

	for (int i : N_sc)
	{
		masterModel.add(u[i] <= BIGM * z[i]);
		masterModel.add(d[i] <= BIGM * (1 - z[i]));
	}


	for (int i : N_st)
	{
		masterModel.add(x[i] <= BIGM * k[i]);
		masterModel.add(0.01 * lam - (1 - k[i]) * BIGM <= x[i]);
	}

	IloExpr expc(masterEnv);
	for (int i : N_st)
	{
		expc += k[i];
	}

	if (instance->frame == 1)
		masterModel.add(expc == 40);
	if (instance->frame == 3)
		masterModel.add(expc == 70);
	if (instance->frame == 4)
		masterModel.add(expc == 10);

	masterCplex.extract(masterModel); // <<<< IMPORTANT
	cout << "Done create init MasterProblem\n";
}

void Solver::dispay_solution()
{
	cout << "L_alpha = " << masterCplex.getValue(L) << endl;
	cout << "V = " << masterCplex.getValue(V) << endl;
	cout << "Lamda = " << masterCplex.getValue(lam) << endl;

	for (int i : N_st)
	{
		if (masterCplex.getValue(x[i]) >= 0.00001)
			cout << "x[" << i << "] = " << masterCplex.getValue(x[i]) / masterCplex.getValue(lam) << endl;
	}

	cout << "k_4 = " << masterCplex.getValue(k[4]) << endl;

	/*for (int i : N_st)
	{
		if (masterCplex.getValue(x[i]) >= 0.00001)
			cout << "k[" << i << "] = " << masterCplex.getValue(x[i]) / masterCplex.getValue(lam) << endl;
	}*/

	/*for (int i : N_sc)
	{
		cout << "v[" << i << "] = " << masterCplex.getValue(v[i]) << endl;
	}*/

	/*for (int i : N_sc)
	{
		float sum = 0;
		for (int j : n_st)
		{
			sum += b[j][i] * masterCplex.getValue(x[j]) * 1.0 / masterCplex.getValue(lam);
		}
		cout << "u[" << i << "] = " << masterCplex.getValue(u[i]) * 1.0 / masterCplex.getValue(lam) << "; ";
		cout << "d[" << i << "] = " << masterCplex.getValue(d[i]) * 1.0 / masterCplex.getValue(lam) << "; ";
		cout << "L = " << masterCplex.getValue(L) * 1.0 / masterCplex.getValue(lam) - sum << endl;
	}*/

	float sum = 0;
	for (int i : N_st)
	{
		sum += masterCplex.getValue(x[i]) * 1.0 * instance->out_sample[i] / masterCplex.getValue(lam);
	}
	sum = sum * -1;
	cout << sum << endl;
}

void Solver::write_output()
{
	float sum = 0;
	for (int i : N_st)
	{
		sum += masterCplex.getValue(x[i]) * 1.0 * instance->out_sample[i] / masterCplex.getValue(lam);
	}
	sum = sum * -1;
	cout << sum << endl;

	vector<float> x_result;
	for (int i : N_st)
	{
		if (masterCplex.getValue(x[i]) >= 0.0000001)
			x_result.push_back(masterCplex.getValue(x[i]) * 1.0 / masterCplex.getValue(lam));
	}
	cout << "Size: " << x_result.size() << endl;
	cout << endl;
	float min_weight = *min_element(x_result.begin(), x_result.end());
	float max_weight = *max_element(x_result.begin(), x_result.end());
	std::ofstream ocsv;
	string output_name;
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << alpha;
	string dir = "C:/OCVaRC/";
	if (instance->frame == 1)
		output_name = dir + "OCVaRC_s&p_" + ss.str() + ".csv";
	if (instance->frame == 3)
		output_name = dir + "OCVaRC_russel_" + ss.str() + ".csv";
	if (instance->frame == 4)
		output_name = dir + "OCVaRC_nikkei_" + ss.str() + ".csv";
	cout << min_weight << endl;
	cout << max_weight << endl;
	cout << masterCplex.getObjValue() << endl;
	//cout << output_name << endl;
	ocsv.open(output_name, std::ios_base::app);
	if (instance->index == 0)
	{
		ocsv << "Window" << "," << "Return" << "," << "Omega" << "," << "Min weight" << "," << "Max weight" << "," << "L" << ", " << "Time" << ", " << "Gap" << endl;
	}
	ocsv << "window " << instance->index << ","
		<< sum << "," << masterCplex.getObjValue() << "," << min_weight << "," << max_weight << "," << masterCplex.getValue(L) / masterCplex.getValue(lam) << "," << runtime << "," << masterCplex.getMIPRelativeGap() * 100 << endl
		<< std::flush;
	ocsv.close();
}
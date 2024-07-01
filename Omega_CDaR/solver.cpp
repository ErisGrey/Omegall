#include "solver.h"
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
	/*for (int i = 0; i < n; i++)
	{
		n_st.push_back(i);
	}*/

	for (int i = 0; i < N; i++)
	{
		N_st.push_back(i);
	}

	for (int i = 0; i < T; i++)
	{
		N_sc.push_back(i);
	}


	B = instance->realization;
	for (int i : N_st)
	{
		for (int j : N_sc)
		{
			B[i][j] = B[i][j] * -1;
		}
	}
	p.resize(T);
	q.resize(T);
	for (int i = 0; i < T; i++)
	{
		p[i] = 1.0 / T;
		q[i] = 1.0 / T;
	}
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
	
	n = IloNumVarArray(masterEnv, T);//n_t
	Q = IloNumVarArray(masterEnv, T);//Q_k
	m = IloNumVarArray(masterEnv, T);//m_t

	x = IloNumVarArray(masterEnv, N); // x_i
	h = IloNumVarArray(masterEnv, N); // h_i
	u = IloNumVarArray(masterEnv, T); //u_t
	d = IloNumVarArray(masterEnv, T); //d_t
	L = IloNumVar(masterEnv); //L
	lam = IloNumVar(masterEnv); //lam
	V = IloNumVar(masterEnv); //V
	k = IloNumVarArray(masterEnv, N); //k_i



	stringstream name;

	// x_i
	for (int i : N_st)
	{
		name << "x." << i;
		x[i] = IloNumVar(masterEnv, 0, IloInfinity, ILOFLOAT, name.str().c_str());
		name.str("");
	}

	// h_i
	for (int i : N_st)
	{
		name << "h." << i;
		h[i] = IloNumVar(masterEnv, 0, 1, ILOINT, name.str().c_str());
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


	//Q_t
	for (int i : N_sc)
	{
		name << "Q." << i;
		Q[i] = IloNumVar(masterEnv, 0, IloInfinity, ILOFLOAT, name.str().c_str());
		name.str("");
	}

	//n_t
	for (int i : N_sc)
	{
		name << "n." << i;
		n[i] = IloNumVar(masterEnv, 0, IloInfinity, ILOFLOAT, name.str().c_str());
		name.str("");
	}

	//m_t
	for (int i : N_sc)
	{
		name << "m." << i;
		m[i] = IloNumVar(masterEnv, 0, IloInfinity, ILOFLOAT, name.str().c_str());
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
	}

	/*IloExpr exprSolution(masterEnv);

	exprSolution += V;*/
	masterModel.add(IloMaximize(masterEnv, exprSolution));
	//CONSTRAINT

	for (int i : N_sc)
	{
		IloExpr exp(masterEnv);
		for (int j : N_st)
		{
			exp += B[j][i] * x[j];
		}

		masterModel.add(exp + u[i] - d[i] == L);
	}
	
	IloExpr expd(masterEnv);
	for (int i : N_sc)
	{
		expd += p[i] * d[i];
	}
	masterModel.add(expd == 1) ;

	
	IloExpr expx(masterEnv);
	for (int i : N_st)
	{
		expx += x[i];
	}
	masterModel.add(expx == lam);

	masterModel.add(L <= V);

	IloExpr expq(masterEnv);
	for (int i : N_sc)
	{
		expq += Q[i];
	}
	masterModel.add(expq == 1);


	for (int k : N_sc)
	{
		masterModel.add(Q[k] <= 1 * 1.0 / ((1 - alpha) * T));
		if (k < T - 1)
			masterModel.add(-Q[k] + n[k] + m[k] - m[k + 1] <= 0);
		else
			masterModel.add(-Q[k] + n[k] + m[k] <= 0);

	}
	
	for (int i : N_st)
	{
		IloExpr exp(masterEnv);	
		for (int k : N_sc) 
		{
			exp += instance->y[i][k] * Q[k] - instance->y[i][k] * n[k];
		}
		exp += V;
		masterModel.add(exp <= 0);
	}

	/*for (int i : N_st)
	{
		masterModel.add(x[i] <= BIGM * h[i]);
		masterModel.add(0.01 * lam - (1 - h[i]) * BIGM <= x[i]);
	}
	IloExpr expc(masterEnv);
	for (int i : N_st)
	{
		expc += h[i];
	}
	masterModel.add(expc == 70);*/


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
		//if (masterCplex.getValue(x[i]) >= 0.00001)
			cout << "x[" << i << "] = " << masterCplex.getValue(x[i]) / masterCplex.getValue(lam) << endl;
	}

	//cout <<"k_4 = " <<  masterCplex.getValue(k[4]) << endl;

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

	/*float sum = 0;
	for (int i : N_st)
	{
		sum += masterCplex.getValue(x[i]) * 1.0 * instance->out_sample[i] / masterCplex.getValue(lam);
	}
	sum = sum * -1;
	cout << sum << endl;*/
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
		if (masterCplex.getValue(x[i]) * 1.0 / masterCplex.getValue(lam) >= 0.001)
		{
			cout << masterCplex.getValue(x[i]) * 1.0 / masterCplex.getValue(lam) << endl;
			x_result.push_back(masterCplex.getValue(x[i]) * 1.0 / masterCplex.getValue(lam));
		}

	}
	cout << endl;
	cout << "Size: " << x_result.size() << endl;
	cout << endl;
	float min_weight = *min_element(x_result.begin(), x_result.end());
	float max_weight = *max_element(x_result.begin(), x_result.end());
	std::ofstream ocsv;
	string output_name;
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << alpha;
	string dir = "C:/Omega_CDaR_output_1/";
	output_name = dir + "Output_" + to_string(instance->frame) + "_" + ss.str() + ".csv";
	cout << min_weight << endl;
	cout << max_weight << endl;
	cout << masterCplex.getObjValue() << endl;
	//cout << output_name << endl;
	ocsv.open(output_name, std::ios_base::app);
	if (instance->index == 0)
	{
		ocsv << "Window" << "," << "Return" << "," << "Omega" << "," << "Min weight" << "," << "Max weight" << "," << "Time" << "," << "Gap" << endl;
	}
	ocsv << "window " << instance->index << ","
		<< sum << "," << masterCplex.getObjValue() << "," << min_weight << "," << max_weight << "," <<  runtime << "," << masterCplex.getMIPRelativeGap() * 100 << endl
		<< std::flush;
	ocsv.close();
}
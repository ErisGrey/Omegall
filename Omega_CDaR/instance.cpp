#include "instance.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <iostream>
#include <math.h>
Instance::Instance(string instanceFile, float panel)
{
	vector<string> numbers;
	split(instanceFile, numbers, '/');
	instanceName = numbers[numbers.size() - 1];
	cout << instanceName << endl;
	read_input(instanceFile);
	alpha = panel;
	initialize();
}


void Instance::read_input(const string& inputFile)
{
	ifstream myFile(inputFile);
	if (!myFile.is_open())
	{
		// End program immediately
		cout << "Unable to open instance file \n";
		exit(0);
	}

	string line;
	vector<string> numbers;

	getline(myFile, line);
	cout << line << endl;
	split(line, numbers, ' ');
	num_stock = stod(numbers[1]); //fixed tam thoi
	cout << num_stock << endl;


	num_scenario = stod(numbers[0]); // fixed tam thoi
	cout << num_scenario << endl;

	//getline(myFile, line);
	////cout << line << endl;
	//split(line, numbers, ',');
	//sto_sel.resize(numbers.size());
	//cout << numbers.size() << endl;
	//for (int i = 0; i < numbers.size(); i++)
	//{
	//	sto_sel[i] = stod(numbers[i]);
	//}
	//
	//for (int i = 0; i < sto_sel.size(); i++)
	//{
	//	cout << sto_sel[i] << " ";
	//}
	//cout << endl;



	realization.resize(num_stock);
	for (int i = 0; i < num_stock; i++)
	{
		realization[i].resize(num_scenario, 0);
	}

	for (int i = 0; i < num_scenario; i++)
	{
		getline(myFile, line);
		//cout << line << endl;
		split(line, numbers, ',');
		//cout << numbers.size() << endl;
		for (int j = 0; j < numbers.size(); j++)
		{
			//cout << stod(numbers[j]) << endl;
			//realization[j][i] = ceil(stod(numbers[j]) * 100.0)/100.0;
			realization[j][i] = stod(numbers[j]) * (- 1); //return, not loss
		}
	}

	y.resize(num_stock);
	for (int i = 0; i < num_stock; i++)
	{
		y[i].resize(num_scenario, 0);
	}

	for (int i = 0; i < num_stock; i++)
	{
		for (int t = 0; t < num_scenario; t++)
		{
			for (int k = 0; k <= t; k++)
			{
				y[i][t] += realization[i][k];
			}
		}
	}

	//cout << "done1" << endl;
	out_sample.resize(num_stock, 0);
	getline(myFile, line);
	split(line, numbers, ',');
	for (int i = 0; i < num_stock; i++)
	{

		out_sample[i] = stod(numbers[i]);

		//cout << "out_sample[" << i << "] = " << out_sample[i] << endl;
		//cout << out_sample[i] << " ";
	}
	cout << endl;

	cout << "done read instance" << endl;
	
	///*for (int i = 0; i < num_stock; i++)
	//{*/
	//	for (int j = 0; j < num_scenario; j++)
	//	{
	//		//cout << realization[0][j] << " ";
	//	}
	//	cout << endl;

	////}

	myFile.close();
}

void Instance::initialize()
{
	//get the No of window
	vector<string> numbers;
	cout << instanceName << endl;
	split(instanceName, numbers, '_');
	for (auto nb : numbers)
	{
		cout << nb << endl;
	}
	//cout << numbers[numbers.size()-1] << endl;
	frame = stod(numbers[numbers.size() - 2]);
	index = stod(numbers[numbers.size() - 1]);
	cout << frame << " " << index << endl;
}

template<class Container>
void Instance::split(const std::string& str, Container& cont, char delim)
{
	cont.clear();
	std::stringstream ss(str);
	std::string token;
	while (std::getline(ss, token, delim)) {
		if (token != "")
			cont.push_back(token);
	}
}
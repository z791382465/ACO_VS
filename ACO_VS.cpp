#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <math.h>
#include <string>
#include <limits>

using namespace std;

typedef vector<int> i1d;
typedef vector<i1d> i2d;
typedef vector<double> d1d;
typedef vector<d1d> d2d;

char file_name[255];
int max_time;
int ant_number;
double q0;
double beta;
double rho;

int city_number;
d2d city_position;
i2d ant_path;
d2d city_distance;
d2d eta;
d2d tau;
double tau0;
int step;
i1d optimal_path;
double optimal_distance = numeric_limits<double>::max();
clock_t p_begin;
clock_t p_end;

void run();
void load_file();
void initialize();
void tau0_initialize();
void move();
int	 select(int);
int	 roulette(int);
void local_update(int);
void global_update();
void result();
void txt();

void my_handler(int)
{
	result();
	p_end = clock();
	double elapsed_secs = double(p_end - p_begin) / CLOCKS_PER_SEC;
	cout << elapsed_secs << " sec." << endl;
	txt();
}

int main(int argc, char** argv)
{
	signal(SIGINT, my_handler);

	srand(time(NULL));

	max_time	= atoi(argv[1]);
	ant_number	= atoi(argv[2]);
	q0			= atof(argv[3]);
	beta		= atof(argv[4]);
	rho			= atof(argv[5]);
	strcpy(file_name, argv[6]);

	p_begin = clock();
	run();
	p_end = clock();
	double elapsed_secs = double(p_end - p_begin) / CLOCKS_PER_SEC;
	cout << elapsed_secs << " sec." << endl;
	cout << "done." << endl;
	txt();

	return 0;
}

void run()
{
	load_file();
	initialize();
	for (int i = 0; i <= max_time; i++)
		move();
	result();
}

void load_file()
{
	ifstream fin1(file_name), fin2(file_name);
	string line;
	city_number = 0;
	int delimiter1, delimiter2;

	while (getline(fin1, line)) city_number++;
	city_position.assign(city_number, d1d(2, 0.0));
	city_number = 0;
	while (getline(fin2, line))
	{
		delimiter1 = line.find(" ", 0);
		delimiter2 = line.find(" ", delimiter1 + 1);
		city_position[city_number][0] = atof((line.substr(delimiter1, delimiter2)).c_str());
		city_position[city_number][1] = atof((line.substr(delimiter2, line.length())).c_str());
		city_number++;
	}
}

void initialize()
{
	city_distance.assign(city_number, d1d(city_number, 0.0));
	eta.assign(city_number, d1d(city_number, 0.0));
	for (int i = 0; i<city_number; i++)
	{
		for (int j = 0; j<city_number; j++)
		{
			city_distance[i][j] =
				sqrt(
					pow(city_position[i][0] - city_position[j][0], 2) +
					pow(city_position[i][1] - city_position[j][1], 2)
				);
			eta[i][j] = 1 / city_distance[i][j];
		}
	}
	tau0_initialize();
	tau.assign(city_number, d1d(city_number, tau0));
	optimal_path.assign(city_number, -1);
}

void tau0_initialize()
{
	double min_distance;
	double total_distance = 0;
	vector<int> greedy_path(city_number, -1);
	greedy_path[0] = city_number * rand() / (RAND_MAX + 1.0);

	for (int i = 0; i<city_number - 1; i++)
	{
		min_distance = numeric_limits<double>::max();
		for (int j = 0; j<city_number; j++)
		{
			bool repeat = false;
			for (int k = 0; k<city_number; k++)
			{
				if (j == greedy_path[k])
				{
					repeat = true;
					break;
				}
			}
			if (repeat == false && i != j && city_distance[greedy_path[i]][j] < min_distance)
			{
				min_distance = city_distance[greedy_path[i]][j];
				greedy_path[i + 1] = j;
			}
		}
		total_distance += min_distance;
	}
	total_distance += city_distance[greedy_path[city_number - 1]][greedy_path[0]];
	tau0 = 1 / (city_number*total_distance);
}

void move()
{
	step = 1;
	ant_path.assign(ant_number, i1d(city_number, -1));
	for (int i = 0; i < ant_number; i++)
		ant_path[i][0] = rand() / (RAND_MAX + 1.0) * city_number;

	for (int i = 0; i < city_number - 1; i++)
	{
		for (int j = 0; j<ant_number; j++)
		{
			if ((double)rand() / RAND_MAX <= q0)
				ant_path[j][step] = select(j);
			else
				ant_path[j][step] = roulette(j);
			local_update(j);
		}
		step++;
	}
	global_update();
}

int select(int ant)
{
	int position = ant_path[ant][step - 1];
	double factor = 0.0;
	int best_destination = -1;

	for (int i = 0; i<city_number; i++)
	{
		bool repeat = false;
		for (int j = 0; j<city_number; j++)
		{
			if (i == ant_path[ant][j])
			{
				repeat = true;
				break;
			}
		}
		if (repeat == false && factor < tau[position][i] * pow(eta[position][i], beta))
		{
			factor = tau[position][i] * pow(eta[position][i], beta);
			best_destination = i;
		}
	}
	return best_destination;
}

int roulette(int ant)
{
	int position = ant_path[ant][step - 1];
	vector<double> factor(city_number, 0.0);
	double sigma = 0.0;
	vector<double> probability(city_number, 0.0);
	double ball;
	int best_destination = -1;

	for (int i = 0; i<city_number; i++)
	{
		bool repeat = false;
		for (int j = 0; j<step; j++)
		{
			if (i == ant_path[ant][j])
			{
				repeat = true;
				break;
			}
		}
		if (repeat == false)
			factor[i] = tau[position][i] * pow(eta[position][i], beta);
		else
			factor[i] = 0.0;
		sigma += factor[i];
	}
	for (int i = 0; i<city_number; i++)
		probability[i] = factor[i] / sigma;
	ball = (double)rand() / (RAND_MAX + 1.0);
	for (int i = 0; i<city_number; i++)
	{
		ball -= probability[i];
		if (ball <= 0.0 && probability[i] != 0.0)
		{
			best_destination = i;
			break;
		}
	}
	return best_destination;
}

void local_update(int ant)
{
	int i = ant_path[ant][step - 1];
	int j = ant_path[ant][step];
	tau[i][j] = (1 - rho)*(tau[i][j]) + rho*tau0;
	tau[j][i] = tau[i][j];
}

void global_update()
{
	double best_distance;
	int best_ant;
	double distance_temp;
	best_distance = numeric_limits<double>::max();
	best_ant = -1;

	for (int i = 0; i < ant_number; i++)
	{
		distance_temp = 0;
		for (int j = 0; j < city_number - 1; j++)
			distance_temp += city_distance[ant_path[i][j]][ant_path[i][j + 1]];
		distance_temp += city_distance[ant_path[i][city_number - 1]][ant_path[i][0]];
		if (best_distance > distance_temp)
		{
			best_distance = distance_temp;
			best_ant = i;
		}
	}
	for (int i = 0; i < city_number - 1; i++)
	{
		int j = ant_path[best_ant][i];
		int k = ant_path[best_ant][i + 1];
		tau[j][k] = (1 - rho)*tau[j][k] + rho * 100 / best_distance;
	}
	if (optimal_distance>best_distance)
	{
		optimal_distance = best_distance;
		for (int i = 0; i < city_number; i++)
			optimal_path[i] = ant_path[best_ant][i];
	}
}

void result()
{
	cout << "Optimal path : " << endl;
	for (int i = 0; i < city_number; i++)
		cout << optimal_path[i] << " ";
	cout << endl;
	cout << "Optimal distance : " << optimal_distance << endl;
}

void txt()
{
	char fn[]="tour.txt";
	fstream fp;
	fp.open(fn, ios::out);
	for (int i = 0; i < city_number; i++)
		fp << optimal_path[i]+1 << endl;
	fp << optimal_path[0]+1 << endl;
}

# include <iostream>
# include <fstream>
# include <memory.h>
# include <cmath>
# include <cstring>
# include <ctime>
# include "Timer.h"
using namespace std;

// This is an option for the initial value in Leading_Vector()
//# define RANDOM_V0

// set some parameters
const int MAX_ITER = 1000;			// The maximum iteration times in the power method
const double BETA_Adjust = 0;		// An optional parameter for quicker convergence. Its effect is uncertain
const double Epsilon = 0.0001;		// If |x - x0| < Epsilon, quit iteraion 
const double LAMBDA = 0.001;		// if labmda > LAMBDA, initiate the division

int N;								// The number of voxels
double * v, * v0, * verr, * sumBG;	// some buffers used in the iteration
ofstream fout;						// logfile

// Forward Declaration
void Partition(int * R, int * C, int * Result);
bool Sub_Partition(int * R, int * C, int M, bool * G, int * Result, int * Max_Result);
double Lead_Vector(int * R, int * C, int M, double * sumBG, bool * G, double beta);
template <class Type> double VectorNorm(Type * x, bool * G, int N);


int main(int argc, char * argv[])
{
	if (argc != 2) 
	{
		cerr<<"Input format: Network name. For example: ./Modularity_CP.exe X.csr "<<endl;
		exit(1);	
	}

	Setup(0);
	Start(0);
	int i = 0, j = 0, k = 0, l =0;

	// Read x.csr
	ifstream fin(argv[1], ios::binary);
	if (!fin.good())
	{	cout<<"Can't open\t"<<argv[1]<<endl;	return 0;}
	int Rlength = 0, Clength = 0;
	fin.read((char*)&Rlength, sizeof(int));
	int * R = new int [Rlength];
	fin.read((char*)R, sizeof(int) * Rlength);
	fin.read((char*)&Clength, sizeof(int));
	int * C = new int [Clength];
	fin.read((char*)C, sizeof(int) * Clength);
	fin.close();
	N = Rlength - 1;

	// allocate buffers used in the iteration
	v = new double [N];
	v0 = new double [N];
	verr = new double [N];
	sumBG = new double [N];

	// allocate the result buffer and call Partition()
	int * Result = new int [N];	
	Partition(R, C, Result);	

	// Write result to x.modu file
//	fout.open(??, ios::binary|ios::out);
//	fout.write((char *)&N, sizeof(int));
//	fout.write((char *) Result, sizeof(int) * N); 
//	fout.close();
	
	// Clean up
	delete []R;
	delete []C;
	delete []Result;
	delete []sumBG;
	delete []verr;
	delete []v0;
	delete []v;
	return 0;
}


/* 
This funtion does the partition, no return value.
R and C represent the adjacency matrix in CSR format.
Result stores the partition results.
*/
void Partition(int * R, int * C, int * Result)
{
	int M = (R[N] - R[0]) / 2;			// The total number connection in the network
	int Round = 0;						// The iteration round
	int Max_Result = 0;					// Maximum index of modules
	int Module_Num = 1;					// Used for adjust module index.
	bool Issub;							// Return by function Sub_Partition()
	
	bool * G = new bool [N];			// G[i] = 1 if node i is involved in this round of partition
	memset(Result, 0, sizeof(int) * N);
	int * Adjust_Result = new int [N];	// Map the results to consecutive intergers starting from 1 
	int Index = 1;						// Used in the adjusted results, starting from 1, increase by 1 at each successful division
	int NumG = 0;						//

	int i = 0, j = 0;
	fout.open("Module.log", ios::out);	// Open the log file 

	while (Round <= Max_Result)
	{
		NumG = 0;
		for (i = 0; i < N; i++)
		{
			G[i] = (Result[i] == Round);
			NumG += G[i];				// Set buffer G. G[i] = 1 if node i is involved in this round
		}
		if (NumG)						 
		{
			cout<<"\nRound:\t"<<Round<<'\t';
			cout<<"number of nodes:\t"<<NumG<<'\t';
			fout<<"\nRound:\t"<<Round<<'\t';
			fout.flush();
			fout<<"number of nodes:\t"<<NumG<<'\t';	
			Issub = Sub_Partition(R, C, M, G, Result, &Max_Result);
										// call Sub_Partition() for this round of division
			if (!Issub)					// If divided, record the adjusted result
				Adjust_Result[Round] = Index++;
			Module_Num += Issub;		// Updata the total number modules 
		}
		Round++;
	}
	Stop(0);

	// calculate Q
	double Q = 0;
	for (i = 0; i < N; i++)
		for (j = R[i]; j < R[i+1]; j++)
			Q += (Result[i] == Result[C[j]]);
	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++)
			Q -= (Result[i] == Result[j]) * (R[i+1]-R[i]) * (R[j+1]-R[j]) / 2 / M;

	Q /= 2 * M;

	cout<<"\n\nNumber of Modules: "<<Module_Num<<endl;
	fout<<"\n\nNumber of Modules: "<<Module_Num<<endl;
	cout<<"Q="<<Q<<endl;
	fout<<"Q="<<Q<<endl;
	cout<<"\nElapsed time:   "<<GetElapsedTime(0)<<"s"<<endl;
	fout<<"\nElapsed time:   "<<GetElapsedTime(0)<<"s"<<endl;
	fout.close();

	// Adjust the results
	for (i = 0; i < N; i++)	
		Result[i] = Adjust_Result[Result[i]];
	delete []Adjust_Result;
	delete []G;
	return;
}


/* 
This funtion does the division of each round.
It returns a bool variable, indicating whether this round of partition is successful.
R and C represent the adjacency matrix in CSR format.
M is the total connection of the network..=
G indicates whether a node is involved in this round.
Result stores the partition results.
Max_Result is updated for outer function to decide whether tto terminate partition.
*/
bool Sub_Partition(int * R, int * C, int M, bool * G, int * Result, int * Max_Result)
{
	int i = 0, j = 0;
	int temp1, temp2;
	// Initialize sumBG for this round
	for (i = 0; i < N; i++)
	{
		if (!G[i])
			continue;
		sumBG[i] = 0;
		temp1 = temp2 = 0;
		for (j = R[i]; j < R[i+1]; j++)
			temp1 += G[C[j]];
		for (j = 0; j < N; j++)
			temp2 += (R[j+1]-R[j]) * G[j];
		sumBG[i] = temp1 - (R[i+1] - R[i]) * temp2 / 2 / M;
	}

	// Call Lead_Vector() to calucate the most positive eigenvalue lambda
	double lambda = 0;
	lambda = Lead_Vector(R, C, M, sumBG, G, BETA_Adjust);
	lambda -= BETA_Adjust;
	
	// If lambda < 0, calucate the leading eigenvalue for  B - lambda * I
	if (lambda < 0)
		lambda += Lead_Vector(R, C, M, sumBG, G, -lambda);

	cout<<"Eigen Value: "<<lambda<<'\t';
	fout<<"Eigen Value: "<<lambda<<'\t';

	// Decide whether this round of partition is successful 
	int subN = 0, subP = 0;
	for (i = 0; i < N; i++)
		subP += (G[i] && v[i] > 0);
	for (i = 0; i < N; i++)
		subN += (G[i] && v[i] <= 0);
	bool Issub = (lambda > LAMBDA && subP && subN);

	cout<<"Divide?: "<<Issub<<'\t';
	fout<<"Divide?: "<<Issub<<'\t';
	// If not divided, return; otherwise updata Result and Max_Result
	if (!Issub)
		return 0;
	for (i = 0; i < N; i++)
		if (G[i])
			Result[i] = *Max_Result + 1 + (v[i]*(subP - subN) >= 0);
	(*Max_Result) += 2;
	return Issub;
}


/* 
This funtion calculates the most positive eigenvalue for matrix B - beta * I.
It returns the most positive value.

R and C represent the adjacency matrix in CSR format.
M is the total connection of the network.
G indicates whether a node is involved in this round.
For definition of sumBG1, see Sub-Partition() or [1].
For definition of beta, see the first line of this comment.

Global parameters v, v0, verr are also used in this function.
v and v0 are vectors for iteration. verr is their difference. 
*/
double Lead_Vector(int * R, int * C, int M, double * sumBG1, bool * G, double beta)
{
	int i = 0, j = 0;
	// Initialize v. Two methods are optional. Define RANDOM_V0 if you want to use random starting vector
#ifdef RANDOM_V0
	srand(time(0));
	for (i = 0; i < N; i++)
		v[i] = G[i]? (rand()) : 0;
#else
	for (i = 0; i < N && !G[i]; i++)
		;
	v[i] = 1;
#endif

	double err1 = 1, err2 = 1;
	int ITER = 0;
	double vNorm = 0;
	double v_k;
	double temp1;

	while (err1 > Epsilon && err2 > Epsilon && ITER < MAX_ITER)
	{
		for (i = 0; i < N; i++)
			v0[i] = G[i] ? v[i]: 0;
		v_k = 0;
		// The dot product of v * k
		for (i = 0; i < N; i++)
			v_k += G[i] ? v0[i] * (R[i+1] - R[i]): 0;
		// Do the matrix-vector multiplication
		for (i = 0; i < N; i++)
		{
			if (!G[i]) 
				continue;
			temp1 = 0;
			for (j = R[i]; j < R[i+1]; j++)
				temp1 += G[C[j]] ? v0[C[j]] : 0;
			temp1 -= v_k / 2 / M * (R[i+1] - R[i]) + (sumBG1[i] - beta) * v0[i];
			v[i] = temp1;
		}
		// Decide whether converge
		vNorm = VectorNorm(v, G, N);
		for (i = 0; i < N; i++)
			v[i] = G[i] ? (v[i] / vNorm) : 0;
		for (i = 0; i < N; i++)
			verr[i] = G[i] ? (v[i] - v0[i]) : 0;
		err1 = VectorNorm(verr, G, N);
		for (i = 0; i < N; i++)
			verr[i] = v[i] + v0[i];
		err2 = VectorNorm(verr, G, N);
		ITER++;
	}
	cout<<"Iterations:\t"<<ITER<<'\t'<<"residual:\t"<<min(err1, err2)<<'\t';
	fout<<"Iterations:\t"<<ITER<<'\t'<<"residual:\t"<<min(err1, err2)<<'\t';

	// return the eigenvalue
	int max_index = 0;
	v[0] = G[0] ? v[0] : 0;
	for (i = 0; i < N; i++)
		if (G[i] && fabs(v[i]) > fabs(v[max_index]))
			max_index = i;
	return (v[max_index] * v0[max_index] > 0) ? vNorm: -vNorm;
}

/* 
This function returns the norm of the input vector x[G].
G is the logic subscriber and N is the matrix dimension.
*/
template <class Type>
double VectorNorm(Type * x, bool * G, int N)
{
	int i = 0;
	double Norm = 0;
	for (i = 0; i < N; i++)
		Norm += G[i] ? (x[i] * x[i]) : 0 ;
	return sqrt(Norm);
}

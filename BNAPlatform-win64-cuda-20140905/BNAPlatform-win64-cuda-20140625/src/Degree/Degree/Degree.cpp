# include <iostream>
# include <iomanip>
# include <fstream>
# include <memory.h>
# include <stdlib.h>
# include <cstring>
# include <ctime>
# include "dirent.h"
using namespace std;


int main(int argc, char * argv[])
{
	ofstream flog("BNA_time_log", ios::app);
	clock_t total_time = clock();
	if (argc < 2) 
	{
		cerr<<"Input format: .\\Deg.exe dir_for_csr \nFor example: .\\Deg.exe d:\\data "<<endl;
		exit(1);	
	}

	DIR *dp;
	struct dirent *dirp;
	if (NULL == (dp = opendir(argv[1])))
	{
		printf("can't open %s", argv[1]);
		exit (1);
	}
	int FileNumber = 0;
	string filenametmp;
	while((dirp = readdir(dp)) != NULL)
	{
		filenametmp = string(dirp->d_name);
		
		if (filenametmp.find_last_of('.') == -1)
			continue;
		if(filenametmp.length()>4 && filenametmp.substr(filenametmp.find_last_of('.'),4).compare(".csr") == 0 && filenametmp.size() - filenametmp.find_last_of('.') - 1 == 3)
		{
			FileNumber++;
		}
	}
	cout<<FileNumber<<" files to be processed."<<endl;

	closedir(dp);
	string *filename = new string[FileNumber];
	dp = opendir(argv[1]);
	int i = 0;
	while((dirp = readdir(dp)) != NULL)
	{
		filenametmp = string(dirp->d_name);
		if (filenametmp.find_last_of('.') == -1)
			continue;
		if(filenametmp.length()>4 && filenametmp.substr(filenametmp.find_last_of('.'),4).compare(".csr") == 0 && filenametmp.size() - filenametmp.find_last_of('.') - 1 == 3)
		{
			filename[i++] = filenametmp;
		}
	}

	string isolated_v_file = string(argv[1]).append("\\").append("isolated_v_mark.txt");
	ofstream iso_file;
	iso_file.open(isolated_v_file.c_str(), ios::out);

	for (int i = 0; i < FileNumber; i++)
	{
		string a = string(argv[1]).append("\\").append(filename[i]);
		cout<<"\ncalculating Degree for "<<a.c_str()<<" ..."<<endl;
		ifstream fin(a.c_str(), ios_base::binary);
		if (!fin.good())
		{	cout<<"Can't open\t"<<a.c_str()<<endl;	return 0;}

		// Read x.csr
		int Rlength = 0, Clength = 0;
		fin.read((char*)&Rlength, sizeof(int));
		int * R = new int [Rlength];
		fin.read((char*)R, sizeof(int) * Rlength);
		//fin.read((char*)&Clength, sizeof(int));
		//int * C = new int [Clength];
		//fin.read((char*)C, sizeof(int) * Clength);
		fin.close();
		int N = Rlength - 1;

		clock_t time = clock();
		int isolated_n = 0;
		int *Degree = new int [N];
		for (int k = 0; k < N; k++)
		{
			Degree[k] = R[k+1] - R[k];
			if(Degree[k]==0)
				isolated_n++;
		}
		iso_file<< isolated_n << endl;
		time = clock() - time;
		cout<<"Elapsed time: "<<time<<" ms. "<<endl;
		flog<<"Degree\t"<<a.c_str()<<"CPU\tkernel time\t"<<1.0*time/1000<<"s"<<endl;
		// Parse file name
		string X_deg = a.substr(0, a.find_last_of('.') + 1).append("deg");
		cout<<"Save degree data for each node as "<<X_deg.c_str()<<endl;
		ofstream fout;
		fout.open(X_deg.c_str(), ios::binary|ios::out);
		fout.write((char*)&N, sizeof(int));
		fout.write((char*)Degree, sizeof(float) * N);
		fout.close();

		delete []Degree;
		delete []R;
		//delete []C;
	}
	iso_file.close();
	total_time = clock() - total_time;
	cout<<"total elapsed time: "<<1.0*total_time/1000<<" s."<<endl;
	cout<<"==========================================================="<<endl;
	flog<<"Degree\tCPU\ttotal time\t"<<1.0*total_time/1000<<"s"<<endl;
	flog<<endl;
	flog.close();
	delete []filename;
	return 0;
}

# include <iostream>
# include <ctime>
using namespace std;

void myprint_blocked(float * costmat, int numVertices, int block_size)
{
	int block_cnt	= (numVertices + block_size - 1) / block_size;
	int index = 0;
	for (int i = 0; i < numVertices; i++)
	{
		for (int j = 0; j < numVertices; j++)
		{
			int block_row = i / block_size;
			int block_col = j / block_size;
			int block_i   = i % block_size;
			int block_j   = j % block_size;
			int offset    = block_row * block_cnt + block_col;
			offset *= block_size*block_size;
			cout<<costmat[block_i * block_size + block_j + offset]<<"\t";
		}
		cout<<endl;
	}
	cout<<endl;
}

void BFW_one_block_C(float* dst_ij, float* src_ik, float* src_kj, int block_size)
{
	for (int k = 0; k < block_size; k++)
		{
			for (int i = 0; i < block_size; i++)
			{
				for (int j = 0; j < block_size; j++)
				{
					dst_ij[i * block_size + j] = min(dst_ij[i * block_size + j], src_ik[i * block_size + k] + src_kj[k * block_size + j]);
				}
			}
		}
}

void BFW_C(float *costmat, int numVertices, int block_size)
{
	clock_t time_cpu = 0;
	int block_cnt = numVertices / block_size;
	for (int k_block = 0; k_block < block_cnt; k_block++)
	{
		//phase 1
		clock_t time_ph1_cpu = clock();
		int block_row = k_block;
		int block_col = k_block;
		int offset_ph1 = (block_row * block_cnt + block_col) * block_size * block_size;

		BFW_one_block_C(costmat + offset_ph1, costmat + offset_ph1, costmat + offset_ph1, block_size);
		time_ph1_cpu = clock() - time_ph1_cpu;
		cout<<"Phase1 cpu time: "<<time_ph1_cpu<<" ms."<<endl;
		time_cpu += time_ph1_cpu;
		
		//phase 2
		clock_t time_ph2_cpu = clock();
		for (int colrow = 0; colrow < block_cnt; colrow ++) 
		{
			if (colrow == k_block)
				continue;
			int block_row = k_block;
			int block_col = colrow;
			int offset_ph2 = (block_row * block_cnt + block_col) * block_size * block_size;
			BFW_one_block_C(costmat + offset_ph2, costmat + offset_ph1, costmat + offset_ph2, block_size);
		}
		for (int colrow = 0; colrow < block_cnt; colrow ++) 
		{
			if (colrow == k_block)
				continue;
			int block_row = colrow;
			int block_col = k_block;
			int offset_ph2 = (block_row * block_cnt + block_col) * block_size * block_size;
			BFW_one_block_C(costmat + offset_ph2, costmat + offset_ph2, costmat + offset_ph1, block_size);
		}
		time_ph2_cpu = clock() - time_ph2_cpu;
		cout<<"Phase2 cpu time: "<<time_ph2_cpu<<" ms."<<endl;
		time_cpu += time_ph2_cpu;

		//phase 3
		for (int block_row = 0; block_row < block_cnt; block_row ++) 
		{
			if (block_row == k_block) continue;
			
			for (block_col = 0; block_col < block_cnt; block_col ++) 
			{
				if (block_col == k_block) continue;
				
				clock_t time_ph3_cpu = clock();
				int src_ik_row = block_row;
				int src_ik_col = k_block;
				int src_kj_row = k_block;
				int src_kj_col = block_col;

				int offset_ph3 = (block_row * block_cnt + block_col) * block_size * block_size;
				int offset_src_ik = (src_ik_row * block_cnt + src_ik_col) * block_size * block_size;
				int offset_src_kj = (src_kj_row * block_cnt + src_kj_col) * block_size * block_size;
				BFW_one_block_C(costmat + offset_ph3, costmat + offset_src_ik, costmat + offset_src_kj, block_size);
				time_ph3_cpu = clock() - time_ph3_cpu;
				cout<<"Phase3 cpu time: "<<time_ph3_cpu<<endl;	
				time_cpu += time_ph3_cpu;
			}		
		}
	}
	cout<<"�������������������������������������"<<endl;
	cout<<"Total cpu time: "<<time_cpu<<" ms."<<endl;
}
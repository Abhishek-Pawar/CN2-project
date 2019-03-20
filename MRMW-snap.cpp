#include <iostream>
#include <fstream>
#include <thread>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <vector>
#include <mutex>
#include <algorithm>
#include <pthread.h>
#include <random>
#include <atomic>
#include <ctime>
#include <set>

using namespace std;

atomic_int** HELPSNAP;
int num, lambda1 ,lambda2 ,k;
atomic_bool term;
float avg_time = 0;

class MRMWSnap
{
	public:
		atomic_int value;
		int pid;
		int seq;

		MRMWSnap()
		{
			value.store(-1, memory_order_relaxed);
			pid = -1;
			seq = 0;

		}


		void update_val(int id ,int val)
		{	
			seq++;
			pid = id;
			value.store( val , std::memory_order_relaxed);
			HELPSNAP[id] = snap();

		}

		atomic_int snap()
		{
			set<int> can_help;
			MRMWSnap aa[num];
			MRMWSnap bb[num];

			for(int i = 0 ;i < num ;i++)
			{
				//aa[i] = reg[i];
				(aa[i].value).store(reg[i].value , memory_order_relaxed);
				aa[i].seq = reg[i].seq;
				aa[i].pid = reg[i].pid; 
			}


			while(1)
			{
				for(int i = 0 ;i < num ;i++)
				{
				    (bb[i].value).store(reg[i].value , memory_order_relaxed);
					bb[i].seq = reg[i].seq;
					bb[i].pid = reg[i].pid;
				}

				int count = 0;

				for(int i = 0 ;i < num ;i++)
				{
					if(aa[i].value == bb[i].value)
					{
						count++;
					}
				}
				
				if(count != num)
				{
					atomic_int ans[num];

					for (int i = 0; i < num ; i++)
					{
						ans[i].store(bb[i].value , memory_order_relaxed);
					}

					return ans;
				}

				for(int i = 0 ;i < num ;i++)
				{
					if(aa[i].value != bb[i].value)
					{
						const bool is_in = can_help.find(i) != can_help.end();
						if(is_in)
						{
							return HELPSNAP[i];
						}
						else
						{
							can_help.insert(i);
						}
					}
				}

				for(int i = 0 ;i < num ;i++)
				{
					(aa[i].value).store(bb[i].value , memory_order_relaxed);
					aa[i].seq = bb[i].seq;
					aa[i].pid = bb[i].pid;
				}

			}

		}




};

MRMWSnap* reg;
MRMWSnap** HELPSNAP;


double ran_exp(float lambda)   						//finds the ranom exponential time for slaeping the thread
{
	default_random_engine generate;
	srand(5);
	exponential_distribution <double> distribution(1/lambda);

	return distribution (generate);
}

void thWrite(int i)
{	int number;
	time_t exitTime;
	srand(time(NULL));
	while(!term)
	{
		
		number = i+rand()%100;
		reg[i].update_val(i,number);

		exitTime = time(0);
		tm* ltm = localtime(&exitTime);
		printf("Thread%d writes %d at %d:%d\n", i+1 ,number,1 + ltm->tm_min,1 + ltm->tm_sec);
		usleep(100000*ran_exp(lambda1));
	}
}


void getSnap()
{
	int i = 0;
	time_t exitTime;
	while(i < k)
	{
		auto start = std::chrono::system_clock::now();
		atomic_int* res  = cls->scan();
		auto end = std::chrono::system_clock::now();
		exitTime = time(0);
		string s = "Snapshot: ";

		for (int j = 0; j < num ; j++)
		{	int val = res[j].load(std::memory_order_relaxed);
			s+= "th"+ to_string(j+1) + "-"+to_string(val);
		}
		tm* ltm = localtime(&exitTime);
		s+= "finished at" + to_string(1+ltm->tm_min) +":" + to_string(1+ltm->tm_sec);

		printf("%s\n", s.c_str());
		std::chrono::duration<double> elapsed_seconds = end - start;
		avg_time += elapsed_seconds.count();
		usleep(100000*ran_exp(lambda2));

		i++;
	}

	term.store(true , std::memory_order_relaxed);
}


int main()
{
	cout<<"enter the params"<<endl;
	cin>>num>>lambda1>>lambda2>>k ;

	term.store(false,std::memory_order_relaxed);

	reg = new MRMWSnap[num];
	HELPSNAP = new MRMWSnap*[num];

	for(int i = 0; i < num ; i++) 
	{
		HELPSNAP[i] = reg;
	}

	thread writers[num];

	for(int i=0 ;i<num ; i++)
	{
		writers[i] = thread(thWrite ,i);
	}

	// thread snappy = thread(getSnap);
	// snappy.join();

	for(int i=0 ;i<num ; i++)
	{
		writers[i].join();
	}


	//delete cls;
	return 0;

}


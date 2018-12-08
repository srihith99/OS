// C++ libraries for fileIO , Math and Time measuring
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <sstream>
#include <algorithm>	
#include <chrono>

//libraries for time , fork and wait functions
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>


//libraries for creating shared memory objects 
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

using namespace std;
using namespace std::chrono;

//start,end timers
high_resolution_clock::time_point start_time,end_time;	


//Contains static methods for required statistics
class Statistic
{
	public:
		//average = sum/count
		static long double Average(vector<int> v)
		{
			int n = v.size();
			long double sum = 0.0;
			for(int i=0;i<n;i++)
				sum += v[i];
			return sum/n;
		}

		//SD is root mean square of differences with respect to mean
		static long double Standard_deviation(vector<int> v)
		{
			int n = v.size();
			long double sum = 0.0;
			long double square_sum = 0.0;
			for(int i=0;i<n;i++)
			{
				sum += v[i];
			}
			long double mean = sum/n;
			for(int i=0;i<n;i++)
				square_sum += (v[i]-mean)*(v[i]-mean);
			return sqrt( square_sum/n ); 
		}

		//Median is the middle element in sorted order of the array
		static long double Median(vector<int> v)
		{
			int n = v.size();
			sort(v.begin(),v.end());
			if(n%2 == 1)
				return v[n/2];
			return ( v[(n-1)/2]+v[(n)/2] )/2.0;
		}
};



class ProcStat
{
	private:
		//input vector , statistics are class variables
		vector<int> input;
		string average,standard_deviation,median;
		
		//size and names of memory mapped objects
		const int SIZE = 4096;
		const char* name1 = "Shared_memory1";
		const char* name2 = "Shared_memory2";
		const char* name3 = "Shared_memory3";

		//A private method to strip trailing spaces of a string
		void rightstrip(string& s)
		{
			int n = s.size();
			n--;
			while(n>=0 && s[n]==' ')
			{
				s.erase(s.begin()+n);
				n--;
			}
			return;
		}

	public:
		//Takes input as a string and loads the input integer vector
		ProcStat()
		{
			string line1,line2;
			getline(cin,line1);
			getline(cin,line2);
		    int prev = 0;
		    rightstrip(line2);
		    for(int i=0;i<=line2.size();i++)
		    {
		 		if(i==line2.size() || line2[i]==' ')
		 		{
		 			//string to integer convesrion using stringstream
		 			string temp(line2.begin()+prev,line2.begin()+i);
		 			stringstream convert(temp);
		 			int num;
		 			convert >> num;
		 			input.push_back(num);
		 			prev = i+1; 
		 		}   	
		    }	
		}

		void Run()
		{	
			//Three memory mapped objects are created into which the processes parallely write into
			int fd1 = shm_open(name1,O_CREAT | O_RDWR | O_TRUNC,0666);
			int fd2 = shm_open(name2,O_CREAT | O_RDWR | O_TRUNC,0666);
			int fd3 = shm_open(name3,O_CREAT | O_RDWR | O_TRUNC,0666);
			ftruncate(fd1,SIZE);
			ftruncate(fd2,SIZE);
			ftruncate(fd3,SIZE);
			pid_t p1 = fork();
			pid_t p2 = fork();
			if(p1 == -1 || p2==-1)
			{
				//fork failure case
				cout<<"fork error"<<endl;
				exit(0);
			}
			
			//child of child
			if(p1 ==0 && p2==0)
			{
				//sleep(2);
				char* ptr1 = (char*) mmap(0,SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd1,0);
				if(ptr1 == MAP_FAILED)
				{
					// map fail check
					cout<<"mmap error"<<endl;
					exit(0);
				}
				//Process for computing average , the output is sent vua ptr1
				long double avg=Statistic::Average(input);
				sprintf(ptr1,"%Lf",avg);
			}
			
			//parent of above child
			if(p1 ==0 && p2>0)
			{
				//sleep(3);
				//wait's for its coresponding child to complete
				waitpid(p2,NULL,0);	
				char* ptr2 = (char*) mmap(0,SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd2,0);
				if(ptr2 == MAP_FAILED)
				{
					//map fail check
					cout<<"mmap error"<<endl;
					exit(0);
				}
				//SD calculation , output is sent via ptr2
				long double s_d=Statistic::Standard_deviation(input);
				sprintf(ptr2,"%Lf",s_d);
			}
			
			if(p1 >0 && p2==0)
			{
				//sleep(1);
				//wait for it's corresponding child
				waitpid(p1,NULL,0);
				char* ptr3 = (char*) mmap(0,SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd3,0);
				if(ptr3 == MAP_FAILED)
				{
					//map fail check
					cout<<"mmap error"<<endl;
					exit(0);
				}
				//Median Calculation , output is sent via ptr3
				long double med=Statistic::Median(input);
				sprintf(ptr3,"%Lf",med);
			}
			
			if(p1 >0 && p2 > 0)
			{
				//wait's for all of it's children
				waitpid(p1,NULL,0);
				waitpid(p2,NULL,0);	
				//once remaining processes are done , look at the mmap files 
				char* fptr1 = (char*) mmap(0,SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd1,0);
				char* fptr2 = (char*) mmap(0,SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd2,0);
				char* fptr3 = (char*) mmap(0,SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd3,0);
				if(fptr1 == MAP_FAILED || fptr2 == MAP_FAILED || fptr3 == MAP_FAILED) 
				{
					//map error check
					printf("mmap error\n");
					exit(0);
				}
				//converrting char* arrays into strings
				string s1(fptr1),s2(fptr2),s3(fptr3);
				//load the class statistc variables
				average = s1;
				standard_deviation = s2;
				median = s3;
				//unmap and unlink the created objects
				munmap(fptr1,SIZE);
				shm_unlink(name1);
				munmap(fptr2,SIZE);
				shm_unlink(name2);
				munmap(fptr3,SIZE);
				shm_unlink(name3);
				//record the final timestamp
				end_time =  high_resolution_clock::now();
				//Get the duration of process execution
				auto duration = duration_cast<microseconds>(end_time-start_time).count();
				cout<<duration<<endl;
				//dump the answers into a file
				FileDump();

				ofstream time_file; 
				time_file.open("time.csv",ios::out | ios::app);
				time_file<<duration<<endl;
				time_file.close();

			}
		}

		//C++ file IO
		void FileDump()
		{
			ofstream myfile;
			myfile.open("out_process.txt");
			myfile << "The average value is " + average<<endl;
			myfile << "The standard deviation is " + standard_deviation<<endl;
			myfile << "The median value is " + median<<endl;
			myfile.close();
		}
};



int main()
{
	ProcStat p;
	//record start time and then run the processes
	start_time = high_resolution_clock::now();
    p.Run();
	return 0;
}
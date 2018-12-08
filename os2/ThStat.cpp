// C libraries for exit() syscalls , POSIX thread functionalities
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

//C++ FileIO , Math , Sort and Time measuring libraries
#include <iostream>
#include <fstream>
#include <math.h>
#include <sstream>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;	

//size of input array
int n;
//list statistics
long double average,standard_deviation,median;


//All static methods responsible for required metrics 
class Statistic
{
	public :
		
		//Median of array
		static void *Median(void *array_ptr)
		{
			//type-cast the void* ptr to int* inorder to get back the original array
			int *temp = (int*)array_ptr; 	
			
			//sort the array in ascending order
			sort(temp,temp+n);

			//Apply median definition
			if(n%2 == 1)
				median = temp[n/2];
			else
			    median = ( temp[(n-1)/2]+ temp[(n)/2] )/2.0;

		}

		//Standard deviation of array
		static void *Standard_deviation(void *array_ptr)
		{
			int *temp = (int*)array_ptr; 	
			long double sum = 0.0;
			long double square_sum = 0.0;
			for(int i = 0; i < n; i++)
			{		
				sum += temp[i];	
			}
			long double mean = sum/n;
			for(int i=0;i<n;i++)
				square_sum += (temp[i]-mean)*(temp[i]-mean);
			//standard deviation is the root mean square of differences wrt mean	
			standard_deviation = sqrt( square_sum/n );	
			return NULL;
		}

		//Average of array
		static void *Average(void *array_ptr)
		{
			int *temp = (int*)array_ptr; 	
			long double sum = 0.0;
			for(int i = 0; i < n; i++)
			{		
				sum += temp[i];	
			}
			//Sum divided by count
			average = sum/n;		
			return NULL;
		}
};



typedef void* (*fptr)(void*);
//Array of function pointers each referencing a required statistic in above class
fptr fpointer[3] = {Statistic::Average,Statistic::Standard_deviation,Statistic::Median};	

//Thread identifiers of the thread to be created
pthread_t thread[3];

class Thread
{ 
	public :
		
		static void createThreads(int *array_ptr)
		{
			 //create 3 threads
			 for(int i=0;i<=2;i++)
			 {
			 	//Each thread will invoke a different function in the order defined in the fpointer array
			 	if( pthread_create(&thread[0], NULL,fpointer[i], (void *)array_ptr) != 0 )
			 	{
			 		//Thread creation failure handling
			 		cout<<"Error in create_thread\n";
			 		exit(0);
			 	}
			 }
		}

		static void joinThreads()
		{
			//joining all the spawned threads
			int thread_count = 2;
			while(thread_count >= 0)
			{ 
				pthread_join(thread[thread_count], NULL);
				thread_count--;
			}
			
		}
};

class ThStat
{
	private:
		
		//input array
		int* array_ptr;

		// A method which strips off trailing spaces in a string
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
		
		ThStat()
		{
			//takes input and builds the array
			string line1,line2;
			getline(cin,line1);
			//converting string to num using string stream C++
			stringstream convert(line1);
		 	convert >> n;
		 	array_ptr = new int[n+1];
			getline(cin,line2);
		    int prev = 0;
		    rightstrip(line2);
		    int count = 0;	
		    for(int i=0;i<=line2.size();i++)
		    {
		 		if(i==line2.size() || line2[i]==' ')
		 		{
		 			string temp(line2.begin()+prev,line2.begin()+i);
		 			stringstream convert(temp);
		 			int num;
		 			convert >> num;
		 			array_ptr[count] = num;
					count++;
		 			prev = i+1; 
		 		}   	
		    }
		}

		void Run()
		{
			//create 3 threads
			Thread::createThreads(array_ptr);
			//Wait for all three threads to exit
			Thread::joinThreads();	
			return;
		}

		//Dumps the output to a file 
		void Filedump()
		{
			ofstream myfile;
			myfile.open("out_thread.txt");
			myfile << "The average value is "<< average<<endl;
			myfile << "The standard deviation is "<<standard_deviation<<endl;
			myfile << "The median value is "<<median<<endl;
			myfile.close();
			return;	
		}
};


int main()
{
	ThStat t;
	
	//record start time
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
    //run the threads
    t.Run();
    //record end time
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
	
	//dump the output in a file 
	t.Filedump();
	
	//Get the duration of the thread execution
	auto duration = duration_cast<microseconds>(t2-t1).count();
	cout<<duration<<endl;

	ofstream time_file; 
	time_file.open("time.csv",ios::out | ios::app);
	time_file<<n<<","<<duration<<",";
	time_file.close();
	
	return 0;
}
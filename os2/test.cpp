#include <iostream>
#include <fstream>
#include <stdio.h> 
#include <stdlib.h> 
#include<time.h>
using namespace std;
int main()
{
	ofstream time_file;
	time_file.open("time.csv");
	time_file << "input_size,thread_time(in microseconds),process_time(in microseconds)" << endl;
	time_file.close();
	system("g++ -std=c++11 ThStat.cpp -pthread -o thread");
	system("g++ -std=c++11 ProcStat.cpp -lpthread -lrt -o process");
	srand(time(0));
	int n = 1000000,step = 1000000;
	for(int i=1;i<=5;i++)
	{
		srand(time(0));
		ofstream input_file;
		input_file.open("inp.txt");
		input_file<<n<<endl;
		int temp = rand()%100;
		input_file << temp;
		for(int j=1;j<n;j++)
		{
			temp = rand()%100;
			input_file << " " << temp;
		}
		input_file<<endl;
		input_file.close();
		system("./thread <inp.txt");
		system("./process <inp.txt");
		n = n+step;
	}
	//system("cat time.csv");
	cout<<"time.csv file generated "<<endl;
	system("rm process thread");
	return 0;
}
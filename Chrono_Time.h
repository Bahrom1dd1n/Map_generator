#ifndef __Chrono_Timing__
#define __Chrono_Timing__

#include<iostream>
#include<chrono>

// this timer object is used to determini time performance of code
class Timer
{
public:
	Timer()
	{
		start = std::chrono::high_resolution_clock::now();
	}

	void Stop()
	{
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		std::cout<<duration.count()*1000<<'\n';
	}

	~Timer()
	{
		this->Stop();
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> b;

	std::chrono::time_point<std::chrono::steady_clock> start, end; // these are time points
	std::chrono::duration<float> duration;
};

void foo()
{
	Timer t;
	for (int i = 0; i < 10000; i++)
		std::cout << "Hello\n";
}

#endif // !__Chrono_Timing__

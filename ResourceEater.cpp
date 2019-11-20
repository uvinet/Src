// ResourceEater.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <conio.h>

static std::atomic<bool> st_bEat = true;

int main()
{
	struct EaterPool
	{
		EaterPool()
		{
			for(unsigned int n=0; n<std::thread::hardware_concurrency(); n++)
				threads_vector.emplace_back([]{
					while (st_bEat)
						;//yum-yum
				});
		}
		~EaterPool()
		{
			for (auto& t : threads_vector)
				t.join();
		}
	private:
		std::vector<std::thread> threads_vector;
	} ep;

	printf("Eating...\npress any key to stop");
	_getch();
	st_bEat = false;
}

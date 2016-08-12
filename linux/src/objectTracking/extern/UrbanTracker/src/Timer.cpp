#include "Timer.h"
#include <iostream>

#define PERFTIMER

Timer::Timer(const std::string& timerName)
:mName(timerName)
{
	
}

Timer::~Timer()
{

}

void Timer::start()
{

#ifdef WIN32
	 QueryPerformanceFrequency(&mFrequency);
	 QueryPerformanceCounter(&mTimeStart);
	
#else
	mTimeStart = clock();
#endif

}

void Timer::stop()
{

	#ifdef WIN32
	 QueryPerformanceCounter(&mTimeStop);
	
#else
	mTimeStop = clock();
#endif


}

void Timer::printResult(const std::string& name)
{
	#ifdef PERFTIMER
	std::string tmpName = name != "" ? name : mName;


	#ifdef WIN32
	double msByFrame =  1000.0*(mTimeStop.QuadPart - mTimeStart.QuadPart) / (double)mFrequency.QuadPart ;
	#else
	double msByFrame = 1000*(mTimeStop-mTimeStart) / ((double) CLOCKS_PER_SEC);
#endif
	std::cout << "Timer_" << tmpName << ": " << msByFrame << "ms. " << std::endl;
	/*
	//Resolution is very approximative (10 ms).
	if(sByFrame < 0.01)
		std::cout << "Timer_" << mName << ": " << sByFrame << "s by frame. " << ">100" << " fps." << std::endl;
	else
		std::cout << "Timer_" << mName << ": " << sByFrame << "s by frame. " <<  1/sByFrame << " fps." << std::endl;*/
	#endif
}


void Timer::printTimeSinceLastPrint(const std::string& name)
{
	#ifdef PERFTIMER
	stop();
	printResult(name);
	start();
#endif
}

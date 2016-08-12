#ifndef TIMER_H
#define TIMER_H
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NONIMAX
#include <windows.h>                // for Windows APIs

#else
#include <time.h>
#endif
#include <string>

class Timer
{
public:
	Timer(const std::string& timerName= "");
	virtual ~Timer();
	void start();
	void stop();
	void printResult(const std::string& name = "");
	void printTimeSinceLastPrint(const std::string& name = "");

protected:
#ifdef WIN32
	LARGE_INTEGER mFrequency;
	LARGE_INTEGER mTimeStart;
	LARGE_INTEGER mTimeStop;
#else
	clock_t mTimeStart;
	clock_t mTimeStop;

#endif

	
	std::string mName;
	int mLastNbFrame;
};

#endif
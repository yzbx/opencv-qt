#include "DrawableTimer.h"
#include "StringHelpers.h"

DrawableTimer::DrawableTimer(const std::string& timerName)
: Timer(timerName) 
{

}

DrawableTimer::~DrawableTimer()
{

}

//http://www.songho.ca/misc/timer/timer.html
void DrawableTimer::drawFPS(cv::Mat& image)
{
#ifdef _WIN32
	double sByFrame =  (double)(mTimeStop.QuadPart - mTimeStart.QuadPart) /(double) mFrequency.QuadPart;
#else
	double sByFrame = ((double)mTimeStop-(double)mTimeStart) / ((double) CLOCKS_PER_SEC);
#endif

	std::string fps =  Utils::String::toString((int)(1/sByFrame));
	cv::putText(image, fps, cv::Point(0,25), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0,255,255),2);
}

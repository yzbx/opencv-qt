#ifndef DRAWABLE_TIMER_H
#define DRAWABLE_TIMER_H

#include "Timer.h"
#include <opencv2/opencv.hpp>

class DrawableTimer: public Timer
{
public:
	DrawableTimer(const std::string& timerName= "");
	virtual ~DrawableTimer();
	void drawFPS(cv::Mat& image);
private:

};
#endif
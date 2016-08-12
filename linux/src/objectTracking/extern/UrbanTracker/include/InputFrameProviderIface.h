#ifndef INPUT_FRAME_PROVIDER_IFACE_H
#define INPUT_FRAME_PROVIDER_IFACE_H

#include <opencv2/opencv.hpp>
#include <string>
#include "StringHelpers.h"
#include "Logger.h"

class InputFrameProviderIface
{
public: 
	virtual bool getNextFrame(cv::Mat&)=0;
	virtual ~InputFrameProviderIface(){}
	virtual unsigned int getNbFrame() = 0;
	virtual double getNbFPS() = 0;
	virtual bool isOpen() const = 0;
	virtual void setNumberFrame(unsigned int nb) = 0;
	virtual bool seekAtFramePos(unsigned int framepos) = 0;
	virtual bool seekAtMs(unsigned int msec) = 0;
	virtual unsigned int getNextFramePos() = 0;
	virtual int getWidth() = 0;
	virtual int getHeight() = 0;

	bool seekAtMs(const std::string& start, const std::string& position)
	{
		bool success = false;
		unsigned int long start_ms = get_ms(start);
		unsigned int long position_ms = get_ms(position);
		//If we are at midnight, there is a wrap around.
		if(start_ms > position_ms) 
			position_ms+=24*3600*1000; //We add a full day
		success = seekAtMs(position_ms-start_ms);
		return success;
	}
private:
	unsigned long get_ms(std::string strPos) const
	{
		StringArray strPosComponents = Utils::String::Split(strPos, ":", 0);
		assert(strPosComponents.size() == 3);
		unsigned long nbMs = 0;
		unsigned int hours, minutes, seconds;
		if(Utils::String::StringToUInt(strPosComponents[0], hours) && Utils::String::StringToUInt(strPosComponents[1], minutes) &&	Utils::String::StringToUInt(strPosComponents[2], seconds))
		{
			nbMs = (hours*3600+minutes*60+seconds)*1000;
		}
		else
		{
			LOGASSERT(false, "Unknown format for strPos " << strPos);
		}
		return nbMs;
	}
	//Accept string in the format 10:01:42. 
	//start allows to display the start of the video
	//Position should be the desired position

};


#endif
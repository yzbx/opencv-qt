#ifndef INPUT_VIDEO_FILE_MODULE_H
#define INPUT_VIDEO_FILE_MODULE_H

#include "InputFrameProviderIface.h"
#include <string>


class InputVideoFileModule : public InputFrameProviderIface
{
public:
	InputVideoFileModule(const std::string& videoPath);
	~InputVideoFileModule();
	void setNumberFrame(unsigned int nb) { mNumberOfFrame = nb;}// This is useful since MPEG nb of frame is badly reported in OpenCV.
	bool isOpen() const { return mInit;}
	int getVideoType();
	bool getNextFrame(cv::Mat&);
	bool seekAtMs(unsigned int msec);
	bool seekAtFramePos(unsigned int framepos);
	unsigned int getNbFrame();
	double getNbFPS();
	unsigned int getNextFramePos();
	int getWidth();
	int getHeight();
	
private:
	
	cv::VideoCapture mVideoCapture;
	bool mInit;
	unsigned int mNumberOfFrame;
};

#endif
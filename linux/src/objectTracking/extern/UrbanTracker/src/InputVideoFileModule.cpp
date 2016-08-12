#include "InputVideoFileModule.h"
#include "StringHelpers.h"
#include "Logger.h"

InputVideoFileModule::InputVideoFileModule(const std::string& videoPath) 
: mInit(false)
, mNumberOfFrame(0)
{
	mInit = mVideoCapture.open(videoPath.c_str());
	if(mInit)
	{
		double frameCount;
		frameCount = mVideoCapture.get(CV_CAP_PROP_FRAME_COUNT);	
		mNumberOfFrame = (unsigned int)frameCount;
	}
	else
		std::cout << "Error: cannot open " << videoPath << std::endl;
}

int InputVideoFileModule::getVideoType()
{
	return mVideoCapture.get(CV_CAP_PROP_FORMAT);
}


bool InputVideoFileModule::seekAtMs(unsigned int msec)
{
	bool success = false;
	int nbFrame = getNbFrame();
	if(msec/getNbFPS() <= nbFrame)
	{
		mVideoCapture.set(CV_CAP_PROP_POS_MSEC, msec);
		success = true;
	}
	return success;
}

int InputVideoFileModule::getWidth()
{
	return (int)mVideoCapture.get(CV_CAP_PROP_FRAME_WIDTH);
}

int InputVideoFileModule::getHeight()
{
	return (int)mVideoCapture.get(CV_CAP_PROP_FRAME_HEIGHT);
}

bool InputVideoFileModule::seekAtFramePos(unsigned int framepos)
{
	bool success = false;
	if(framepos <= getNbFrame())
	{
		mVideoCapture.set(CV_CAP_PROP_POS_FRAMES, framepos);
		success = true;
	}
	return success;
}

unsigned int InputVideoFileModule::getNextFramePos()
{
	double framePos;
	framePos = mVideoCapture.get(CV_CAP_PROP_POS_FRAMES); // This return the next frame number
	return (unsigned int)framePos; 
}

unsigned int InputVideoFileModule::getNbFrame()
{
	return mNumberOfFrame;
}




InputVideoFileModule::~InputVideoFileModule(void)
{

}



double InputVideoFileModule::getNbFPS()
{
	return mVideoCapture.get(CV_CAP_PROP_FPS);
}

bool InputVideoFileModule::getNextFrame(cv::Mat& outputPicture)
{
	bool success = false;
	if(mInit)
	{		
		//for(int it = 0; it < 5; ++it)
		mVideoCapture >> outputPicture;				
		success = !outputPicture.empty();
	}
	return success;
}


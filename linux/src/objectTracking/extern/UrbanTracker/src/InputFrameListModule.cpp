#include "InputFrameListModule.h"
#include <fstream>
#include <StringHelpers.h>
InputFrameListModule::InputFrameListModule(const std::string& basePath, const std::string& pictureList, double framerate)
: mFPS(framerate)
, mInit(false)
, mBasePath(basePath+"\\")
, mCurrentIdx(0)
, mWidth(-1)
, mHeight(-1)
{
	loadFileList(pictureList);
}
InputFrameListModule::~InputFrameListModule()
{

}

void InputFrameListModule::setNumberFrame(unsigned int nb)
{
	if(nb >= fileList.size() && nb > 0 )
	{
		fileList.resize(nb);
	}
}

bool InputFrameListModule::getNextFrame(cv::Mat& mat)
{
	bool success = false;
	if((mCurrentIdx+1) < fileList.size() && (mCurrentIdx+1)>=0)
	{
		const std::string& fileName = mBasePath+fileList[mCurrentIdx++];
		mCurrentFrame = cv::imread(fileName);
		if(mWidth == -1 || mHeight == -1)
		{
			mWidth = mCurrentFrame.cols;
			mHeight = mCurrentFrame.rows;
		}


		if(!mCurrentFrame.empty())
			success = true;
		mat = mCurrentFrame;
	}
	
	return success;
}

int InputFrameListModule::getWidth()
{
	if(mWidth == -1 || mHeight == -1)
	{
		cv::Mat tmp;
		if(getNextFrame(tmp))
		{
			mWidth = tmp.cols;
			mHeight = tmp.rows;
		}
	}
	return mWidth;
}

int InputFrameListModule::getHeight()
{
	if(mWidth == -1 || mHeight == -1)
	{
		cv::Mat tmp;
		if(getNextFrame(tmp))
		{
			mWidth = tmp.cols;
			mHeight = tmp.rows;
		}
	}
	return mHeight;
}

unsigned int InputFrameListModule::getNbFrame()
{
	return fileList.size();
}
double InputFrameListModule::getNbFPS()
{
	return mFPS;
}

bool InputFrameListModule::seekAtFramePos(unsigned int framepos)
{
	bool success = false;
	if(mInit&& framepos < fileList.size())
	{
		mCurrentIdx = framepos;
		success = true;
	}
	return success;
}
bool InputFrameListModule::seekAtMs(unsigned int msec)
{
	return seekAtFramePos(mFPS*msec/(1000.));	
}

unsigned int InputFrameListModule::getNextFramePos()
{
	return mCurrentIdx;
}

void InputFrameListModule::loadFileList(const std::string& path)
{
	std::ifstream inputFile(path.c_str());
	std::string fileContains;
	if (inputFile.is_open())
	{
		std::ostringstream contents;
		contents << inputFile.rdbuf();
		inputFile.close();
		fileContains = contents.str();
		fileList = Utils::String::SplitInLines(fileContains);
		mInit = true;
	}
}
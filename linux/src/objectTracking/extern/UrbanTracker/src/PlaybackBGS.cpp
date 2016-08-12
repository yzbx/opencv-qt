#include "PlaybackBGS.h"
#include <sstream>
#include <iostream>

PlaybackBGS::PlaybackBGS(const std::string& path, int currentTimestamp)
: mPath(path)
, mCurrentTimestamp(currentTimestamp)
{
	setPath(path);
}

//Gray input
void PlaybackBGS::process(const cv::Mat &img_input, cv::Mat &img_output)
{
	std::stringstream ss;
	ss << mPath << std::setfill('0') << std::setw(8) << mCurrentTimestamp++ << ".png";	
	std::string bgsPath = ss.str();
	img_output = cv::imread(bgsPath, CV_LOAD_IMAGE_GRAYSCALE);
	if(img_output.empty())
	{
		std::cout << "Bgs frame not available: " << bgsPath << std::endl;
		img_output = cv::Mat::zeros(img_input.size(), CV_8U);
	}
}

void PlaybackBGS::setCurrentFrameNumber(int timestamp)
{
	mCurrentTimestamp = timestamp;
}


void PlaybackBGS::saveConfig()
{

}

void PlaybackBGS::loadConfig()
{

}

void PlaybackBGS::setPath(const std::string& path)
{
	mPath = path;
	if(mPath.back() != '\\')
		mPath.push_back('\\');
}


#ifndef PLAYBACK_BGS_H
#define PLAYBACK_BGS_H

#include "IBGS.h"
#include <string>
#include <opencv2/opencv.hpp>
#include <iomanip>


class PlaybackBGS : public IBGS
{
public:
	PlaybackBGS(const std::string& path = "bgs//", int currentTimestamp=0);
	void process(const cv::Mat &img_input, cv::Mat &img_output);
	void loadConfig();
	void saveConfig();
	void setCurrentFrameNumber(int timestamp);
	void setPath(const std::string& path);
	int getNbChannel() {return 3;}

private:
	std::string mPath;
	int mCurrentTimestamp;


};


#endif

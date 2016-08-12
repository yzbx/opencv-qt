#ifndef INPUT_FRAME_LIST_MODULE_H
#define INPUT_FRAME_LIST_MODULE_H

#include "InputFrameProviderIface.h"
#include <string>
#include <vector>

class InputFrameListModule : public InputFrameProviderIface
{
public:
	InputFrameListModule(const std::string& basePath,const std::string& pictureList, double framerate = 30);
	~InputFrameListModule();
	bool getNextFrame(cv::Mat&);
	unsigned int getNbFrame();
	double getNbFPS();
	bool isOpen() const { return mInit;}
	bool seekAtFramePos(unsigned int framepos);
	bool seekAtMs(unsigned int msec);
	bool seekAtMs(const std::string& start, const std::string& position);
	unsigned int getNextFramePos();
	void setNumberFrame(unsigned int nb);
	int getWidth();
	int getHeight();
private:
	void loadFileList(const std::string& path);
	std::vector<std::string> fileList;
	int mCurrentIdx;
	double mFPS;
	bool mInit;
	std::string mBasePath;
	cv::Mat mCurrentFrame;
	int mWidth;
	int mHeight;
	/*unsigned long get_ms(std::string strPos) const;*/
};

#endif
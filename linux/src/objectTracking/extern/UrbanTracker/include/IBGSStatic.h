#ifndef IBGS_STATIC_H
#define IBGS_STATIC_H

#include <cv.h>

#include "IBGS.h"

class IBGSStatic : public IBGS
{
public:

	virtual ~IBGSStatic(){}
	virtual void removeBlobFromModel(const cv::Mat& labelFrame, const std::set<int> label, const std::vector<cv::Rect>& rectList, const cv::Mat& currentImageHistory) = 0;


};

#endif
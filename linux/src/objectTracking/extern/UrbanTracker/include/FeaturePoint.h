#ifndef FEATURE_POINT_H
#define FEATURE_POINT_H

//We have some macro leaking somewhere
#undef min
#undef max
#include <opencv2/opencv.hpp>

class ApplicationContext;

class FeaturePoint
{	
public:
	FeaturePoint();
	FeaturePoint(const cv::Point2f& pt, ApplicationContext* ac);
	const cv::Point2f& getPoint() const {return mPoint;}
	const cv::Point2f& getProjectedPoint() const {return mProjectedPoint;}
private:
	cv::Point2f mPoint;
	cv::Point2f mProjectedPoint;	
};

#endif
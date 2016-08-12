#include "FeaturePoint.h"
#include "ApplicationContext.h"
#include "OpenCVHelpers.h"

FeaturePoint::FeaturePoint()
{

}

FeaturePoint::FeaturePoint(const cv::Point2f& pt, ApplicationContext* ac)
: mPoint(pt)
{
	//Calculate projected point
	const cv::Mat& perspToPlaneMatrix = ac->getPerspToPlaneMatrix();
	Utils::OpenCV::getProjectedPoint(perspToPlaneMatrix, pt, mProjectedPoint);

}
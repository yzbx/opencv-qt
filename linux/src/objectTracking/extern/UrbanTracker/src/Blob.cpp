#include "Blob.h"
#include "ApplicationContext.h"
#include "OpenCVHelpers.h"
Blob::Blob(const cv::Rect_<float>& bb, ObjectState state, ApplicationContext* context)
: mObjectBoundingBox(bb)
, mState(state)
, mContext(context)
{
	setBoundingBox(bb);
}


void Blob::setBoundingBox(const cv::Rect_<float>& rect)
{ 
	mObjectBoundingBox = rect;
	mCentroid = cv::Point2f(rect.x+rect.width/2.f, rect.y+rect.height/2.f);
	const cv::Mat& perspToPlaneMatrix = mContext->getPerspToPlaneMatrix();
	
	std::vector<cv::Point2f> beforeHomography;
	std::vector<cv::Point2f> afterHomography;
	beforeHomography.push_back(mCentroid);
	//beforeHomography.push_back(cv::Point2f(mCentroid.x-mObjectBoundingBox.width/2.f, mCentroid.y-mObjectBoundingBox.height/2.f));
	//beforeHomography.push_back(cv::Point2f(mCentroid.x+mObjectBoundingBox.width/2.f, mCentroid.y+mObjectBoundingBox.height/2.f)); 
 
	
	beforeHomography.push_back(cv::Point2f(mCentroid.x-mObjectBoundingBox.width/2.f, mCentroid.y)); 
	beforeHomography.push_back(cv::Point2f(mCentroid.x+mObjectBoundingBox.width/2.f, mCentroid.y)); 
	beforeHomography.push_back(cv::Point2f(mCentroid.x, mCentroid.y-mObjectBoundingBox.height/2.f)); 
	beforeHomography.push_back(cv::Point2f(mCentroid.x, mCentroid.y+mObjectBoundingBox.height/2.f));
	cv::perspectiveTransform(beforeHomography,afterHomography, perspToPlaneMatrix);
	mProjectedCentroid = afterHomography[0];
	float x = afterHomography[1].x;
	float width = (afterHomography[2].x -x);
	float y = afterHomography[3].y;
	float height = (afterHomography[4].y -y);
	//int y = (int)afterHomography[3].y;
	//int height = (int)(afterHomography[4].y - y);
	mProjectedBoundingBox = cv::Rect_<float>(x,y,width, height);
}
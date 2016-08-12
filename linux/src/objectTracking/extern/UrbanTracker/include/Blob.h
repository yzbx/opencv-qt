#ifndef BLOB_H
#define BLOB_H

#include "ObjectState.h"
#include <opencv2/opencv.hpp>

class ApplicationContext;

class Blob
{
public:
	
	Blob(const cv::Rect_<float>& bb, ObjectState state, ApplicationContext* context);
	const cv::Rect_<float>& getBoundingBox() const { return mObjectBoundingBox;}
	ObjectState getState() const {return mState;}
	void setBoundingBox(const cv::Rect_<float>& rect);
	cv::Point2f getCentroid() const {return mCentroid;}
	const cv::Point2f& getProjectedCentroid() const {return mProjectedCentroid;}
	const cv::Rect_<float>& getProjectedBoundingBox() const {return mProjectedBoundingBox;}
	void setState(ObjectState state) { mState = state;}
private:
	cv::Rect_<float> mProjectedBoundingBox;
	cv::Point2f mCentroid;
	cv::Rect_<float> mObjectBoundingBox;
	cv::Point2f mProjectedCentroid;
	ObjectState mState;
	ApplicationContext* mContext;
};


#endif
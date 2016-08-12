#ifndef POINTS_BLOB_H
#define POINTS_BLOB_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "OpenCVHelpers.h"
#include <map>
#include "FeaturePoint.h"

struct KpDesc
{
	std::vector<FeaturePoint> mKeyPoints; 
	cv::Mat mDescriptors;
};

class ApplicationContext;

struct PointsBlob
{
	std::vector<cv::KeyPoint> mkeyPoints; 
	cv::Mat mDescriptors;
	std::vector<int> mBlobId; //Id of each keypoints

	void reset();
	void calculatePointBlobAssociation(const cv::Mat& mask, ApplicationContext* c);
	void getKpDesc(int blobId, std::vector<FeaturePoint>& kp, cv::Mat& desc);
	void updateBlobId(int oldId, int newId);


private:
	std::map<int, KpDesc> mBlobIdToKpDesc;
	
};


#endif
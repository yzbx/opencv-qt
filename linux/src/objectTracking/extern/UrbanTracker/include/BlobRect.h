#ifndef BLOB_RECT_H
#define BLOB_RECT_H

#include <opencv2/opencv.hpp>

struct BlobRect
{
	BlobRect(){}
	BlobRect(const cv::Rect_<float>& bb, const cv::Mat& blob)
	: mBoundingBox(bb)
	, mBlob(blob)
	{

	}

	static int getAreaIntersection(const BlobRect& A, const BlobRect& B)
	{
		cv::Rect_<float> intersection = A.mBoundingBox & B.mBoundingBox;
		int area = (int)intersection.area();
		if(area > 0)
		{
			cv::Rect intersectionA = cv::Rect(intersection.x-A.mBoundingBox.x, intersection.y-A.mBoundingBox.y, intersection.width, intersection.height);
			const cv::Mat& AIntersection = A.mBlob(intersectionA);
			cv::Rect intersectionB = cv::Rect(intersection.x - B.mBoundingBox.x,intersection.y- B.mBoundingBox.y, intersection.width, intersection.height);
			const cv::Mat& BIntersection = B.mBlob(intersectionB);
			cv::Mat andMat;
			cv::bitwise_and(AIntersection, BIntersection, andMat);
			area = cv::sum(andMat)[0];
		}
		return area;
	}



	cv::Rect_<float> mBoundingBox;
	cv::Mat mBlob;
};

#endif

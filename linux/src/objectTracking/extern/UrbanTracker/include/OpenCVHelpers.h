#ifndef OPENCV_HELPERS_H
#define OPENCV_HELPERS_H

#include <opencv2/opencv.hpp>
#include "BlobRect.h"
namespace Utils
{
namespace OpenCV
{	

	
	bool IsOutsideROI(const cv::Mat& mask, const cv::Rect_<float>& r);
	bool IsOutsideROI(const cv::Mat& mask, const cv::Point2f& p);
	bool IsPartOfForeground(const cv::Mat& mask, const cv::Point2f& p);
	bool IsPartOfForeground(const cv::Mat& mask, const cv::Point2f& p, unsigned int radius);
	unsigned int GetValue(const cv::Mat& mask, const cv::Point2f& p);
	unsigned int GetValue(const cv::Mat& mask, const cv::Point& p);
	bool overlap(const cv::Rect_<float>& A, const cv::Rect_<float>& B);
	BlobRect mergeBlobRect(const BlobRect& A, const BlobRect& B);
	cv::Rect_<float> mergeRect(const cv::Rect_<float>& A, const cv::Rect_<float>& B);
	cv::Point2f getCentroid(const cv::Rect_<float>& A);
	void getProjectedPoint(const cv::Mat& homography, const cv::Point2f& in, cv::Point2f& out);

	cv::Rect_<float> dilate(const cv::Rect_<float>& r, double dilatation);


}
}


#endif
#ifndef FEATURE_DETECTOR_EXTRACTOR_MATCHER_H
#define FEATURE_DETECTOR_EXTRACTOR_MATCHER_H

#undef min
#undef max
#include <opencv2/opencv.hpp>
#include "IFeatureDetectorExtractorMatcher.h"


class FeatureDetectorExtractorMatcher: public IFeatureDetectorExtractorMatcher
{
public:
	FeatureDetectorExtractorMatcher(cv::FeatureDetector* detector, cv::DescriptorExtractor* extractor, cv::DescriptorMatcher* matcher, float ratio);
	virtual ~FeatureDetectorExtractorMatcher();

	virtual void detect(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, cv::Mat& descriptors, const cv::Mat& mask);
	virtual void match(const cv::Mat& d1, const cv::Mat& d2, std::vector< cv::DMatch >& matches);
	virtual void match(const cv::Mat& d1, const cv::Mat& d2, std::vector< cv::DMatch >& matches, const cv::Mat& mask);
	virtual int getDescriptorSize() const { return mDescriptorSize;};
	virtual int getDescriptorType() const { return mDescriptorType;};
	virtual int getNoPointBorderSize() const { return 40;} //TODO: Unfixed

private:
	//OpenCV 2 Cookbook
	int ratioTest(std::vector<std::vector<cv::DMatch>>& matches, float ratio);
	 void symmetryTest(const std::vector<std::vector<cv::DMatch>>& matches1,
		                const std::vector<std::vector<cv::DMatch>>& matches2,
					    std::vector<cv::DMatch>& symMatches);
	cv::FeatureDetector* mDetector;
	cv::DescriptorExtractor* mExtractor;
	cv::DescriptorMatcher* mMatcher;
	float mMatchRatio;
	int mDescriptorSize;
	int mDescriptorType;
};




#endif
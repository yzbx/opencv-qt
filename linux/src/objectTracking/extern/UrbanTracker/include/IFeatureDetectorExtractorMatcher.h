#ifndef I_FEATURE_DETECTOR_MATCHER_EXTRACTOR_H
#define I_FEATURE_DETECTOR_MATCHER_EXTRACTOR_H

class IFeatureDetectorExtractorMatcher
{
public:

	virtual ~IFeatureDetectorExtractorMatcher() {}

	virtual void detect(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, cv::Mat& descriptors, const cv::Mat& mask) = 0;
	virtual void match(const cv::Mat& d1, const cv::Mat& d2, std::vector< cv::DMatch >& matches, const cv::Mat& mask) = 0;
	virtual void match(const cv::Mat& d1, const cv::Mat& d2, std::vector< cv::DMatch >& matches) = 0;
	virtual int getDescriptorSize() const = 0;
	virtual int getDescriptorType() const = 0;
	virtual int getNoPointBorderSize() const { return 0;} 
};

#endif
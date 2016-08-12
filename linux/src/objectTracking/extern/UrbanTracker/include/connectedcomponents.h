#include <opencv2/opencv.hpp>


namespace cv
{
	//! computes the connected components labeled image of boolean image I with 4 or 8 way connectivity - returns N, the total
	//number of labels [0, N-1] where 0 represents the background label.
	int connectedComponents(cv::Mat &L, const cv::Mat &I, int connectivity = 8);
	std::vector<unsigned int> getBlobArea(const cv::Mat& label, unsigned int nbLabel);
	int filterBlobByArea(cv::Mat& labelMat, unsigned int nbLabel, unsigned int areaSize);

}
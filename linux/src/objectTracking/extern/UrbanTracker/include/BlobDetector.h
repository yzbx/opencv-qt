#ifndef BLOB_DETECTOR_H
#define BLOB_DETECTOR_H

#include "ApplicationContext.h"
#include "IBGS.h"
#include <cvblob.h>

#include "BlobRect.h"


class BlobDetector
{
public:
	BlobDetector(ApplicationContext* appContext, const std::string& bgsType);
	~BlobDetector();
	void update(const cv::Mat& inputFrame);

	const cv::Mat& getBGSMask() const { return mCurrentBGSMask;}
	const cv::Mat& getLabelMask() const { return mLabelMask;}
	const std::map<int, BlobRect>& getBlobBoundingBoxMap() const {return mBlobIdRectMap;}
	IBGS* getBGS() { return mBackgroundSubstractor;}
private:
	void runCCA(const cv::Mat& inputFrame);
	void runBGS(const cv::Mat& inputFrame);
	void removeFrozenBlobs(const cv::Mat& inputLabels,cvb::CvBlobs& blobs, const cv::Mat& inputFrame, const cv::Rect& roi, std::set<int>& frozenBlobLabel);
	void getSmallBlobsLabels(unsigned int minAreaSz, cvb::CvBlobs& blobs, std::set<int>& outBlobToRemove);
	void removeBlobsFromMaskByLabel(const cv::Mat& labels, cvb::CvBlobs& blobs, cv::Mat& outmask, const cv::Rect& roi, std::set<int>& blobToRemove);

	std::map<int, BlobRect> mBlobIdRectMap;
	cv::Mat mCurrentBGSMask;
	cv::Mat mLabelMask;
	IBGS* mBackgroundSubstractor;
	bool mReplayBGS;
	ApplicationContext* mContext;
	cv::Mat mMask;
	cv::Mat mCurrentImage;
	cv::Mat mImageGray[2];
	int mCurrentFrameIdx;
	int mLastFrameIdx;
	

};

#endif 
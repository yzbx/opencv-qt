#include "BlobDetector.h"
#include "Logger.h"
#include "StringHelpers.h"
#include "BGSFactory.h"
//#include "PlaybackBGS.h"
#include <string>
#include <opencv2/opencv.hpp>
#include <algorithm>
#include "IBGSStatic.h"
#include "BlobRect.h"
BlobDetector::BlobDetector(ApplicationContext* appContext, const std::string& bgsType)
: mContext(appContext)
, mBackgroundSubstractor(nullptr)
, mReplayBGS(bgsType == "PlaybackBGS")
, mLastFrameIdx(0)
, mCurrentFrameIdx(1)
{
	BGSFactory bgsFactory;
	mBackgroundSubstractor =  bgsFactory.getBGSInstance(bgsType);
	mMask = mContext->getMask();

    //WARNING
//    if(mBackgroundSubstractor->getNbChannel() == 1 && mMask.channels() > 1)
//        cv::cvtColor(mMask, mMask, CV_BGR2GRAY);
//    else if(mBackgroundSubstractor->getNbChannel() == 3 && mMask.channels() == 1)
//        cv::cvtColor(mMask, mMask, CV_GRAY2BGR);
    if(mMask.channels()==1) cv::cvtColor(mMask, mMask, CV_GRAY2BGR);
//	if(mReplayBGS)
//	{
//		PlaybackBGS* bgs = dynamic_cast<PlaybackBGS*>(mBackgroundSubstractor);
//		if(bgs)
//		{
//			int pos = appContext->getTimestamp();
//			bgs->setCurrentFrameNumber(pos);
//		}
//		else
//			LOGASSERT(false, "PlaybackBGS should have PlaybackBGS type...");
//	}
}


BlobDetector::~BlobDetector()
{
	delete mBackgroundSubstractor;
}

void BlobDetector::update(const cv::Mat& inputFrame)
{
	std::swap(mLastFrameIdx, mCurrentFrameIdx);
	runBGS(inputFrame);
	runCCA(inputFrame);
}



void BlobDetector::removeFrozenBlobs(const cv::Mat& inputLabels, cvb::CvBlobs& blobs, const cv::Mat& inputFrame, const cv::Rect& roi, std::set<int>& blobToRemoveList)
{
	if(!blobs.empty())
	{
		IBGSStatic* staticBgs = dynamic_cast<IBGSStatic*>(mBackgroundSubstractor);
		if(staticBgs)
		{
			int camNoiseDev = mContext->getTrackerAlgorithm().camNoise;
			double minimumMovementDev = mContext->getTrackerAlgorithm().movingPixelRatio;

			std::map<int,int> labelNbChangedPixel;
			cv::Mat deltaImage = cv::abs(mImageGray[mLastFrameIdx]-mImageGray[mCurrentFrameIdx]);
			unsigned int rows = std::min(roi.height+roi.y, inputLabels.rows), cols = std::min(roi.width+roi.x,inputLabels.cols);

			for(unsigned int c = roi.x ; c < cols; ++c)
			{				
				for(unsigned int r = roi.y; r < rows; ++r)
				{		
					int val = inputLabels.ptr<unsigned int>(r)[c];
					if(val != 0)
					{
						int delta = deltaImage.ptr<unsigned char>(r)[c];
						if(delta > camNoiseDev)
						{
							++labelNbChangedPixel[val];
						}
					}
				}
			}

			std::set<int> frozenBlobsId;
			std::vector<cv::Rect> rectList;
			for(auto blobIt = blobs.begin(); blobIt != blobs.end(); ++blobIt)
			{
				if (frozenBlobsId.find(blobIt->first) == frozenBlobsId.end())
				{
					float ratio = (float)labelNbChangedPixel[blobIt->first]/(float)blobIt->second->area;
					LOGDEBUG("ratio:" << ratio << " Area: " << (float)blobIt->second->area << " Centroid:" << blobIt->second->centroid.x << ", " << blobIt->second->centroid.y);
					if(ratio < minimumMovementDev)
					{
						frozenBlobsId.insert(blobIt->first);
						rectList.push_back(cv::Rect(blobIt->second->minx, blobIt->second->miny, blobIt->second->maxx-blobIt->second->minx, blobIt->second->maxy-blobIt->second->miny));
					}
				}
			}
			if (!frozenBlobsId.empty())
			{
				staticBgs->removeBlobFromModel(inputLabels, frozenBlobsId, rectList, mCurrentImage);
			}
			blobToRemoveList.insert(frozenBlobsId.begin(), frozenBlobsId.end());
		}
		
	}
}


void BlobDetector::runBGS(const cv::Mat& inputFrame)
{
	if(mReplayBGS)
	{
        //WARNING
        cv::Mat img_background;
        mBackgroundSubstractor->process(inputFrame, mCurrentBGSMask,img_background);
	}
	else
	{
		
		//
		LOGASSERT(inputFrame.rows>0, "Image number of row should be > 0");
	
		//if(mBackgroundSubstractor->getNbChannel() == 1)


			
		//cv::GaussianBlur(mImageGray[mCurrentFrameIdx], mImageGray[mCurrentFrameIdx], cv::Size(5,5), 1.5);
		//std::cout << mImageGray[mCurrentFrameIdx].channels() << " " << mMask.channels() << std::endl; 


		
        //WARNING
//		if(mBackgroundSubstractor->getNbChannel() == 1)
        if(1==2)
		{
			cv::cvtColor(inputFrame.clone(), mImageGray[mCurrentFrameIdx], CV_BGR2GRAY);
			if(!mMask.empty())
				cv::min(mImageGray[mCurrentFrameIdx], mMask, mImageGray[mCurrentFrameIdx]);
            //WARNING
            cv::Mat img_background;
            mBackgroundSubstractor->process(mImageGray[mCurrentFrameIdx], mCurrentBGSMask,img_background);
			mCurrentImage = mImageGray[mCurrentFrameIdx];
		}
		else
		{
			mCurrentImage = inputFrame.clone();
			if(!mMask.empty())
				cv::min(mCurrentImage, mMask, mCurrentImage);
			cv::cvtColor(mCurrentImage, mImageGray[mCurrentFrameIdx], CV_BGR2GRAY);

			//cv::GaussianBlur(mImageGray[mCurrentFrameIdx], mImageGray[mCurrentFrameIdx], cv::Size(5,5), 1.5);
            //WARNING
            cv::Mat img_background;
            mBackgroundSubstractor->process(mCurrentImage, mCurrentBGSMask,img_background);
			
			//Fill holes
			/*cv::imshow("Mask1", mCurrentBGSMask);
			cv::Mat des;
			cv::bitwise_not(mCurrentBGSMask, des);
			std::vector<std::vector<cv::Point> > contours;
			cv::findContours(des, contours, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
			for( int i = 0; i< contours.size(); ++i )
				cv::drawContours(des, contours, i, 255, -1);
			cv::bitwise_not(des, mCurrentBGSMask);
			cv::imshow("Mask2", mCurrentBGSMask);
			cv::waitKey();*/


		}

		//cv::imshow("BGSRaw" , mCurrentBGSMask);
		//cv::dilate(mCurrentBGSMask, mCurrentBGSMask, cv::Mat(), cv::Point(-1,-1),4);
		//cv::erode(mCurrentBGSMask, mCurrentBGSMask, cv::Mat(), cv::Point(-1,-1),2);

		if(mContext->getRecordBGS())
		{
			std::stringstream ss;
			ss << mContext->getBGSPath() << std::setfill('0') << std::setw(8) << mContext->getTimestamp() << ".png";
			std::string filePath = ss.str();
			bool success = cv::imwrite(filePath, mCurrentBGSMask);
			if(!success)
				LOGERROR("Can't save " << filePath << " make sure the intermediary directory exist");
		}
	}
}

void BlobDetector::runCCA(const cv::Mat& inputFrame)
{
	mBlobIdRectMap.clear();
	IplImage iplGrayImg = mCurrentBGSMask;	
	IplImage* labelImg=cvCreateImage(cvGetSize(&iplGrayImg), IPL_DEPTH_LABEL, 1);
	cvb::CvBlobs blobs;	
	cvb::cvLabel(&iplGrayImg, labelImg, blobs);

	
	unsigned int xMin = inputFrame.cols;
	unsigned int yMin = inputFrame.rows;
	unsigned int xMax = 0;
	unsigned int yMax = 0;
	for(auto blobIt = blobs.begin(); blobIt != blobs.end(); ++blobIt)
	{
		xMin = std::min(xMin, blobIt->second->minx);
		yMin = std::min(yMin, blobIt->second->miny);
		xMax = std::max(xMax, blobIt->second->maxx);
		yMax = std::max(yMax, blobIt->second->maxy);
	}
	cv::Rect roi(xMin,yMin,xMax-xMin, yMax-yMin);
	
	cv::Mat labels(labelImg);

	std::set<int> blobToRemove;
	getSmallBlobsLabels(mContext->getTrackerAlgorithm().minimumBlobsize, blobs, blobToRemove);


	if(mContext->getTrackerAlgorithm().removeGhosts)
		removeFrozenBlobs(labels, blobs, inputFrame, roi, blobToRemove);

	
	removeBlobsFromMaskByLabel(labels, blobs, mLabelMask, roi, blobToRemove);
	

	for(auto it = blobs.begin(); it != blobs.end(); ++it)
	{
		cvb::CvBlob* b = (*it).second;
		auto targetRect = cv::Rect_<float>(b->minx, b->miny, b->maxx - b->minx, b->maxy - b->miny);
		const cv::Mat& subImage = labels(targetRect);

		if(blobToRemove.find(b->label) == blobToRemove.end())
		{
			//labels
			cv::Mat binaryBlob = cv::Mat::zeros(subImage.rows, subImage.cols, CV_8UC1);
			for(unsigned int c = 0; c < subImage.cols; ++c)
			{
				for(unsigned int r = 0; r < subImage.rows; ++r)
				{
					unsigned int value = subImage.ptr<unsigned int>(r)[c];

					if(value == it->first)
					{
						binaryBlob.ptr<unsigned char>(r)[c] = 1; 
					}
				}
			}
			mBlobIdRectMap[b->label] = BlobRect(targetRect, binaryBlob);
		}
	}

	cvReleaseImage(&labelImg);
	cvReleaseBlobs(blobs);
}

void BlobDetector::getSmallBlobsLabels(unsigned int minAreaSz, cvb::CvBlobs& blobs, std::set<int>& outBlobToRemove)
{
	for (auto blobIt = blobs.begin(); blobIt != blobs.end(); ++blobIt)
	{
		cvb::CvLabel label = (*blobIt).second->label;
		int area = (*blobIt).second->area;
		if (area <= minAreaSz)
			outBlobToRemove.insert(label);
	}
}

void BlobDetector::removeBlobsFromMaskByLabel(const cv::Mat& inputLabels, cvb::CvBlobs& blobs, cv::Mat& outmask, const cv::Rect& roi, std::set<int>& blobToRemove)
{
	mCurrentBGSMask = cv::Mat::zeros(inputLabels.rows, inputLabels.cols, CV_8UC1);
	outmask = cv::Mat::zeros(inputLabels.rows, inputLabels.cols, CV_8UC1);
	blobToRemove.insert(0); //Used outside blobs
	blobToRemove.insert(-1); //Used on edge
	if (!blobs.empty())
	{
		unsigned int rows = std::min(roi.height + roi.y, inputLabels.rows), cols = std::min(roi.width + roi.x, inputLabels.cols);
		for (unsigned int c = roi.x; c < cols; ++c)
		{
			for (unsigned int r = roi.y; r < rows; ++r)
			{
				int val = inputLabels.ptr<unsigned int>(r)[c];
				if (blobToRemove.find(val) == blobToRemove.end())
				{
					outmask.ptr<unsigned char>(r)[c] = val;
					mCurrentBGSMask.ptr<unsigned char>(r)[c] = 255;
				}
			}
		}
	}
}

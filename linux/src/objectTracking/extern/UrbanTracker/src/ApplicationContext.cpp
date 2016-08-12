#include "ApplicationContext.h"
#include "Tracker.h"
#include "StringHelpers.h"
#include "Logger.h"
#include "InputFrameProviderIface.h"


ApplicationContext::ApplicationContext(const cv::Mat& mask, const cv::Mat& homography, float fps, float pixelToMeters, DrawingFlags drawingFlags, BlobTrackerAlgorithmParams algoParams, bool recordBGS, InputFrameProviderIface* frameProvider)
: mMask(mask)
, mHomography(homography)
, mInverseHomography(homography.inv())
, mFPS(fps)
, mSecondByFrame(1/fps)
, mMetersByPixel(1/pixelToMeters)
, mPixelToMeters(pixelToMeters)
, mTracker(nullptr)
, mObjectDatabaseManager(nullptr)
, mDrawingFlags(drawingFlags)
, mCurrentId(0)
, mTrackerParams(algoParams)
, mRecordingBGS(recordBGS)
, mFrameProvider(frameProvider)
, mBGSPath("bgs/")
{
	
}

int ApplicationContext::getTimestamp()
{ 
	return mFrameProvider->getNextFramePos();
}

ApplicationContext::~ApplicationContext()
{

}


std::string ApplicationContext::getUniqueId()
{
	return Utils::String::toString(mCurrentId++);
}



Tracker* ApplicationContext::getTracker()
{
	LOGASSERT(mTracker, "Tracker not initialised");
	return mTracker;
}



TrackerPersistance* ApplicationContext::getObjectDatabaseManager() 
{
	LOGASSERT(mObjectDatabaseManager, "Object database manager not initialised");
	return mObjectDatabaseManager;
}
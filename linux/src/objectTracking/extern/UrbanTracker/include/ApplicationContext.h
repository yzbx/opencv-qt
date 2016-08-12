#ifndef APPLICATION_CONTEXT_H
#define APPLICATION_CONTEXT_H

#include <opencv2/opencv.hpp>
#include "DrawingFlags.h"
#include "BlobTrackerAlgorithmParams.h"
#include <string>
class Tracker;
class TrackerPersistance;
class InputFrameProviderIface;

class ApplicationContext
{
public:

	ApplicationContext(const cv::Mat& mask, const cv::Mat& homography, float fps, float pixelToMeters, DrawingFlags drawingFlags, BlobTrackerAlgorithmParams algoParams, bool recordBGS, InputFrameProviderIface* frameProvider);
	~ApplicationContext();

	const BlobTrackerAlgorithmParams& getTrackerAlgorithm() { return mTrackerParams;}
	const cv::Mat& getPerspToPlaneMatrix() const { return mHomography;}
	const cv::Mat& getPlaneToPerspMatrix() const { return mInverseHomography;}
	const cv::Mat& getMask() const { return mMask; }
	const DrawingFlags* getDrawingFlags() const { return &mDrawingFlags;}
	DrawingFlags* getRefDrawingFlags() { return &mDrawingFlags;}
	float getFPS() const {return mFPS;}
	float getNbMeterByPixel() const { return mMetersByPixel;}
	float getNbPixelByMeter() const { return mPixelToMeters;}
	float getNbSecondByFrame() const { return mSecondByFrame;}
	std::string getUniqueId();
	Tracker* getTracker();
	void setTracker(Tracker* tracker) { mTracker = tracker;}
	void setObjectDatabaseManager(TrackerPersistance* objectSQLManager) { mObjectDatabaseManager = objectSQLManager;}
	TrackerPersistance* getObjectDatabaseManager();
	bool getRecordBGS() { return mRecordingBGS;}
	int getTimestamp(); 
	const cv::Mat& getCurrentFrame() { return mCurrentFrame;};
	void setCurrentFrame(const cv::Mat& frame){ mCurrentFrame = frame;}
	void setBGSPath(const std::string& path)  { mBGSPath = path;}
	const std::string& getBGSPath() const { return mBGSPath;}
private:
	const cv::Mat mInverseHomography;
	const cv::Mat mHomography;    /*The homography used to make the link between perspective and aerial view*/
	const cv::Mat mMask; /*The mask use on the original video */
	const float mFPS;   /*The number of frame by second of the video*/   
	const float mMetersByPixel; /*The number of meter to represent one pixel in the projected view*/
	const float mPixelToMeters; /*The number of pixel to represent one meter in the projected view*/
	const float mSecondByFrame; /*The number of second that last one frame*/
	DrawingFlags mDrawingFlags;
	int mCurrentId;
	TrackerPersistance* mObjectDatabaseManager; /*A ptr to the database system for persistance of objects*/
	std::string mBGSPath;
	BlobTrackerAlgorithmParams mTrackerParams;
	bool mRecordingBGS;

	InputFrameProviderIface* mFrameProvider;
	Tracker* mTracker;     /*A ptr to the tracker that will extract the individual trajectory*/
	cv::Mat mCurrentFrame;


};
#endif

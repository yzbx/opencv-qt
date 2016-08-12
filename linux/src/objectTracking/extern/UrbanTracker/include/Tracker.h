#ifndef TRACKER_H
#define TRACKER_H

#include <opencv2/opencv.hpp>
#include "PointsBlob.h"
#include "FrameAssociation.h"
#include "ObjectState.h"
#include <list>

#include "BlobRect.h"
class IFeatureDetectorExtractorMatcher;
class BlobDetector;
class ApplicationContext;
class IBGS;
class IObject;
class Object;


class Tracker
{
public:
	Tracker(ApplicationContext* c, const std::string& bgsType);
	~Tracker();
	void processFrame(const cv::Mat& img);
	void draw(cv::Mat& img);
	IFeatureDetectorExtractorMatcher* getFeatureManager() { return mPointDetectors;}
	BlobDetector* getBlobDetector() {return mBlobExtractor;}
private:
	void calculateAssociation();
	void updateModel(const FrameAssociation& association, const cv::Mat& currentFrame);
	void handleNewObjects(const FrameAssociation& association, const cv::Mat& currentFrame);
	void handleOneToOne(const FrameAssociation& association, const cv::Mat& currentFrame);
	void handleSplits(const FrameAssociation& association, const cv::Mat& currentFrame);
	void handleMerge(const FrameAssociation& association, const cv::Mat& currentFrame);
	void saveGoodObjects(IObject* object);
	bool isGoodObject(Object* object);
	void mergeObjectWithSimilarStart(const std::list<IObject*>& enteringList, std::list<IObject*>& mergedList, int maxTimeDistance, int newBlobLabel);

	void updateState();
	std::list<IObject*> getObject(int state) const;
	std::list<IObject*> getObject(const std::list<IObject*>& objectList, int state) const;
	void updateBlobFrameObjectMapping(IObject* object, int blobLabel);
	void updateObjectReference(IObject* oldObject, IObject* newObject);
	void updateModelWithBlob(IObject* obj, int newBlobId, const cv::Mat& currentFrame);
	IFeatureDetectorExtractorMatcher* mPointDetectors;	 //TODO: Rename le type PointManager
	
	bool blobListOverlap(const std::list<std::pair<int, BlobRect>>& A, const std::list<std::pair<int, BlobRect>>& B);

	BlobDetector* mBlobExtractor;
	std::vector<cv::DMatch> mPointMatches;
	ApplicationContext* mContext;
	unsigned int mCurrentFrameIdx;
	unsigned int mLastFrameIdx;
	std::list<IObject*> mObjectList;
	PointsBlob mPointBlobAssociation[2];
	std::map<int, BlobRect> mIdBoundingBoxMap[2];
	std::map<int, IObject*> mBlobLabelToObject[2];
	std::map<IObject*, std::set<int>> mObjectToBlobLabel[2];
	std::map<IObject*, IObject*> mReplacementObject;
};


#endif
#ifndef OBJECT_MODEL_GROUP_H
#define OBJECT_MODEL_GROUP_H

#include "IObjectModel.h"
#include <vector>
#include <opencv2/opencv.hpp>
#include <list>
#include <memory>
#include "matrix.h"
#include "Logger.h"
class IObject;
class ObjectModel;
class ObjectGroup;
class ApplicationContext;

class ObjectModelGroup : public IObjectModel
{
public:
	ObjectModelGroup(ApplicationContext* c, ObjectGroup* objectGroup);
	virtual ~ObjectModelGroup();
	virtual int getFirstTimestamp() const;
	virtual int getLastTimestamp() const;
	virtual cv::Rect_<float> getLastBoundingBox() const;

	virtual void addPoint(int timestamp, const std::vector<FeaturePoint>& featurePointList, const cv::Mat& descriptorList);
	virtual void addAndMatchPoint(int timestamp, const std::vector<FeaturePoint>& featurePointList, const cv::Mat& descriptorList);
	virtual const std::vector<Track*>& getTracks();
	virtual const std::map<int, Blob>& getBlobs();
	virtual void handleLeaving(std::vector<IObject*>& newObject) {LOGWARNING("This should not be called at this time");} 
	virtual void addTrack(Track* t);
	virtual void addMergeBlob(int timestamp, const Blob& b);
	virtual void replaceBlob(int timestamp, const Blob& b);
	virtual void clearObjectModel();
	virtual void clearObjectModelAndDelete();
	virtual void moveObjectModel(IObjectModel* model);

	virtual void addBlobs(const std::map<int, Blob>& blobs);
	virtual void addAndMatchTracks(const std::vector<Track*>& tracks); 

	virtual void handleSplit(const std::vector<ObjectModel*>& modelList, std::list<IObject*>& trackerObjectList, std::vector<std::pair<IObject*, ObjectModel*>>& association);
	virtual void addUnmatchedGroupBlobToExistingObjects();
	void simplifyModel();
	void addObjectModel(ObjectModel* om);
	void removeObjectModel(ObjectModel* om);
	ObjectGroup* getLinkedObject() { return mLinkedGroup;}
	virtual bool hasMinimalMovement(int minimalMovement) const;
private:
	virtual void addUnmatchedGroupBlobToExistingObjects(const std::list<ObjectModel*>& groupObjectList);
	void updateTrackList();
	ObjectModel* mModelGroup;
	std::list<ObjectModel*> mObjectModelListNonOwner;
	ObjectGroup* mLinkedGroup;
	bool mTrackListDirty;
	std::vector<Track*> mFullTrackList;
	std::map<Track*, ObjectModel*> mTrackToModel;
	std::map<int, Blob> mFullBlobList;
};

#endif 
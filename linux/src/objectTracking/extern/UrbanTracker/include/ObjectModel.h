#ifndef OBJECT_MODEL_H
#define OBJECT_MODEL_H

#include <vector>
#include "Track.h"
#include "Blob.h"

#include "FeaturePoint.h"
#include "IObjectModel.h"

class ApplicationContext;
class Object;


struct NormSort
{
   bool operator () (const cv::Point2f & lhs, const cv::Point2f & rhs)
   {
	   return (lhs.x*lhs.x+lhs.y*lhs.y) < (rhs.x*rhs.x+rhs.y*rhs.y);
   }
};


struct BlobEstimation
{
	BlobEstimation(cv::Point2f centroid, float height, float width, bool partialObs, int temporalDistance)
	: mCentroid(centroid)
	, mPartialObservation(partialObs)
	, mHeight(height)
	, mWidth(width)
	, mTemporalDistance(temporalDistance)
	{

	}
	
	bool mPartialObservation;
	cv::Point2f mCentroid;
	float mHeight;
	float mWidth;
	int getTemporalTimestamp() const { return mTemporalDistance;}
private:
	int mTemporalDistance;
};


struct BlobEstimationSort
{
   // Surcharge de l'opérateur
  bool operator () (const BlobEstimation& lhs, const BlobEstimation& rhs)
   {
      return lhs.getTemporalTimestamp() < rhs.getTemporalTimestamp();
   }
};



class ObjectModel: public IObjectModel
{
public:
	ObjectModel(ApplicationContext* context, Object* object);
	virtual ~ObjectModel();
	
	virtual int getFirstTimestamp() const;
	virtual int getLastTimestamp() const;
	virtual cv::Rect_<float> getLastBoundingBox() const;
	cv::Rect_<float> getLastStableBoundingBox() const;

	virtual void updateInGroupBlobs();
	void correctGroupObservation();

	virtual void addPoint(int timestamp, const std::vector<FeaturePoint>& featurePointList, const cv::Mat& descriptorList);
	virtual void addAndMatchPoint(int timestamp, const std::vector<FeaturePoint>& featurePointList, const cv::Mat& descriptorList);
	virtual void addTrack(Track* t);
	virtual const std::vector<Track*>& getTracks() { return mTracks;}
	void removeTracksNoDelete(const std::set<Track*>& tracksToRemove);
	virtual const std::map<int, Blob>& getBlobs() const{ return mBlobs;}
	virtual std::map<int, Blob>& getBlobs() { return mBlobs;}
	virtual void clearObjectModel();
	virtual void clearObjectModelAndDelete();
	
	virtual void addMergeBlob(int timestamp, const Blob& b);
	virtual void replaceBlob(int timestamp, const Blob& b);
	virtual void moveObjectModel(IObjectModel* model);

	
	void split(const std::vector<ObjectModel*>& modelList);
	virtual void simplifyModel();
	void extractObjectModel(ObjectModel* B, ApplicationContext* context);
	void interpolateMissingFrame(unsigned int maximumGap = -1);
	static void getMatches(IObjectModel* A, IObjectModel* B, ApplicationContext* context, std::vector<cv::DMatch>& matches);
	static int getMatchingPointNumber(IObjectModel* A, IObjectModel* B, ApplicationContext* context);
	static std::vector<double> getMatchingPointMovement(IObjectModel* A, IObjectModel* B, ApplicationContext* context);
	static bool isSimilar(IObjectModel* A, IObjectModel* B, ApplicationContext* context); 
	virtual bool hasMinimalMovement(int minimalMovement) const;
	int getNumberRemovedFeatures() const { return mNumberFeatures;}
	virtual void handleLeaving(std::vector<IObject*>& newObject);

	void addBlobs(const std::map<int, Blob>& blobs);
	void addAndMatchTracks(const std::vector<Track*>& tracks); 
	bool getExpectedPosition(int timestamp, cv::Point2f& expectedPosition);
	bool getProjectedSpeed(int timestamp, cv::Point2f& speed);
	Object* getLinkedObject() { return mLinkedObject;}
private:
	std::map<int, Blob> mBlobs; //Timestamp to blob
	std::vector<Track*> mTracks;

	Object* mLinkedObject;
	int mNumberFeatures;
};


#endif
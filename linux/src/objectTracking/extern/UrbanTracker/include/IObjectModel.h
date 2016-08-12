#ifndef IOBJECT_MODEL_H
#define IOBJECT_MODEL_H

class ApplicationContext;
#include <opencv2/opencv.hpp>
#include "FeaturePoint.h"
#include "Blob.h"
#include "Logger.h"
class Track;

class IObject;

class IObjectModel
{
public:
	IObjectModel(ApplicationContext* context)
		: mContext(context){}
	virtual ~IObjectModel(){}



	ApplicationContext* getContext() { return mContext;}
	

	virtual void simplifyModel() = 0;
	virtual int getFirstTimestamp() const = 0;
	virtual int getLastTimestamp() const = 0;
	virtual cv::Rect_<float> getLastBoundingBox() const = 0;
	
	virtual void handleLeaving(std::vector<IObject*>& newObject) = 0; 

	
	virtual void addPoint(int timestamp, const std::vector<FeaturePoint>& featurePointList, const cv::Mat& descriptorList) = 0;
	virtual void addAndMatchPoint(int timestamp, const std::vector<FeaturePoint>& featurePointList, const cv::Mat& descriptorList) = 0;
	virtual void addTrack(Track* t) = 0;
	virtual void clearObjectModel() = 0;
	virtual void clearObjectModelAndDelete() = 0;
	virtual const std::vector<Track*>& getTracks() = 0;
	virtual const std::map<int, Blob>& getBlobs() = 0;
	virtual void addMergeBlob(int timestamp, const Blob& b) = 0;
	virtual void replaceBlob(int timestamp, const Blob& b) = 0;
	virtual void moveObjectModel(IObjectModel* model) = 0;

	virtual void addBlobs(const std::map<int, Blob>& blobs)= 0;
	virtual void addAndMatchTracks(const std::vector<Track*>& tracks)= 0; 
	
	
	
	virtual bool hasMinimalMovement(int minimalMovement) const = 0;

	virtual bool hasMinimalMovement(const std::map<int, Blob>& blobs, int minimalMovement) const
	{
		bool hasMinimalMovement = false;
		float dist = 0;
		if(blobs.size()>1)
		{
			cv::Point2f startPos = blobs.begin()->second.getCentroid();
			cv::Point2f endPos = blobs.rbegin()->second.getCentroid();
			cv::Point2f d = endPos-startPos;
			if(sqrt(d.x*d.x+d.y*d.y) > minimalMovement)
				hasMinimalMovement = true;
			else //We will verify the total movement in case we have someone entering and leaving at the same point of the scene
			{
				
				auto it = blobs.begin();
				auto it2 = it;
				++it2;
				while(it2 != blobs.end() && !hasMinimalMovement)
				{
					cv::Point2f startPos = it->second.getCentroid();
					cv::Point2f endPos = it2->second.getCentroid();
					cv::Point2f d = endPos-startPos;				
					dist+=sqrt(d.x*d.x+d.y*d.y);
					if(dist > minimalMovement)
						hasMinimalMovement = true;
					else
					{
						it = it2;
						++it2;
					}
				}
			}
			if(!hasMinimalMovement)
				LOGINFO("Only " << dist << "px");
			return hasMinimalMovement;
		}
		




				
		return hasMinimalMovement;
	}
	


private:
	ApplicationContext* mContext;
};

#endif
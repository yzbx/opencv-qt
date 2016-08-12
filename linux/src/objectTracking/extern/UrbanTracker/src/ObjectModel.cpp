#include "ObjectModel.h"
#include "ApplicationContext.h"
#include "Tracker.h"
#include "IFeatureDetectorExtractorMatcher.h"
#include "Logger.h"
#include "Object.h"
ObjectModel::ObjectModel(ApplicationContext* context, Object* object)
: IObjectModel(context)
, mLinkedObject(object) //This is most likely coming from unitilialized object. It should never be used here.
, mNumberFeatures(0)
{

}


ObjectModel::~ObjectModel()
{
	clearObjectModelAndDelete();
}

void ObjectModel::replaceBlob(int timestamp, const Blob& b)
{
	auto blobIt = mBlobs.find(timestamp);
	if(blobIt != mBlobs.end())
		blobIt->second = b;
	else
		mBlobs.insert(std::make_pair(timestamp, b));
}

void ObjectModel::addMergeBlob(int timestamp, const Blob& b)
{
	auto it = mBlobs.find(timestamp);
	if(it == mBlobs.end()) 
		mBlobs.insert(std::make_pair(timestamp, b));
	else
	{
		cv::Rect_<float> b1 = it->second.getBoundingBox();
		cv::Rect_<float> b2 = b.getBoundingBox();
		it->second.setBoundingBox(Utils::OpenCV::mergeRect(b1,b2));
	}
}
bool ObjectModel::hasMinimalMovement(int minimalMovement) const
{
	return IObjectModel::hasMinimalMovement(mBlobs, minimalMovement);
}

 int ObjectModel::getFirstTimestamp() const
{
	return mBlobs.empty() ? 0 : mBlobs.begin()->first; //Map sorté en ordre croissant
}

 int ObjectModel::getLastTimestamp() const
{
	return mBlobs.empty() ? 0 : mBlobs.rbegin()->first; 
}

 void ObjectModel::removeTracksNoDelete(const std::set<Track*>& tracksToRemove)
 {
	 for(auto it = mTracks.begin(); it != mTracks.end(); )
	 {
		 if(tracksToRemove.find(*it) != tracksToRemove.end())
		 {
			 it = mTracks.erase(it);

		 }
		 else
			 ++it;

	 }

 }

 cv::Rect_<float> ObjectModel::getLastBoundingBox() const
 {
	if(mBlobs.empty())
	{
		LOGERROR("No bounding box found. This should never happens.");
		assert(false);
		return cv::Rect(0,0,1,1);
	}
	return mBlobs.rbegin()->second.getBoundingBox();
}


cv::Rect_<float> ObjectModel::getLastStableBoundingBox() const
{
	std::vector<std::pair<int, cv::Rect>> areaList;
	int i=0;
	//We look in the last 10 box to try and find the most stable one
	for(auto it = mBlobs.rbegin(); it != mBlobs.rend() && i < 10; ++it, ++i) //PARAM
	{
		cv::Rect bb = (*it).second.getBoundingBox();
		areaList.push_back(std::make_pair((*it).second.getBoundingBox().area(), bb));
	}
	

	int lastStableBox = -1;
	bool found = false;
	for(int i = 0; i < areaList.size() && !found;  ++i)
	{
		found = true;
		float deltaSz = (0.15*areaList[i].first);
		for(int j = i+1; j < areaList.size() && (j-i) < 4 && found; ++j)
		{
			if((areaList[j].first-areaList[i].first) > deltaSz) //Variation			
				found = false;
			
		}
		if(found)
		{
			lastStableBox = i;
		}
	}
	if(lastStableBox == -1)
	{
		LOGINFO("No stable box...");
		lastStableBox = 0;
	}
	cv::Rect_<float> r = areaList[lastStableBox].second;
	//std::cout << lastStableBox << " selected " << r.width << " " << r.height << std::endl;
	return r;

}

void ObjectModel::addTrack(Track* t)
{
	mTracks.push_back(t);
}


void ObjectModel::split(const std::vector<ObjectModel*>& modelList)
{
	std::set<Track*> trackToRemove;

	//1) We sort the existing tracks by timestamp
	std::map<int, std::vector<Track*> > lastTimestampToTracks;
	LOGINFO(mTracks.size() << " tracks to split objectModel ");	

	std::cout << "Extracting model...";
	for(int m = 0; m < modelList.size(); ++m)		
		extractObjectModel(modelList[m], getContext());
	std::cout << "Extracting model...Done";


	for(auto trackIt = mTracks.begin(); trackIt != mTracks.end(); ++trackIt)
	{		
		Track* t = (*trackIt);
		lastTimestampToTracks[t->getLastTimestamp()].push_back(t);
	}

	//Calcul des points par modèle
	std::map<int, std::vector<std::vector<cv::Point2f>>> timestampToPointByModel;
	for(int m = 0; m < modelList.size(); ++m)
	{
		const auto& trackList = modelList[m]->getTracks();
		for(auto t = trackList.begin(); t != trackList.end(); ++t)
		{
			const auto& pointList = (*t)->getPointList();
			for(auto p = pointList.begin(); p != pointList.end(); ++p)
			{
				auto timestampIt = timestampToPointByModel.find(p->first);
				if(timestampIt == timestampToPointByModel.end())
				{
					timestampToPointByModel.insert(std::make_pair(p->first,std::vector<std::vector<cv::Point2f>>(modelList.size(), std::vector<cv::Point2f>())));
				}
				timestampToPointByModel[p->first] [m].push_back(p->second.getPoint()); 
			}
		}
	}


	
	for(auto trackIt = lastTimestampToTracks.rbegin(); trackIt != lastTimestampToTracks.rend(); ++trackIt)
	{
		int timestamp = trackIt->first;
		
		const std::vector<std::vector<cv::Point2f>>& pointsByObject = timestampToPointByModel[timestamp];
		std::vector<cv::Rect_<float>> rectList;
		for(int m = 0; m < pointsByObject.size(); ++m)
		{
			const std::vector<cv::Point2f>& points = pointsByObject[m];
			if(!points.empty())
			{
				float x1  = points[0].x;
				float y1  = points[0].y;
				float x2  = points[0].x;
				float y2  = points[0].y;

				for(unsigned int p = 1; p < points.size(); ++p)
				{
					x1 = x1 > points[p].x ? points[p].x : x1;
					y1 = y1 > points[p].y ? points[p].y : y1;
					x2 = x2 < points[p].x ? points[p].x : x2;
					y2 = y2 < points[p].y ? points[p].y : y2;
				}
				cv::Rect_<float> minBB = cv::Rect_<float>(x1,y1, x2-x1, y2-y1);
				std::cout << minBB.x << " " << minBB.y << " " << minBB.width << " " << minBB.height << std::endl;
				if(minBB.width != 0 && minBB.height != 0)
				{
					if(modelList[m]->getBlobs().find(timestamp) == modelList[m]->getBlobs().end())	
					{
						auto blobIt = mBlobs.find(timestamp);
						if(blobIt != mBlobs.end())
						{
							Blob b(blobIt->second.getBoundingBox(), INGROUP, getContext());
							modelList[m]->addMergeBlob(timestamp, b);
						}
					}
					rectList.push_back(minBB);
				}
				else
					rectList.push_back(cv::Rect_<float>(0,0,0,0));
				
			}
			else
				rectList.push_back(cv::Rect_<float>(0,0,0,0));
		}
		//std::cout << "Before intersection";
		std::vector<bool> intersectionList;
		for(int r1 = 0; r1 < rectList.size(); ++r1)
		{
			bool noIntersection = true;
			for(int r2 = r1+1; r2 < rectList.size(); ++r2)
			{
				cv::Rect_<float> intersect = rectList[r1] & rectList[r2];
				if(intersect.width != 0)
					noIntersection = false;
			}
			intersectionList.push_back(!noIntersection);
		}
		//std::cout << "Before rect";
		for(int r1 = 0; r1 < rectList.size(); ++r1)
		{
		//	std::cout << "r1" <<  r1 << "/" << rectList.size() << std::endl;
			if(intersectionList[r1] == false)
			{
				//We can add all the track inside !
				int nbTracks = 0;
				const std::vector<Track*>& trackList = trackIt->second;
				for(int t = 0; t < trackList.size(); ++t)
				{
			//		std::cout << "t" << t << "/" << trackList.size()  << std::endl;
					if(trackList[t]->getPointList().size() > 1 && trackToRemove.find(trackList[t]) == trackToRemove.end())
					{
						auto& pointList = trackList[t]->getPointList();
						//std::cout << "BeforeC" << rectList[r1].width << " " << rectList[r1].height << " "  << pointList[timestamp].getPoint().x << " " << pointList[timestamp].getPoint().y;

						auto pIt = pointList.find(timestamp);
						if(pIt != pointList.end())
						{
							const cv::Point2f& ptPos = pIt->second.getPoint();
							if(rectList[r1].width > 0 && rectList[r1].height>0 && rectList[r1].contains(ptPos))
							{
								//std::cout << "AfterC";
								modelList[r1]->addTrack(trackList[t]);
								int p = 0;
								for(auto pIt = pointList.begin(); pIt != pointList.end(); ++pIt)
								{
									//std::cout << "p" << p << "/" << pointList.size() << std::endl;
									auto timestampIt = timestampToPointByModel.find(timestamp);
									if(timestampIt == timestampToPointByModel.end())
									{
										timestampToPointByModel.insert(std::make_pair(timestamp,std::vector<std::vector<cv::Point2f>>(modelList.size(), std::vector<cv::Point2f>())));
									}
									timestampToPointByModel[timestamp] [r1].push_back(pIt->second.getPoint()); 
									++p;
								}

								trackToRemove.insert(trackList[t]);
								++nbTracks;
							}
						}
						else
							std::cout << "Timestamp invalid !\n";
					}
				}
				std::cout << "Adding " << nbTracks << " tracks for " << timestamp << std::endl;
			}			
		}
	}
	//std::cout << "Wtf BBQ" << std::endl;
	std::vector<Track*> newTrackList;
	for(auto trackIt = mTracks.begin(); trackIt != mTracks.end(); ++trackIt)
	{	
		if(trackToRemove.find(*trackIt) == trackToRemove.end())
			newTrackList.push_back(*trackIt);
		//else
		//	delete *trackIt;
	}
	mTracks = newTrackList;
	for(int m = 0; m < modelList.size(); ++m)
	{
		std::cout << modelList[m]->getTracks().size() << " tracks in object " << std::endl;
	}
}

void ObjectModel::correctGroupObservation()
{
	for(auto blobIt = mBlobs.begin(); blobIt != mBlobs.end(); )
	{
		if( blobIt->second.getState() & (INGROUP|OBJECTGROUP))	
		{			
			blobIt = mBlobs.erase(blobIt);
			LOGINFO("Deleting blob " << blobIt->first << " object " << this->getLinkedObject()->getObjectId());
		}
		else
			++blobIt;
	}
}

void ObjectModel::interpolateMissingFrame(unsigned int maximumGap)
{
	std::vector<std::pair<int, Blob>> interpolatedBlobList; 
	if(mBlobs.size() > 1)
	{
		auto it = mBlobs.begin();
		const Blob*  previousBlob = &it->second;
		int previousTimestamp = it->first;
		++it;
		const Blob* currentBlob = &it->second;
		for(; it != mBlobs.end(); ++it)
		{
			currentBlob = &it->second;
			int currentTimestamp = it->first;
			int deltaTimestamp = currentTimestamp-previousTimestamp;
			if(deltaTimestamp > 1 && deltaTimestamp <= maximumGap)
			{
				const cv::Rect_<float>& previousRect = previousBlob->getBoundingBox();
				const cv::Rect_<float>& currentRect = currentBlob->getBoundingBox();
							
				for(int t = previousTimestamp+1; t < currentTimestamp; ++t)
				{
					float ratio = ((float)t-previousTimestamp)/deltaTimestamp;
					float ratioStart = 1-ratio;
					float ratioEnd = ratio;
					int x = ratioStart*previousRect.x+ratioEnd*currentRect.x;
					int y = ratioStart*previousRect.y+ratioEnd*currentRect.y;
					int w = ratioStart*previousRect.width+ratioEnd*currentRect.width;
					int h = ratioStart*previousRect.height+ratioEnd*currentRect.height;
					interpolatedBlobList.push_back(std::make_pair(t, Blob(cv::Rect_<float>(x,y, w,h), currentBlob->getState(), getContext())));
				}
			}
			previousTimestamp = currentTimestamp;
			previousBlob = &it->second;
		}
		if(interpolatedBlobList.size() > 0)
		{
			LOGINFO(interpolatedBlobList.size() << " bounding boxes were interpolated for " << getLinkedObject()->getObjectId() << " by interpolateMissingFrame()");
			
			mBlobs.insert(interpolatedBlobList.begin(), interpolatedBlobList.end());
		}
	}
}

void ObjectModel::extractObjectModel(ObjectModel* B, ApplicationContext* context)
{
	ObjectModel* A = this;
	IFeatureDetectorExtractorMatcher* featureManager = context->getTracker()->getFeatureManager();
	const std::vector<Track*>& tracksA = A->getTracks();
	const std::vector<Track*>& tracksB = B->getTracks();
	if(!tracksA.empty() && !tracksB.empty())
	{
		cv::Mat descriptorsA(tracksA.size(), featureManager->getDescriptorSize(), featureManager->getDescriptorType());
		cv::Mat descriptorsB(tracksB.size(), featureManager->getDescriptorSize(), featureManager->getDescriptorType());
		for(unsigned int i = 0; i < tracksA.size(); ++i)		
			tracksA[i]->getDescriptor().row(0).copyTo(descriptorsA.row(i));
		for(unsigned int i = 0; i < tracksB.size(); ++i)		
			tracksB[i]->getDescriptor().row(0).copyTo(descriptorsB.row(i));

		std::vector<cv::DMatch> matches;
		featureManager->match(descriptorsA, descriptorsB, matches);

		std::set<Track*> trackToMove;

		std::set<int> timestampToUpdate;

		for(auto it = matches.begin(); it != matches.end(); ++it)		
		{
			Track* at = tracksA[it->queryIdx];
			Track* bt = tracksB[it->trainIdx];
			const std::map<int, FeaturePoint>& aPointList = at->getPointList();
			std::map<int, FeaturePoint>& bPointList = bt->getPointList();
			bPointList.insert(aPointList.begin(), aPointList.end());			
			trackToMove.insert(at);

			for(auto pIt =aPointList.begin(); pIt != aPointList.end(); ++pIt)
				timestampToUpdate.insert(pIt->first);
		}
		std::map<int, Blob>& aBlob = A->getBlobs();
		std::map<int, Blob>& bBlob = B->getBlobs();
		for(auto it = timestampToUpdate.begin(); it != timestampToUpdate.end(); ++it)		
		{
			auto itA = aBlob.find(*it);
			if(itA != aBlob.end())
			{
				itA->second.setState(INGROUP);
				bBlob.insert(*itA);
			}
		}

		
		std::vector<Track*> newA;
		for(auto it = tracksA.begin(); it != tracksA.end(); ++it)
		{
			Track* t = *it;
			auto ta = trackToMove.find(t);
			if(ta == trackToMove.end())
				newA.push_back(t);
			else
				delete t;
		}
		mTracks = newA;
	}
}


void ObjectModel::addAndMatchTracks(const std::vector<Track*>& tracksB)
{
	std::set<int> usedTracks;
	//Merge Tracks
	const std::vector<Track*>& tracksA = mTracks;
	if(!tracksA.empty() && !tracksB.empty())
	{
		IFeatureDetectorExtractorMatcher* featureManager = getContext()->getTracker()->getFeatureManager();
		cv::Mat descriptorsA(tracksA.size(), featureManager->getDescriptorSize(), featureManager->getDescriptorType());
		cv::Mat descriptorsB(tracksB.size(), featureManager->getDescriptorSize(), featureManager->getDescriptorType());
		for(unsigned int i = 0; i < tracksA.size(); ++i)		
			tracksA[i]->getDescriptor().row(0).copyTo(descriptorsA.row(i));
		for(unsigned int i = 0; i < tracksB.size(); ++i)		
			tracksB[i]->getDescriptor().row(0).copyTo(descriptorsB.row(i));

		std::vector<cv::DMatch> matches;
		featureManager->match(descriptorsA, descriptorsB, matches);
		
		for(unsigned int matchIdx = 0; matchIdx != matches.size(); ++matchIdx)
		{
			int trackAIdx = matches[matchIdx].queryIdx;
			int trackBIdx = matches[matchIdx].trainIdx;
			Track* A = tracksA[trackAIdx];
			Track* B = tracksB[trackBIdx];
			if(A->getLastTimestamp() < B->getFirstTimestamp() || B->getLastTimestamp() < A->getFirstTimestamp())
			{
				std::map<int, FeaturePoint>& pointListA = A->getPointList();
				pointListA.insert(B->getPointList().begin(), B->getPointList().end());
				usedTracks.insert(trackBIdx);
				delete B;
			}
			else
			{
				//We ignore this track
			}
		}
	}
	for(unsigned int i = 0; i < tracksB.size(); ++i)
	{
		if(usedTracks.find(i) == usedTracks.end())
		{				
			Track* B = tracksB[i];
			mTracks.push_back(B);
		}
	}
}


void ObjectModel::addBlobs(const std::map<int, Blob>& blobsB)
{
	std::map<int, Blob>& blobsA = mBlobs;

	int BlobTimeStartA = blobsA.empty() ? 0 : blobsA.begin()->first; 
	int BlobTimeEndA = blobsA.empty() ? 0 : blobsA.rbegin()->first; 

	int BlobTimeStartB = blobsB.empty() ? 0 : blobsB.begin()->first; 
	int BlobTimeEndB = blobsB.empty() ? 0 : blobsB.rbegin()->first; 
	if(BlobTimeEndA < BlobTimeStartB ||  BlobTimeEndB < BlobTimeStartA)
	{
		blobsA.insert(blobsB.begin(), blobsB.end());
	}	
	else
	{
		for(auto bIt = blobsB.begin(); bIt != blobsB.end(); ++bIt)
		{
			auto blobT = blobsA.find(bIt->first);
			if(blobT != blobsA.end())
			{
				blobT->second.setBoundingBox(Utils::OpenCV::mergeRect(blobT->second.getBoundingBox(), bIt->second.getBoundingBox()));
			}
			else
			{
				blobsA.insert(std::make_pair(bIt->first, bIt->second));
			}
		}
	}
}





void ObjectModel::moveObjectModel(IObjectModel* otherObject)
{
	const std::vector<Track*>& otherObjectTracks =  otherObject->getTracks();
	//Merge Tracks
	addAndMatchTracks(otherObjectTracks);
		
	//Merge blobs
	const std::map<int, Blob>& otherObjectBlob = otherObject->getBlobs();
	addBlobs(otherObjectBlob);
	

	otherObject->clearObjectModel();
}

void ObjectModel::clearObjectModel()
{
	mBlobs.clear();
	mTracks.clear();
}	

void ObjectModel::simplifyModel()
{
	//TODO: Exteriose parameters
	int numberOfFrameToKeep = 3;
	
	if(numberOfFrameToKeep != -1)
	{
		int minimumPointInTrack = 2; //TODO: Exteriose parameters


		int currentTimestamp = getContext()->getTimestamp();
		std::vector<Track*> newTracks;
		for(int i = 0; i < mTracks.size(); ++i)
		{
			bool oldTrack = currentTimestamp > (mTracks[i]->getLastTimestamp() + numberOfFrameToKeep);

			if(oldTrack && mTracks[i]->getPointList().size() < minimumPointInTrack)
			{
				mNumberFeatures+=mTracks[i]->getPointList().size();
				delete mTracks[i];
			}
			else
				newTracks.push_back(mTracks[i]);
		}
		/*if(mTracks.size() != newTracks.size())
		{
			//LOGINFO("removed" << mTracks.size()-newTracks.size() << " tracks !");
		}*/
		mTracks = newTracks;
	}
}

void ObjectModel::clearObjectModelAndDelete()
{
	for(unsigned int i = 0; i < mTracks.size(); ++i)
	{
		delete mTracks[i];
	}
	mTracks.clear();
	clearObjectModel();
}



void ObjectModel::addAndMatchPoint(int timestamp, const std::vector<FeaturePoint>& featurePointList, const cv::Mat& pointDescriptorList)
{
	std::set<int> newPointMatchedIdx;
	if(!mTracks.empty() && !featurePointList.empty())
	{
		IFeatureDetectorExtractorMatcher* featureManager = getContext()->getTracker()->getFeatureManager();
		
		cv::Mat currentTracksDesc(mTracks.size(), featureManager->getDescriptorSize(), featureManager->getDescriptorType());
		for(unsigned int i = 0; i < mTracks.size(); ++i)		
			mTracks[i]->getDescriptor().row(0).copyTo(currentTracksDesc.row(i));
		std::vector<cv::DMatch> matches;
		featureManager->match(currentTracksDesc, pointDescriptorList, matches);
		
		
		for(unsigned int matchIdx = 0; matchIdx != matches.size(); ++matchIdx)
		{
			int trackIdx = matches[matchIdx].queryIdx;
			int pointIdx = matches[matchIdx].trainIdx;

			newPointMatchedIdx.insert(pointIdx);
			mTracks[trackIdx]->addPoint(timestamp, featurePointList[pointIdx]);	
			mTracks[trackIdx]->updateDescriptor(pointDescriptorList.row(pointIdx));
		}
	}

	for(unsigned int i = 0; i < featurePointList.size(); ++i)
	{
		if(newPointMatchedIdx.find(i) == newPointMatchedIdx.end())
		{
			Track* t= new Track(pointDescriptorList.row(i));
			t->addPoint(timestamp, featurePointList[i]);
			mTracks.push_back(t);
		}
	}

}

void ObjectModel::addPoint(int timestamp, const std::vector<FeaturePoint>& featurePointList, const cv::Mat& pointDescriptorList)
{	
	for(unsigned int i = 0; i < featurePointList.size(); ++i)
	{
		Track* t= new Track(pointDescriptorList.row(i));
		t->addPoint(timestamp, featurePointList[i]);
		addTrack(t);
	}
}

void ObjectModel::getMatches(IObjectModel* A, IObjectModel* B, ApplicationContext* context, std::vector<cv::DMatch>& matches)
{
	IFeatureDetectorExtractorMatcher* featureManager = context->getTracker()->getFeatureManager();
	const std::vector<Track*>& tracksA = A->getTracks();
	const std::vector<Track*>& tracksB = B->getTracks();
	if(!tracksA.empty() && !tracksB.empty())
	{
		cv::Mat descriptorsA(tracksA.size(), featureManager->getDescriptorSize(), featureManager->getDescriptorType());
		cv::Mat descriptorsB(tracksB.size(), featureManager->getDescriptorSize(), featureManager->getDescriptorType());
		for(unsigned int i = 0; i < tracksA.size(); ++i)		
			tracksA[i]->getDescriptor().row(0).copyTo(descriptorsA.row(i));
		for(unsigned int i = 0; i < tracksB.size(); ++i)		
			tracksB[i]->getDescriptor().row(0).copyTo(descriptorsB.row(i));

		featureManager->match(descriptorsA, descriptorsB, matches);
	}
}

std::vector<double> ObjectModel::getMatchingPointMovement(IObjectModel* A, IObjectModel* B, ApplicationContext* context)
{
	std::vector<double> deltaMovement;
	std::vector<cv::DMatch> matches;
	getMatches(A,B,context,matches);
	const std::vector<Track*>& tracksA = A->getTracks();
	const std::vector<Track*>& tracksB = B->getTracks();
	for(unsigned int i = 0; i < matches.size(); ++i)
	{
		const auto& pointA = tracksA[matches[i].queryIdx]->getPointList();
		cv::Point2f p1 = (*pointA.rbegin()).second.getPoint();
		const auto& pointB = tracksB[matches[i].trainIdx]->getPointList();
		cv::Point2f p2 = (*pointB.rbegin()).second.getPoint();
		deltaMovement.push_back(cv::norm(p2-p1));
	}
	
	return deltaMovement;


}

int ObjectModel::getMatchingPointNumber(IObjectModel* A, IObjectModel* B, ApplicationContext* context)
{
	std::vector<cv::DMatch> matches;
	getMatches(A,B,context,matches);
	return matches.size();	
}


void ObjectModel::handleLeaving(std::vector<IObject*>& newObjectList)
{
	if(mBlobs.size() >1)
	{
		int lastGoodTimestamp = -1;
		auto it = mBlobs.rbegin();		
		int currentTimestamp = it->first;
		++it;
		auto currentBlobIt = mBlobs.rbegin();
		
		while( it !=mBlobs.rend() && lastGoodTimestamp == -1)
		{
			if(it->second.getState() != LEAVING)
				lastGoodTimestamp = it->first;
			++it;
		}
		LOGINFO(lastGoodTimestamp << " is the last good timestamp");
		if(lastGoodTimestamp != -1) 
		{
			std::set<Track*> compatibleTrack;
			
			for(auto tracksIt = mTracks.begin(); tracksIt != mTracks.end(); ++tracksIt)
			{
				if((*tracksIt)->getLastTimestamp() == currentTimestamp && (*tracksIt)->getFirstTimestamp() <=lastGoodTimestamp)
					compatibleTrack.insert(*tracksIt);
			}
			if(compatibleTrack.size() >= getContext()->getTrackerAlgorithm().minimumMatchBetweenBlob)
			{
				LOGINFO("This is the same object. We have " << compatibleTrack.size() << " compatible tracks.");
			}
			else
			{
				//We will try to find the moment the objet changed using the blob area	
				auto lastGoodTimestampIt = mBlobs.find(lastGoodTimestamp);
				int minArea = lastGoodTimestampIt->second.getBoundingBox().area();
				int minAreaTimestamp = lastGoodTimestamp;
				++lastGoodTimestampIt;
				for(;lastGoodTimestampIt != mBlobs.end(); ++lastGoodTimestampIt)
				{
					int tmpArea = lastGoodTimestampIt->second.getBoundingBox().area();
					//std::cout << lastGoodTimestampIt->first << " " << tmpArea << std::endl;
					if(tmpArea < minArea)
					{
						minArea = tmpArea;
						minAreaTimestamp = lastGoodTimestampIt->first;
					}
				}


				LOGINFO("This is a different object. We are going to split it. We have " << compatibleTrack.size() << " compatible tracks.");
				//1) Find which coordinate are outside ROI
				auto leavingBlob = ++mBlobs.find(lastGoodTimestamp);
				//const auto& boundingBox = leavingBlob->second.getBoundingBox();

				int bestTimestamp  = minAreaTimestamp;
				Object* newObject = new Object(getContext());
				newObject->setState(OBJECT);
				
				for(auto blobIt = mBlobs.find(bestTimestamp); blobIt != mBlobs.end() ; ++blobIt)
				{
					Blob b = blobIt->second;
					if(blobIt->first == currentBlobIt->first)
						b.setState(OBJECT);
					else
						b.setState(ENTERING);
					newObject->getIObjectModel()->addMergeBlob(blobIt->first,b);
				}

				//Let's take all object that start at bestTimestamp
				

				cv::Mat desc;
				std::vector<FeaturePoint> fpList;
				std::vector<Track*> newTrackList;
				for(auto trackIt = mTracks.begin(); trackIt != mTracks.end();)
				{
					if((*trackIt)->getFirstTimestamp() >= bestTimestamp)
					{
						newTrackList.push_back(*trackIt);
						trackIt = mTracks.erase(trackIt);
					}
					else
						++trackIt;

				}

				for(auto trackIt = newTrackList.begin(); trackIt != newTrackList.end(); ++trackIt)
				{					
					const auto& fp = (*trackIt)->getPointList().rbegin();
					fpList.push_back(fp->second);
					desc.push_back((*trackIt)->getDescriptor());
					delete *trackIt;
				}
				newObject->getIObjectModel()->addPoint(currentTimestamp, fpList, desc);
				newObjectList.push_back(newObject);



				for(auto blobIt = mBlobs.find(bestTimestamp); blobIt != mBlobs.end();)
				{
					blobIt = mBlobs.erase(blobIt);
				}
			}

		}
		else //Object was entering ? I don't expect this to happen...
		{
			LOGERROR("Only leaving blobs ? That should not be possible...");
			mLinkedObject->setState(OBJECT);
		}
	}
	else
		LOGERROR("There should be blobs at this point...");

}


bool ObjectModel::isSimilar(IObjectModel* A, IObjectModel* B, ApplicationContext* context)
{
	bool similarity = getMatchingPointNumber(A,B, context) >= context->getTrackerAlgorithm().minimumMatchBetweenBlob;	
	return similarity;
}

bool ObjectModel::getProjectedSpeed(int timestamp, cv::Point2f& outSpeed)
{
	bool success = false;
	std::vector<cv::Point2f> speedList;
	for(auto it = mTracks.begin(); it != mTracks.end(); ++it)
	{
		cv::Point2f speed;
		if((*it)->getProjectedSpeed(timestamp, speed))
		{
			speedList.push_back(speed);
		}
	}
	if(!speedList.empty())
	{
		std::sort(speedList.begin(), speedList.end(), NormSort());
		int midIdx = floor(speedList.size()/2.);	
		outSpeed = speedList[midIdx];
		success = true;
	}
	else
	{
		std::vector<std::pair<int,cv::Point2f>> positions;
		//We will use bounding box displacement
		for(auto blobIt = mBlobs.rbegin(); blobIt != mBlobs.rend() && positions.size() != 2; ++blobIt)
		{
			if(blobIt->second.getState() == OBJECT)
			{
				positions.push_back(std::make_pair(blobIt->first,blobIt->second.getProjectedCentroid()));
			}
		}
		if(positions.size() == 2)
		{
			int deltaTime = positions[0].first-positions[1].first;
			outSpeed = positions[0].second-positions[1].second;
			outSpeed.x /= deltaTime;
			outSpeed.y /= deltaTime;
			success = true;
		}
	}
	return success;
}



bool ObjectModel::getExpectedPosition(int timestamp, cv::Point2f& expectedPosition)
{
	cv::Point2f speed;
	bool success = !mBlobs.empty() && getProjectedSpeed(timestamp, speed);
	if(success)
	{
		success = false;
		int lastBlobTimestamp = -1;
		cv::Point2f lastBlobCentroid;
		for(auto blobIt = mBlobs.rbegin(); blobIt != mBlobs.rend() && lastBlobTimestamp == -1; ++blobIt)
		{
			if(blobIt->second.getState() == OBJECT)
			{
				lastBlobTimestamp = blobIt->first;
				lastBlobCentroid =  blobIt->second.getProjectedCentroid();
			}
		}
		if(lastBlobTimestamp != -1)
		{
			int deltaTime = timestamp - lastBlobTimestamp;
			cv::Point2f expectedProjectedPosition = lastBlobCentroid+deltaTime*speed;
			Utils::OpenCV::getProjectedPoint(getContext()->getPlaneToPerspMatrix(), expectedProjectedPosition,expectedPosition);
			success = true;
		}
	}	
	return success;	
}



void ObjectModel::updateInGroupBlobs()
{
	std::map<int, Blob*> blobToUpdate;
	std::map<int, std::pair<Blob*, bool>> goodBlobs; //bool indicate if less or more reliable (false are more reliable reliable)

	
	for(auto blobIt = mBlobs.begin(); blobIt != mBlobs.end(); ++blobIt)
	{
		Blob* b = &blobIt->second;
		if(b->getState() & (INGROUP|OBJECTGROUP))					
			blobToUpdate.insert(std::make_pair(blobIt->first, b));	
		else
		{
			bool partialObs = b->getState() & (ENTERING|LEAVING); 
			goodBlobs.insert(std::make_pair(blobIt->first, std::make_pair(b, partialObs)));	
		}
	}
	//Everything is sorted here since mBlobs is sorted by timestamp

	//1) We look for tracks
	if(!blobToUpdate.empty())
	{
		std::vector<Track*> interestTrack;
		int startTimestamp = blobToUpdate.begin()->first;
		int endTimestamp = blobToUpdate.rbegin()->first;
		for(auto trackIt = mTracks.begin(); trackIt != mTracks.end(); ++trackIt)
		{
			Track* t = *trackIt;
			if((t->getFirstTimestamp() <= startTimestamp && t->getLastTimestamp() >= startTimestamp) || //We will use previous observation
			   (t->getLastTimestamp() >= endTimestamp && t->getFirstTimestamp() <= endTimestamp))
			{
				interestTrack.push_back(t);
			}
		}
		//std::cout << "Update from" << startTimestamp << " " << endTimestamp << " Tracks " << mTracks.size() <<  std::endl;
		std::map<int, std::vector<BlobEstimation>> timestampToBlobApproximation;

		const cv::Mat& planeToPersp = getContext()->getPlaneToPerspMatrix();

		for(auto trackIt = interestTrack.begin(); trackIt != interestTrack.end(); ++trackIt)
		{
			Track* t = *trackIt;
			const std::map<int, FeaturePoint>& pointList = t->getPointList();
			bool validObs = false;
			int lastValidTimestamp;
			bool partialObs = false;
			cv::Point2f lastValidPosRelCenter; //Homography point
			float lastValidWidth = 0;
			float lastValidHeight = 0;
			std::vector<std::pair<int, cv::Point2f>> lastObs; //Observation that we need to go back on
			for(auto pointIt = pointList.begin(); pointIt != pointList.end(); ++pointIt)
			{
				int timestamp = pointIt->first;

				if(blobToUpdate.find(timestamp) != blobToUpdate.end() && validObs)
				{
					cv::Point2f projPt = pointIt->second.getProjectedPoint();
					if(partialObs || !validObs)
						lastObs.push_back(std::make_pair(timestamp, projPt));
					cv::Point2f estimatedProjCentroid = projPt-lastValidPosRelCenter;
					timestampToBlobApproximation[timestamp].push_back(BlobEstimation(estimatedProjCentroid,lastValidHeight, lastValidWidth, partialObs, abs(lastValidTimestamp-timestamp)));
				}
				else
				{
					auto goodBlotIt = goodBlobs.find(timestamp);
					if(goodBlotIt != goodBlobs.end())
					{
						bool newObsPartial = goodBlotIt->second.second;
						if(!validObs || !(partialObs == true && newObsPartial == false))
						{
							validObs = true;
							lastValidTimestamp = goodBlotIt->first;
							partialObs = newObsPartial;
							cv::Point2f centroid = goodBlotIt->second.first->getProjectedCentroid();
							cv::Point2f point = pointIt->second.getProjectedPoint();
							lastValidPosRelCenter = point-centroid;
							const auto& bb = goodBlotIt->second.first->getProjectedBoundingBox();
							lastValidWidth = bb.width;
							lastValidHeight = bb.height;
							//This is to help backward result if needed
							if(!lastObs.empty())
							{
								for(auto timestampIt = lastObs.begin(); timestampIt != lastObs.end(); ++timestampIt)
								{
									int t = timestampIt->first;
									cv::Point2f projPt = timestampIt->second;
									cv::Point2f estimatedProjCentroid = projPt+lastValidPosRelCenter;
									timestampToBlobApproximation[t].push_back(BlobEstimation(estimatedProjCentroid,lastValidHeight, lastValidWidth, partialObs, abs(lastValidTimestamp-t)));
								}
								lastObs.clear();
							}
						}
					}
					else if (!validObs)
					{
						cv::Point2f projPt = pointIt->second.getProjectedPoint();
						lastObs.push_back(std::make_pair(timestamp, projPt));
					}
				}
			}
		}
		for(auto blobApproxIt = timestampToBlobApproximation.begin(); blobApproxIt != timestampToBlobApproximation.end(); ++blobApproxIt)
		{
			int timestamp = blobApproxIt->first;
			const cv::Rect_<float>& groupBb = blobToUpdate[timestamp]->getBoundingBox();			
			std::vector<BlobEstimation>& estimationList = blobApproxIt->second;
			std::sort(estimationList.begin(), estimationList.end(), BlobEstimationSort());

			if(!estimationList.empty())
			{
				std::vector<int> xPos;
				std::vector<int> yPos;
				std::vector<int> width;
				std::vector<int> height;

				int containsFullEstimation = 0;
				for(auto estimationIt = estimationList.begin(); estimationIt != estimationList.end(); ++estimationIt)
				{
					containsFullEstimation += !(*estimationIt).mPartialObservation;
				}
				int nbEstimation = 0;
				//std::cout << estimationList.size() << " elements!";
				for(auto estimationIt = estimationList.begin(); estimationIt != estimationList.end() && nbEstimation < 10;  ++estimationIt)
				{
					if((!(*estimationIt).mPartialObservation) || (containsFullEstimation < 3))
					{
						xPos.push_back((*estimationIt).mCentroid.x);
						yPos.push_back((*estimationIt).mCentroid.y);
						width.push_back((*estimationIt).mWidth);
						height.push_back((*estimationIt).mHeight);
						++nbEstimation;
						//std::cout <<"ok " << (*estimationIt).getTemporalTimestamp() << " " <<(*estimationIt).mPartialObservation <<std::endl;
					}
					//else
						//std::cout << (*estimationIt).mPartialObservation << " " << containsFullEstimation  << " " << (*estimationIt).getTemporalTimestamp() << std::endl;
				}
				//std::cout << estimationList.size() << "GB: " << goodBlobs.size() << "IT:" << interestTrack.size() << std::endl;
				if(estimationList.size() >= 3)
				{
					//std::cout << nbEstimation << "/" << estimationList.size() << " used " << std::endl;
					int midIdx = floor(xPos.size()/2.f);
					std::sort(xPos.begin(), xPos.end());
					std::sort(yPos.begin(), yPos.end());
					
					
					
					std::sort(width.begin(), width.end());
					std::sort(height.begin(), height.end());
					cv::Point2f centroid = cv::Point2f(xPos[midIdx], yPos[midIdx]);
					float widthValue = width[midIdx];
					float heightValue = height[midIdx];

					std::vector<cv::Point2f> beforeHomography;
					std::vector<cv::Point2f> afterHomography;
					beforeHomography.push_back(centroid);
					beforeHomography.push_back(cv::Point2f(centroid.x-widthValue/2.f, centroid.y)); 
					beforeHomography.push_back(cv::Point2f(centroid.x+widthValue/2.f, centroid.y)); 
					beforeHomography.push_back(cv::Point2f(centroid.x, centroid.y-heightValue/2.f)); 
					beforeHomography.push_back(cv::Point2f(centroid.x, centroid.y+heightValue/2.f));

					//beforeHomography.push_back(cv::Point2f(centroid.x, centroid.y)); 
					//beforeHomography.push_back(cv::Point2f(centroid.x, centroid.y)); 



					cv::perspectiveTransform(beforeHomography,afterHomography, planeToPersp);
					cv::Point2f correctedCentroid = afterHomography[0];
					float x                       = afterHomography[1].x;
					float widthV                  = afterHomography[2].x-x;
					float y                       = afterHomography[3].y;//afterHomography[2].y;
					float heightV                 = afterHomography[4].y-y;//afterHomography[3].y-y;
					cv::Rect_<float> estimatedBB(correctedCentroid.x-widthV/2.f, correctedCentroid.y - heightV/2.f,widthV,heightV);
					
					//cv::Rect_<float> estimatedBB(centroid.x-widthValue/2.f, centroid.y-heightValue/2.f, widthValue,heightValue);
					cv::Rect_<float> intersection = groupBb & estimatedBB;


					if(!(intersection.width == 0 || intersection.height == 0))
					{
						Blob* b = blobToUpdate[timestamp];
						b->setBoundingBox(intersection);
						b->setState(ESTIMATED);	
						goodBlobs.insert(std::make_pair(timestamp,std::make_pair(b, false)));
						//std::cout << timestamp << " Timestamp: W:" << *width.begin() << " to " << *width.rbegin() << ". H:" << *height.begin() << " to " << *height.rbegin() << std::endl;
					}
				}
				//else 
				//	std::cout << mLinkedObject->getObjectId() << " p:"  << estimationList.size() << std::endl;
			}
		}
		int interpolatedBox = 0;
		//Interpolated other box if possible
		for(auto blobIt = blobToUpdate.begin(); blobIt != blobToUpdate.end(); ++blobIt)
		{
			if(!blobIt->second)
				LOGERROR("Not existing blob. Model is incoherent between features track and blobs");
			else if(blobIt->second->getState() & (INGROUP|OBJECTGROUP))
			{
				int timestamp = blobIt->first;
				const cv::Rect_<float>& groupBb = blobToUpdate[timestamp]->getBoundingBox();
				Blob* b = blobIt->second;

				auto upper = goodBlobs.upper_bound(timestamp); //higher
				if(upper!=goodBlobs.begin() && upper != goodBlobs.end())
				{
					auto lower = upper;
					--lower;
					if(upper->first > timestamp && lower->first < timestamp)
					{
						cv::Rect_<float> before = (*lower).second.first->getBoundingBox();
						cv::Rect_<float> after = (*upper).second.first->getBoundingBox();
						int timeLapse = (*upper).first-(*lower).first;
						float ratio = ((float)timestamp-(float)(*lower).first)/timeLapse;
						float ratioStart = 1.f-ratio;
						float ratioEnd = ratio;

						float x = ratioStart*before.x+ratioEnd*after.x;
						float y = ratioStart*before.y+ratioEnd*after.y;
						float w = ratioStart*before.width+ratioEnd*after.width;
						float h = ratioStart*before.height+ratioEnd*after.height;
						cv::Rect_<float> estimatedBB(x,y,w,h);
						cv::Rect_<float> intersection = groupBb & estimatedBB;
						if(!(intersection.width == 0 || intersection.height == 0))
						{
							b->setBoundingBox(intersection);
							b->setState(ESTIMATED);
							++interpolatedBox;
						}
					}
				}	
			}
		}
		if(interpolatedBox != 0)
			LOGINFO("Interpolated " << interpolatedBox << " boxes by updateInGroupBlobs.");
	}
}

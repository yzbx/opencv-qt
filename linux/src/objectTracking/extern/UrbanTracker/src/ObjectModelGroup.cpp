#include "ObjectModelGroup.h"
#include "ApplicationContext.h"
#include "ObjectModel.h"
#include "IFeatureDetectorExtractorMatcher.h"
#include "Track.h"
#include "Logger.h"
#include "Tracker.h"
#include "matrix.h"
#include "munkres.h"
#include <limits>
#include "Object.h"
#include "ObjectGroup.h"

ObjectModelGroup::ObjectModelGroup(ApplicationContext* c, ObjectGroup* objectGroup)
: IObjectModel(c)
, mModelGroup(new ObjectModel(c, nullptr)) 
, mTrackListDirty(true)
, mLinkedGroup(objectGroup) //This is most likely coming from unitilialized object. It should never be used here.
{

}

void ObjectModelGroup::simplifyModel()
{
	
}


ObjectModelGroup::~ObjectModelGroup()
{
	delete mModelGroup;
}

bool ObjectModelGroup::hasMinimalMovement(int minimalMovement) const
{
	return IObjectModel::hasMinimalMovement(mModelGroup->getBlobs(), minimalMovement);
}

int ObjectModelGroup::getFirstTimestamp() const
{
	int firstTimestamp = mModelGroup->getFirstTimestamp();
	for(auto it = mObjectModelListNonOwner.begin(); it != mObjectModelListNonOwner.end(); ++it)
	{
		int tmp = (*it)->getFirstTimestamp();
		if(tmp > firstTimestamp)
			firstTimestamp = tmp;
	}
	return firstTimestamp;
}
int ObjectModelGroup::getLastTimestamp() const
{
	int lastTimestamp = mModelGroup->getLastTimestamp();

	for(auto it = mObjectModelListNonOwner.begin(); it != mObjectModelListNonOwner.end(); ++it)
	{
		int tmp = (*it)->getLastTimestamp();
		if(lastTimestamp < tmp)
			lastTimestamp = tmp;
	}
	return lastTimestamp;
}


cv::Rect_<float> ObjectModelGroup::getLastBoundingBox() const
{
	return mModelGroup->getLastBoundingBox();
}


void ObjectModelGroup::addAndMatchTracks(const std::vector<Track*>& tracksB)
{
	updateTrackList();
	std::set<int> usedTracks;
	//Merge Tracks
	std::vector<Track*>& tracksA = mFullTrackList;
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
			mModelGroup->addTrack(B);
		}
	}
	mTrackListDirty = true;
}


void ObjectModelGroup::addAndMatchPoint(int timestamp, const std::vector<FeaturePoint>& featurePointList, const cv::Mat& pointDescriptorList)
{
	updateTrackList();
	std::set<int> newPointMatchedIdx;
	
	std::map<ObjectModel*, std::vector<cv::Point2f>> pointByModel;

	cv::Mat debugImage = getContext()->getCurrentFrame().clone();
	if(!mFullTrackList.empty() && !featurePointList.empty())
	{
		IFeatureDetectorExtractorMatcher* featureManager = getContext()->getTracker()->getFeatureManager();
		
		cv::Mat currentTracksDesc(mFullTrackList.size(), featureManager->getDescriptorSize(), featureManager->getDescriptorType());
		for(unsigned int i = 0; i < mFullTrackList.size(); ++i)		
		{
			mFullTrackList[i]->getDescriptor().row(0).copyTo(currentTracksDesc.row(i));
		}
		std::vector<cv::DMatch> matches;
		featureManager->match(currentTracksDesc, pointDescriptorList, matches);
		
		for(unsigned int matchIdx = 0; matchIdx != matches.size(); ++matchIdx)
		{
			int trackIdx = matches[matchIdx].queryIdx;
			int pointIdx = matches[matchIdx].trainIdx;

			newPointMatchedIdx.insert(pointIdx);
			Track* t = mFullTrackList[trackIdx];
			t->addPoint(timestamp, featurePointList[pointIdx]);	
			t->updateDescriptor(pointDescriptorList.row(pointIdx));
			if(mTrackToModel[t] != mModelGroup)
			{
				pointByModel[mTrackToModel[t]].push_back(featurePointList[pointIdx].getPoint());
				cv::putText(debugImage, "x",featurePointList[pointIdx].getPoint() , cv::FONT_HERSHEY_DUPLEX, 1, mTrackToModel[t]->getLinkedObject()->getColor());
			}
		}
	}

	//for(auto it = pointByModel.begin(); it != pointByModel.end(); ++it)
	//	std::cout << "Matches for "  << (*it).first->getLinkedObject()->getObjectId() << " : " << it->second.size() << std::endl;

	for(unsigned int i = 0; i < featurePointList.size(); ++i)
	{
		if(newPointMatchedIdx.find(i) == newPointMatchedIdx.end())
		{
			Track* t= new Track(pointDescriptorList.row(i));
			t->addPoint(timestamp, featurePointList[i]);
			addTrack(t);
		}
	}
	/*
	std::map<ObjectModel*, int> nbTrackAddedByModel;
	int nbTrackAdded = 0;
	for(unsigned int i = 0; i < featurePointList.size(); ++i)
	{
		if(newPointMatchedIdx.find(i) == newPointMatchedIdx.end())
		{
			std::map<float, ObjectModel*> nearestNeighbor;
			for(auto modelIt = pointByModel.begin(); modelIt != pointByModel.end(); ++modelIt)
			{
				ObjectModel* model = modelIt->first;
				for(auto trackIt = modelIt->second.begin(); trackIt != modelIt->second.end(); ++trackIt)
				{
					float dist = cv::norm((*trackIt)-featurePointList[i].getPoint());
					nearestNeighbor[dist] = model;
				}
			}
			ObjectModel* model = nullptr;

			if(nearestNeighbor.size()>=3)
			{
				auto distIt = nearestNeighbor.begin();
				auto distIt2 = distIt;
				++distIt2;
				auto distIt3 = distIt2;
				++distIt3;


				if(distIt->second == distIt2->second && distIt3->second == distIt2->second)
				{
					model = distIt->second;
					++nbTrackAdded;
					++nbTrackAddedByModel[model];
				}
			}

			Track* t= new Track(pointDescriptorList.row(i));
			t->addPoint(timestamp, featurePointList[i]);
			if(!model)	
			{
				addTrack(t);
				cv::circle(debugImage, featurePointList[i].getPoint(), 3, cv::Scalar(0,0,255));
			}
			else
			{
				model->addTrack(t);		
				cv::circle(debugImage, featurePointList[i].getPoint(), 3, model->getLinkedObject()->getColor());
			}
		}
	}
	cv::imshow("Clustering point",  debugImage);
	mTrackListDirty = true;
	for(auto modelIt = nbTrackAddedByModel.begin(); modelIt != nbTrackAddedByModel.end(); ++modelIt)
	{
		if(modelIt->first && modelIt->first->getLinkedObject())
		{
			LOGINFO(modelIt->second << " tracks were added into " << modelIt->first->getLinkedObject()->getObjectId());
		}
		else
		{
			if(modelIt->first == mModelGroup)
				LOGINFO(modelIt->second << " tracks were added into modelgroup");
			else
				LOGINFO(modelIt->second << " tracks were added into null");
		}
	}


	LOGINFO(nbTrackAdded << " tracks were added to the object in the group " << getLinkedObject()->getObjectId());
	*/
}



void ObjectModelGroup::addPoint(int timestamp, const std::vector<FeaturePoint>& featurePointList, const cv::Mat& pointDescriptorList)
{	
	for(unsigned int i = 0; i < featurePointList.size(); ++i)
	{
		Track* t= new Track(pointDescriptorList.row(i));
		t->addPoint(timestamp, featurePointList[i]);
		addTrack(t);
	}
}

void ObjectModelGroup::addTrack(Track* t)
{
	mModelGroup->addTrack(t);
	mTrackListDirty = true;
}

void ObjectModelGroup::updateTrackList()
{
	if(mTrackListDirty)
	{
		mTrackToModel.clear();
		for(auto trackIt = mModelGroup->getTracks().begin(); trackIt != mModelGroup->getTracks().end(); ++trackIt)
			mTrackToModel[*trackIt] = mModelGroup;
		mFullTrackList.clear();
		mFullTrackList.insert(mFullTrackList.end(), mModelGroup->getTracks().begin(), mModelGroup->getTracks().end());
		for(auto it = mObjectModelListNonOwner.begin(); it != mObjectModelListNonOwner.end(); ++it)
		{
			ObjectModel* tmp = *it;
			mFullTrackList.insert(mFullTrackList.end(), tmp->getTracks().begin(), tmp->getTracks().end());
			for(auto trackIt = tmp->getTracks().begin(); trackIt != tmp->getTracks().end(); ++trackIt)
				mTrackToModel[*trackIt] = *it;
		}
		mTrackListDirty = false;
	}	
}


const std::vector<Track*>& ObjectModelGroup::getTracks()
{
	if(mTrackListDirty)
		updateTrackList();
	return mFullTrackList;
}


const std::map<int, Blob>& ObjectModelGroup::getBlobs()
{
 	LOGWARNING("const std::map<int, Blob>& ObjectModelGroup::getBlobs() is not implemented");
	return mModelGroup->getBlobs();
}

void ObjectModelGroup::replaceBlob(int timestamp, const Blob& b)
{
	LOGWARNING("const std::map<int, Blob>& ObjectModelGroup::replaceBlob() is not implemented");
	mModelGroup->replaceBlob(timestamp, b);
}

void ObjectModelGroup::clearObjectModel()
{
	mModelGroup->clearObjectModel();
	/*for(auto it = mObjectModelListNonOwner.begin(); it != mObjectModelListNonOwner.end(); ++it) //We don't want to do this since we are not the owner.
		(*it)->clearObjectModel();*/
	mObjectModelListNonOwner.clear();
	mTrackListDirty = true;
}
void ObjectModelGroup::clearObjectModelAndDelete()
{
	mModelGroup->clearObjectModelAndDelete();
	for(auto it = mObjectModelListNonOwner.begin(); it != mObjectModelListNonOwner.end(); ++it)
		(*it)->clearObjectModelAndDelete();
	mObjectModelListNonOwner.clear();
	mTrackListDirty = true;
}


void ObjectModelGroup::addMergeBlob(int timestamp, const Blob& b)
{
	mModelGroup->addMergeBlob(timestamp, b);
	
	for(auto it = mObjectModelListNonOwner.begin(); it != mObjectModelListNonOwner.end(); ++it)	
		(*it)->addMergeBlob(timestamp, b);
	
}

void ObjectModelGroup::moveObjectModel(IObjectModel* otherObject)
{
	//const std::vector<Track*>& otherObjectTracks =  otherObject->getTracks();
	//Merge Tracks
	mTrackListDirty = true;
	//addAndMatchTracks(otherObjectTracks);
		
	//Merge blobs
	const std::map<int, Blob>& otherObjectBlob = otherObject->getBlobs();
	addBlobs(otherObjectBlob);
	
	otherObject->clearObjectModel();
}



void ObjectModelGroup::addBlobs(const std::map<int, Blob>& blobs)
{
	mModelGroup->addBlobs(blobs);
}




void ObjectModelGroup::handleSplit(const std::vector<ObjectModel*>& modelList, std::list<IObject*>& trackerObjectList, std::vector<std::pair<IObject*, ObjectModel*>>& outAssociation)
{
	std::list<ObjectModel*> groupObjectList = mObjectModelListNonOwner;
	Matrix<double> similarityMatrix(mObjectModelListNonOwner.size(), modelList.size());
	Matrix<double> deltaTimestampMatrix(mObjectModelListNonOwner.size(), modelList.size());
	std::vector<std::pair<ObjectModel*, ObjectModel*>> associationList;
	std::vector<ObjectModel*> lostObjects;
	std::vector<ObjectModel*> newObjects;
	LOGINFO("Splitting the group");
	updateTrackList();

	std::map<ObjectModel*, std::pair<ObjectModel*, int>> bestMatches;
	int currentTimestamp = getContext()->getTimestamp();
	//Step 1: We verify the object with more than 3 matches and optimize their association with the hungarian algorithm
	int row = 0;
	int totalNbMatches = 0;
	double maxDist = 1;
	for(auto objectsIt = mObjectModelListNonOwner.begin(); objectsIt != mObjectModelListNonOwner.end(); ++objectsIt)
	{
		int col = 0;
		for(auto modelIt = modelList.begin(); modelIt != modelList.end(); ++modelIt)
		{
			std::vector<cv::DMatch> matches;
			ObjectModel::getMatches(*objectsIt, *modelIt, getContext(), matches);			
			int nbMatches = matches.size();
			LOGINFO((*objectsIt)->getLinkedObject()->getObjectId() << " has " << nbMatches);
			nbMatches =  matches.size() >= getContext()->getTrackerAlgorithm().minimumMatchBetweenBlob ? nbMatches : 0;
			//We want at least 3 matches to do an association
			if(nbMatches > 0)
			{
				similarityMatrix(row, col) = nbMatches;
				int timestamp = 0;
				for(int m = 0; m < matches.size(); ++m)	
					timestamp+=  currentTimestamp - (*objectsIt)->getTracks()[matches[m].queryIdx]->getLastTimestamp();
				double distMoy = (double)(timestamp)/(double)(matches.size());
				if(distMoy > maxDist)
				{
					maxDist = distMoy;
				}
				deltaTimestampMatrix(row, col) = distMoy;
				auto matchesIt = bestMatches.find(*objectsIt);
				if(matchesIt == bestMatches.end())
				{
					bestMatches.insert(std::make_pair(*objectsIt, std::make_pair(*modelIt, nbMatches)));
				}
				else
				{
					if(nbMatches > matchesIt->second.second) //Si on voudrait, on pourrait gérer le cas où 2 blob serait identique en utilisant des distances.
					{
						matchesIt->second.first = *modelIt;
						matchesIt->second.second = nbMatches;
					}
				}				
				totalNbMatches += nbMatches;
			}
			else
			{
				similarityMatrix(row, col) = 0;
			}
			++col;
		}
		++row;
	}
	

	std::set<ObjectModel*> usedGroupObject;
	std::set<ObjectModel*> usedNewObject;
	if(totalNbMatches > 0)
	{
		for(unsigned int r = 0; r < mObjectModelListNonOwner.size(); ++r)
		{
			for(unsigned int c = 0; c < modelList.size(); ++c)
			{
				//double distMoyNorm = (deltaTimestampMatrix(r,c)/maxDist);
				//double scoreDistance = (1.0-distMoyNorm); //(On veut donner un avantage de max 2 points à celui avec des points plus récent)
				similarityMatrix(r, c) =  1-(similarityMatrix(r, c))/(totalNbMatches);
			}
		}

		//Résoudre avec l'algorithme hungarien
		Munkres mkres;
		//similarityMatrix.print();
		mkres.solve(similarityMatrix);
		//similarityMatrix.print();
		int row = 0;
		for(auto objectsIt = mObjectModelListNonOwner.begin(); objectsIt != mObjectModelListNonOwner.end(); ++objectsIt)
		{
			int col = 0;
			for(auto modelIt = modelList.begin(); modelIt != modelList.end(); ++modelIt)
			{
				if(similarityMatrix(row, col) == 0)
				{
					associationList.push_back(std::make_pair(*objectsIt,  *modelIt));
					usedGroupObject.insert(*objectsIt);
					usedNewObject.insert(*modelIt);
				}
				++col;
			}
			++row;
		}
	}
	else
		LOGWARNING("There was no matches for the split. This is not very probable.");
	
	
	//2) On utilise les ancien blob du group non matché et on les associes au nouveau à l'aide de la distance
	if(usedGroupObject.size() != mObjectModelListNonOwner.size())
	{
		//Faire une list des vieux blob non matché		
		std::vector<ObjectModel*> unusedGroupObject;
		int i = 0;
		//On sépare les objets qui ont suffisament de match de ceux qui en ont aucun.
		for(auto objectsIt = mObjectModelListNonOwner.begin(); objectsIt != mObjectModelListNonOwner.end(); ++objectsIt)
		{
			if(usedGroupObject.find(*objectsIt) == usedGroupObject.end())
			{
				auto bestIt = bestMatches.find(*objectsIt);
				if(bestIt != bestMatches.end()) 	//1.2) On utilise une méthode greedy pour assigner les objets qui ont des match
				{
					associationList.push_back(std::make_pair(*objectsIt,  bestIt->second.first));
					usedGroupObject.insert(*objectsIt);
				}
				else
					unusedGroupObject.push_back(*objectsIt);
			}
			++i;
		}

		for(auto objectsIt = unusedGroupObject.begin(); objectsIt != unusedGroupObject.end(); ++objectsIt)
		{
			auto greedyMatchIt = bestMatches.find(*objectsIt);
			if(greedyMatchIt != bestMatches.end()) //Si l'objet est celui qui a le plus de match, alors on l'ajoute aux associations
			{
				associationList.push_back(std::make_pair(*objectsIt, greedyMatchIt->second.first));
			}
			else
			{
				//Before adding to the lost the objects, we try to find if there is an overlap
				cv::Rect_<float> rect = (*objectsIt)->getLastBoundingBox(); //Le blob direct serait mieux, mais bon.
				float lastBBArea = (float)rect.area();
				int bestOverlapArea = 0;
				ObjectModel* bestOverlapModel = nullptr; 

				for(auto modelIt = modelList.begin(); modelIt != modelList.end(); ++modelIt)
				{
					cv::Rect_<float> lastBB = (*modelIt)->getLastBoundingBox();
					cv::Rect_<float> intersection = rect & lastBB;
					if(intersection.width > 0 && intersection.height>0)
					{
						int tmpArea = intersection.area();
						if(tmpArea > bestOverlapArea)
						{
							bestOverlapArea = tmpArea;
							bestOverlapModel = *modelIt;
						}
					}
				}

				float overlapRatio = 0;
				if(lastBBArea > 0)
					overlapRatio = (float)bestOverlapArea/lastBBArea;

				if(bestOverlapModel && overlapRatio > 0.7f)
				{
					associationList.push_back(std::make_pair(*objectsIt, bestOverlapModel));
					usedNewObject.insert(bestOverlapModel);
					LOGINFO("Split association with area overlap for " << (*objectsIt)->getLinkedObject()->getObjectId() << " with area overlap of " << overlapRatio);
				}
				else
					lostObjects.push_back(*objectsIt); //Objet perdu
			}
		}
	}
	

	//At this point, we should have matches all the blob history with the new blob. We will now look at the new blobs 		
	if(usedNewObject.size() != modelList.size())
	{
		for(auto newObjectIt = modelList.begin(); newObjectIt != modelList.end(); ++newObjectIt)
		{
			if(usedNewObject.find(*newObjectIt) == usedNewObject.end())
			{
				newObjects.push_back(*newObjectIt);
			}
		}
	}

	//Lost object are added to the lost object list
	for(auto it = lostObjects.begin(); it != lostObjects.end(); ++it)
	{
		Object* obj = (*it)->getLinkedObject();
		mLinkedGroup->removeObject(obj);
		obj->setState(LOST);
		trackerObjectList.push_back(obj);
		mObjectModelListNonOwner.remove(*it);
	}

	//New objects are created
	for(auto it = newObjects.begin(); it != newObjects.end(); ++it)
	{
		Object* newObj = new Object(getContext());	
		newObj->getObjectModel()->moveObjectModel(*it);
		if(Utils::OpenCV::IsOutsideROI(getContext()->getMask(), newObj->getIObjectModel()->getLastBoundingBox()))
			newObj->setState(ENTERING);
		else
			newObj->setState(HYPOTHESIS);
		groupObjectList.push_back(newObj->getObjectModel());
		trackerObjectList.push_back(newObj);
		outAssociation.push_back(std::make_pair(newObj, *it));
	}

	std::map<ObjectModel*, std::vector<ObjectModel*>> associationGroup; //NewBlobToObject
	for(auto it = associationList.begin(); it != associationList.end(); ++it)
		associationGroup[(*it).second].push_back((*it).first);
	
	


	for(auto it = associationGroup.begin(); it != associationGroup.end(); ++it)
	{
		ObjectModel* newBlob = it->first;
		const std::vector<ObjectModel*> associatedObjects = it->second;
		
		if(associatedObjects.size() == 1)
		{
			Object* obj = associatedObjects[0]->getLinkedObject();
			associatedObjects[0]->moveObjectModel(newBlob);
			mLinkedGroup->removeObject(obj);
			mObjectModelListNonOwner.remove(associatedObjects[0]);
			obj->setState(OBJECT);
			trackerObjectList.push_back(obj);
			obj->getObjectModel()->updateInGroupBlobs();
			outAssociation.push_back(std::make_pair(obj, newBlob));
		}
		else if(associatedObjects.size() > 1)
		{
			ObjectGroup* og = new ObjectGroup(getContext());
			for(auto objIt = associatedObjects.begin(); objIt != associatedObjects.end(); ++objIt)	
			{
				og->addObject((*objIt)->getLinkedObject());
				mLinkedGroup->removeObject((*objIt)->getLinkedObject()); //RemoveObject
				mObjectModelListNonOwner.remove((*objIt)); //RemoveModel
			}
			og->getObjectModelGroup()->moveObjectModel(newBlob);
			trackerObjectList.push_back(og);
			outAssociation.push_back(std::make_pair(og, newBlob));

		}
		else
		{
			LOGERROR("All blob should have been matched at that point");
		}

		//At this point, we verify all blob with no correspondance in either the new one or the old one and we associate them with the blob with closest last observation
		
	}

	addUnmatchedGroupBlobToExistingObjects(groupObjectList);
}

void ObjectModelGroup::addUnmatchedGroupBlobToExistingObjects()
{
	addUnmatchedGroupBlobToExistingObjects(mObjectModelListNonOwner);
}

void ObjectModelGroup::addUnmatchedGroupBlobToExistingObjects(const std::list<ObjectModel*>& groupObjectList)
{
		std::map<int, std::vector<Object*>> matchedGroupTimestamp;
		for(auto objectIt = groupObjectList.begin(); objectIt != groupObjectList.end(); ++objectIt)
		{
			const auto& objectBlobList = (*objectIt)->getBlobs();
			for(auto blobIt = objectBlobList.begin(); blobIt != objectBlobList.end(); ++blobIt)
			{		
				if(!(blobIt->second.getState() & (INGROUP|OBJECTGROUP)))	
				{
					int timestamp = blobIt->first;
					auto groupBlobIt = matchedGroupTimestamp.find(timestamp);
					if(groupBlobIt == matchedGroupTimestamp.end())
					{
						std::vector<Object*> objectList;
						objectList.push_back((*objectIt)->getLinkedObject());
						matchedGroupTimestamp.insert(std::make_pair(timestamp, objectList));
					}
					else
						groupBlobIt->second.push_back((*objectIt)->getLinkedObject());
				}
			}
		}
		if(!matchedGroupTimestamp.empty())
		{
			const auto& tracks = this->mModelGroup->getTracks();
			auto& groupBlobs = mModelGroup->getBlobs();
			for(auto blobIt = groupBlobs.begin(); blobIt != groupBlobs.end(); ++blobIt)
			{			
				int timestamp = blobIt->first;
				auto missingBlobIt = matchedGroupTimestamp.find(timestamp);
				if(missingBlobIt == matchedGroupTimestamp.end())
				{
					LOGINFO("Missing group blob at " << timestamp << ". Trying to associate it");
					//We will assign it to the most appropriate blob
					auto firstElementGreaterOrEqual = matchedGroupTimestamp.lower_bound(timestamp);
					if(firstElementGreaterOrEqual != matchedGroupTimestamp.begin())
					{
						auto elementBefore = firstElementGreaterOrEqual;
						--elementBefore;

						Object* bestObject = nullptr;
						//We want to know which element is last element to had an item
						if(elementBefore->second.size() == 1)
						{
							bestObject = elementBefore->second[0];
						}
						else
						{
							//We will discriminate by size
							float boundingBoxArea = blobIt->second.getBoundingBox().area();

							float candidateAreaDelta = 9999999999999999.f;

							auto candidateBlobList = elementBefore->second;
							for(auto candidateIt = candidateBlobList.begin(); candidateIt != candidateBlobList.end(); ++candidateIt)
							{
								auto candidateBlobIt = (*candidateIt)->getIObjectModel()->getBlobs().find(elementBefore->first);
								if(candidateBlobIt != (*candidateIt)->getIObjectModel()->getBlobs().end())
								{
									float deltaArea = abs((*candidateBlobIt).second.getBoundingBox().area() - boundingBoxArea);
									if(deltaArea < candidateAreaDelta)
									{
										candidateAreaDelta = deltaArea;
										bestObject = (*candidateIt);
									}
								}
							}
						}
						if(bestObject)
						{
							Blob b = blobIt->second;
							b.setState(ESTIMATED);
							//Add associated tracks

							std::set<Track*> tracksToRemove;
							for(auto trackIt = tracks.begin(); trackIt != tracks.end(); ++trackIt)
							{
								Track* t = *trackIt;
								if(t->getFirstTimestamp() == timestamp)
								{
									const cv::Point2f& pt = t->getPointList().begin()->second.getPoint();
									if(b.getBoundingBox().contains(pt))
									{
										tracksToRemove.insert(t);
										bestObject->getObjectModel()->addTrack(t);
									}
								}
							}
							LOGINFO(tracksToRemove.size() << " tracks added from group.");
							if(!tracksToRemove.empty())
								mModelGroup->removeTracksNoDelete(tracksToRemove);

							//bestObject->getIObjectModel()->addAndMatchPoint()
							bestObject->getIObjectModel()->replaceBlob(timestamp, b);							
							//We have one element, we will add it to it
							LOGINFO("Adding group blob " << timestamp << " at " << bestObject->getObjectId());
						}
						else
							LOGWARNING("No object to take a blob. This should never happen");
					}				
				}///
			}
			//We will add tracks



			

		}	
		else
			LOGERROR("Group with no object to assign data into. This seems to be a bug !");
	
}
void ObjectModelGroup::addObjectModel(ObjectModel* om)
{
	mObjectModelListNonOwner.push_back(om);
}

void ObjectModelGroup::removeObjectModel(ObjectModel* om)
{
	mObjectModelListNonOwner.remove(om);
}
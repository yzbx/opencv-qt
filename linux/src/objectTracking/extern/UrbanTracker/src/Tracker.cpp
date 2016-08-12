#include "Tracker.h"
#include "Timer.h"
#include "ApplicationContext.h"
#include "FeatureDetectorExtractorMatcher.h"
#include "BlobDetector.h"
#include "brisk/brisk.h"
#include "IBGS.h"
#include "StringHelpers.h"
#include "ObjectState.h"
#include "FrameAssociation.h"
#include "Logger.h"
#include "IObject.h"
#include "Object.h"
#include "ObjectGroup.h"
#include "TrackerPersistance.h"

#include <opencv2/nonfree/features2d.hpp>

//#define BGS_CREATION


Tracker::Tracker(ApplicationContext* appContext, const std::string& bgsType)
: mContext(appContext)
, mBlobExtractor(new BlobDetector(appContext, bgsType))
, mCurrentFrameIdx(0)
, mLastFrameIdx(1)
{
	mContext->setTracker(this);
	int threshold = appContext->getTrackerAlgorithm().detectorThreshold;
	int octave = appContext->getTrackerAlgorithm().detectorOctave;
	//TODO: Param FREAK vérifier impact
	mPointDetectors = new FeatureDetectorExtractorMatcher(new cv::BriskFeatureDetector(threshold,octave), new cv::FREAK(/*true, true, 22.f, octave*/), new cv::BFMatcher(cv::NORM_HAMMING, false), appContext->getTrackerAlgorithm().matchRatio);
}

Tracker::~Tracker()
{
	std::list<IObject*> unsavedObject = getObject(mObjectList, ALL^HYPOTHESIS);
	for(auto it = unsavedObject.begin(); it != unsavedObject.end(); ++it)
	{
		
		if((*it)->getState() == OBJECTGROUP)
		{
			ObjectGroup* og = dynamic_cast<ObjectGroup*>(*it);
			if(og)
			{
				auto& childList = og->getObjects();
				for(auto childIt = childList.begin(); childIt != childList.end(); ++childIt)
				{
					Object* obj = *childIt;
					saveGoodObjects(obj);
				}
				childList.clear();
			}
		}
		else
		{
			saveGoodObjects(*it);
		}
	}
	for(auto it = mObjectList.begin(); it != mObjectList.end(); ++it)
		delete *it;
	mObjectList.clear();

	delete mPointDetectors;
	delete mBlobExtractor;
}

void Tracker::draw(cv::Mat& img)
{
	if(mContext->getRefDrawingFlags()->mDrawPerspective)
	{
		if(mContext->getRefDrawingFlags()->mDrawBackgroundSubstraction)
			cv::imshow("BGS", mBlobExtractor->getBGSMask());
	

	//Afficher le # de trame courante
	std::string nbFrameCourant = Utils::String::toString(this->mContext->getTimestamp());
	cv::putText(img, nbFrameCourant, cv::Point(img.cols-100,25), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255,0,255),2);

	if(mContext->getRefDrawingFlags()->mDrawKeypoints)
	{
		for(unsigned int keyPtIdx = 0; keyPtIdx < mPointBlobAssociation[mCurrentFrameIdx].mkeyPoints.size(); ++keyPtIdx)	
			cv::circle(img, mPointBlobAssociation[mCurrentFrameIdx].mkeyPoints[keyPtIdx].pt, 1, cv::Scalar(255,0,0), 2);
	}

	if(mContext->getRefDrawingFlags()->mDrawMatches)
	{
		for(unsigned int matchIdx = 0; matchIdx < mPointMatches.size(); ++matchIdx)
		{
			cv::KeyPoint pt1 = mPointBlobAssociation[mLastFrameIdx].mkeyPoints[mPointMatches[matchIdx].queryIdx];
			cv::KeyPoint pt2 = mPointBlobAssociation[mCurrentFrameIdx].mkeyPoints[mPointMatches[matchIdx].trainIdx];
			cv::line(img, pt1.pt, pt2.pt, cv::Scalar(0,255,0), 4);
		}
	}

	//Draw objects
	for(auto it = mObjectList.begin(); it != mObjectList.end(); ++it)
		(*it)->draw(img);
		


	//mPointMatches
		
	if(mContext->getDrawingFlags()->mDrawTrackerStats)
	{
		std::string nbObjects = Utils::String::toString(mObjectList.size()) +" objects";
		cv::putText(img, nbObjects, cv::Point(0,50), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255,0,255),2);
		std::string nbBlobs = Utils::String::toString(mIdBoundingBoxMap[mCurrentFrameIdx].size()) + " blobs";
		cv::putText(img, nbBlobs, cv::Point(0,75), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255,0,255),2);
		std::string nbFeatures = Utils::String::toString(mPointBlobAssociation[mCurrentFrameIdx].mkeyPoints.size()) + " features";
		cv::putText(img, nbFeatures, cv::Point(0,100), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255,0,255),2);
		std::string nbMatches = Utils::String::toString(mPointMatches.size()) + " matches";
		cv::putText(img, nbMatches, cv::Point(0,125), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255,0,255),2);
	}

	}
	
}

void Tracker::processFrame(const cv::Mat& currentFrame)
{
	/*static cv::VideoWriter videoWrite("im/video3.avi", -1, 25, cv::Size(currentFrame.cols, currentFrame.rows));
	static int id = mContext->getTimestamp();
	std::stringstream ss;
	ss << "im/rouen_" << id << ".png";
	std::string fileName = ss.str();
	cv::imwrite(fileName, currentFrame);
	videoWrite << currentFrame;
	++id;*/

	//Swap buffer index
	std::swap(mLastFrameIdx, mCurrentFrameIdx);
	mBlobLabelToObject[mCurrentFrameIdx].clear();
	mPointBlobAssociation[mCurrentFrameIdx].reset();
	mIdBoundingBoxMap[mCurrentFrameIdx].clear();
	mObjectToBlobLabel[mCurrentFrameIdx].clear();
	mReplacementObject.clear();
	
	//1) Détection des blobs
	mBlobExtractor->update(currentFrame);
//#define BGS_CREATION
#ifndef BGS_CREATION
	mIdBoundingBoxMap[mCurrentFrameIdx] = mBlobExtractor->getBlobBoundingBoxMap();

	//2) Extraction des points des blobs et association 
	mPointDetectors->detect(currentFrame, mPointBlobAssociation[mCurrentFrameIdx].mkeyPoints, mPointBlobAssociation[mCurrentFrameIdx].mDescriptors, mBlobExtractor->getBGSMask());
	mPointBlobAssociation[mCurrentFrameIdx].calculatePointBlobAssociation(mBlobExtractor->getLabelMask(), mContext);
	mPointMatches.clear();
	mPointDetectors->match(mPointBlobAssociation[mLastFrameIdx].mDescriptors, mPointBlobAssociation[mCurrentFrameIdx].mDescriptors, mPointMatches);
	
	std::map<std::pair<int,int>, int> nbMatches;

	//3.1) Calcul des associations par points
	for(auto it = mPointMatches.begin(); it != mPointMatches.end(); ++it)
	{
		
		auto keyPair = std::make_pair(mPointBlobAssociation[mLastFrameIdx].mBlobId[(*it).queryIdx], mPointBlobAssociation[mCurrentFrameIdx].mBlobId[(*it).trainIdx]);
		std::map<std::pair<int,int>, int>::iterator matcheIt =  nbMatches.find(keyPair);		
		if(matcheIt == nbMatches.end())
		{
			nbMatches.insert(std::make_pair(keyPair, 1));
		}
		else
			++matcheIt->second;
	}
	if(this->mContext->getTimestamp() == 201)
	{
		std::cout << "BBQ!";
	}
	

	
	//3.2) Calcul des associations B_{t-1} et B_t
	FrameAssociation association(mIdBoundingBoxMap[mLastFrameIdx], mIdBoundingBoxMap[mCurrentFrameIdx], nbMatches);
	

	//4) On met à jour le modèle des objets
	updateModel(association, currentFrame);

	//5) On met à jour l'état des objets
	updateState();

	//6) Mettre à jour l'association des blobs avec leur objet		
	std::map<IObject*, int> objectToLabel;
	for(auto it = mBlobLabelToObject[mCurrentFrameIdx].begin(); it != mBlobLabelToObject[mCurrentFrameIdx].end(); )
	{
		int objectLabel = it->first;
		IObject* object = it->second;
		auto objectIt = objectToLabel.find(object);
		if(objectIt != objectToLabel.end())
		{
			int newLabelForObject = objectIt->second;


			mPointBlobAssociation[mCurrentFrameIdx].updateBlobId(objectLabel, newLabelForObject);

			mIdBoundingBoxMap[mCurrentFrameIdx][newLabelForObject] = Utils::OpenCV::mergeBlobRect(mIdBoundingBoxMap[mCurrentFrameIdx][objectLabel], mIdBoundingBoxMap[mCurrentFrameIdx][newLabelForObject]);
			mIdBoundingBoxMap[mCurrentFrameIdx].erase(objectLabel);
			if(newLabelForObject != objectLabel)
			{
				auto blobLblIt = mBlobLabelToObject[mCurrentFrameIdx].find(objectLabel);
				if(blobLblIt != mBlobLabelToObject[mCurrentFrameIdx].end())
				{
					auto labelIt = mObjectToBlobLabel[mCurrentFrameIdx].find(blobLblIt->second);
					if(labelIt != mObjectToBlobLabel[mCurrentFrameIdx].end())
					{
						labelIt->second.erase(objectLabel);
						if(labelIt->second.empty())
							mObjectToBlobLabel[mCurrentFrameIdx].erase(labelIt);
					}
					it = mBlobLabelToObject[mCurrentFrameIdx].erase(blobLblIt);
				}
				else
					++it;
			}
		}
		else
		{
			objectToLabel[object] = objectLabel;
			++it;
		}
	}



	
	std::list<IObject*> groupList = getObject(mObjectList, OBJECTGROUP);
	//Update occluded bounding box
	for(auto it = groupList.begin(); it != groupList.end(); ++it)
	{
		ObjectGroup* group = dynamic_cast<ObjectGroup*>(*it);
		
		if(group)
		{
			const std::vector<Object*> objectList = group->getObjects();
			for(auto objectIt = objectList.begin(); objectIt != objectList.end(); ++objectIt)
			{				
				ObjectModel* om = (*objectIt)->getObjectModel();
				om->updateInGroupBlobs();	
			}
		}
		else
			LOGERROR("ObjectGroup type should be reserved for actual group");

	}
#endif
}

void Tracker::updateModelWithBlob(IObject* obj, int newBlobId, const cv::Mat& currentFrame)
{
	const cv::Rect_<float>& newBlobBoundingBox = mIdBoundingBoxMap[mCurrentFrameIdx][newBlobId].mBoundingBox;
	obj->getIObjectModel()->addMergeBlob(mContext->getTimestamp(), Blob(newBlobBoundingBox, obj->getState(), mContext));
			
	std::vector<FeaturePoint> keyPointList;
	cv::Mat descriptorList;
	mPointBlobAssociation[mCurrentFrameIdx].getKpDesc(newBlobId, keyPointList, descriptorList);
	obj->getIObjectModel()->addAndMatchPoint(mContext->getTimestamp(), keyPointList, descriptorList);
	updateBlobFrameObjectMapping(obj, newBlobId);
}



void Tracker::handleNewObjects(const FrameAssociation& association, const cv::Mat& currentFrame)
{
	const std::set<int>& newBlob = association.getNewBlob();
	//1)	On créer des objets de type hypothèse/entrant pour tout les cas 0-1
	//		On met les points caractéristiques et l'histogramme de dedans
	for(auto newBlobIt = newBlob.begin(); newBlobIt != newBlob.end(); ++newBlobIt)
	{
		int newBlobLabel = *newBlobIt;
		Object* obj = new Object(mContext);
		updateBlobFrameObjectMapping(obj, newBlobLabel);

		const cv::Rect_<float>& newBlobBoundingBox = mIdBoundingBoxMap[mCurrentFrameIdx][newBlobLabel].mBoundingBox;
		ObjectState state = Utils::OpenCV::IsOutsideROI(mContext->getMask(),newBlobBoundingBox) ? ENTERING : HYPOTHESIS;
		obj->setState(state);	

		updateModelWithBlob(obj, newBlobLabel,currentFrame);
		mObjectList.push_back(obj);
	}
}
void Tracker::handleSplits(const FrameAssociation& association, const cv::Mat& currentFrame)
{
	//Handle split
	const std::set<std::pair<int, std::set<int>>>& splitList = association.getBlobSplit();
	for(auto splitIt = splitList.begin(); splitIt != splitList.end(); ++splitIt)
	{
		int oldBlobLabel = splitIt->first;
		const std::set<int>& newBlobLabels = splitIt->second;
		auto objectIt = mBlobLabelToObject[mLastFrameIdx].find(oldBlobLabel);
		if(objectIt != mBlobLabelToObject[mLastFrameIdx].end())
		{
			IObject* object = objectIt->second;
			
			if(object->getState() == OBJECTGROUP)
			{
				ObjectGroup* og = dynamic_cast<ObjectGroup*>(object);
				LOGASSERT(og, "ObjectGroup is of the wrong type");
		
				std::vector<ObjectModel*> omList;
				std::map<ObjectModel*, int> modelToBlobLabel;
				for (auto it = newBlobLabels.begin(); it != newBlobLabels.end(); ++it)
				{
					int label = (*it);
					ObjectModel* om = new ObjectModel(mContext, nullptr);
					std::vector<FeaturePoint> keyPointList;
					cv::Mat descriptorList;
					mPointBlobAssociation[mCurrentFrameIdx].getKpDesc(label, keyPointList, descriptorList);					
					const cv::Rect_<float>& newBlobBoundingBox = mIdBoundingBoxMap[mCurrentFrameIdx][label].mBoundingBox;
					om->addPoint(mContext->getTimestamp(), keyPointList, descriptorList);
					om->addMergeBlob(mContext->getTimestamp(), Blob(newBlobBoundingBox, HYPOTHESIS, mContext));
					omList.push_back(om);
					modelToBlobLabel[om] = label;
				}
				//LOGINFO("Finding similarity");
				std::vector<std::pair<IObject*, ObjectModel*>> outputAssociation;
				og->getObjectModelGroup()->handleSplit(omList, mObjectList, outputAssociation);				
				for(auto associationIt = outputAssociation.begin(); associationIt != outputAssociation.end(); ++associationIt)				
					updateBlobFrameObjectMapping(associationIt->first, modelToBlobLabel[associationIt->second]);
				
				og->setState(DELETED);
				//updateInGroupBlobs
			}
			else
			{
				Object* obj = dynamic_cast<Object*>(object);
				LOGASSERT(obj, "Object should not be of any other state");

				//BlobRect
				typedef std::list<std::pair<int, BlobRect>> SpatiallyRelatedBlobList;
				std::list<SpatiallyRelatedBlobList> blobDivision;
				for (auto blobLabelIt = newBlobLabels.begin(); blobLabelIt != newBlobLabels.end(); ++blobLabelIt)
				{
					int blobLabel = *blobLabelIt;
					std::list<std::pair<int, BlobRect>> overlappedPair;
					BlobRect br;
					auto blobIt = mIdBoundingBoxMap[mCurrentFrameIdx].find(blobLabel);
					if (blobIt != mIdBoundingBoxMap[mCurrentFrameIdx].end())
					{						
						const cv::Rect& bbRect = blobIt->second.mBoundingBox;
						br.mBoundingBox = Utils::OpenCV::dilate(bbRect, mContext->getTrackerAlgorithm().maxSegDist);
						if (br.mBoundingBox.area() > 0)
						{
							cv::resize(blobIt->second.mBlob, br.mBlob, br.mBoundingBox.size(), 0, 0);
							overlappedPair.push_back(std::make_pair(blobLabel, br));
							blobDivision.push_back(overlappedPair);
						}
						else
						{
							LOGERROR("Blob area should be bigger than 0");
						}
					}
					else
					{
						LOGERROR("blobLabel is not present in frame");
					}

					
				}
				int originalBlobDivisionSz = blobDivision.size();
				int blobDivisionSz;
				//Identify overlapping spatially related blob list. We do multiple pass.
				//Surely this can be optimized
				do
				{
					blobDivisionSz = blobDivision.size();
					for(auto bdIt = blobDivision.begin(); bdIt != blobDivision.end(); )
					{			
						auto bdIt2 = bdIt;
						++bdIt2;
						for(; bdIt2 != blobDivision.end(); )
						{			
							if(blobListOverlap(*bdIt, *bdIt2))
							{
								//Merge both list
								(*bdIt).insert((*bdIt).end(), (*bdIt2).begin(),(*bdIt2).end());
								//First loop iterator will be invalid after this, so we assign it
								bdIt = blobDivision.erase(bdIt2);
								bdIt2 = bdIt;
								if (bdIt != blobDivision.end())
									++bdIt2;
							}
							else
								++bdIt2;
						}
						if (bdIt != blobDivision.end())
							++bdIt;
					}
				}
				while (blobDivision.size() != blobDivisionSz && blobDivision.size() > 1);
				//We merge simply the blobs
				if(blobDivision.empty())
				{
					LOGERROR("No blob division, original division size was " << originalBlobDivisionSz << " blobLabelSz: " << newBlobLabels.size());
				}
				else if(blobDivision.size() == 1)
				{
					const std::list<std::pair<int, BlobRect>>& blobToMerge = *blobDivision.begin();
					
					for(auto blobToMergeIt = blobToMerge.begin(); blobToMergeIt != blobToMerge.end(); ++blobToMergeIt)
					{
						//Update Model
						int newBlobId = blobToMergeIt->first;
						updateModelWithBlob(obj, newBlobId, currentFrame);
					}					
				}
				else //More than 1
				{
					//LOGWARNING("This is a real split from an object");
					std::vector<ObjectModel*> modelList;
					std::map<ObjectModel*, std::vector<int>> indexToLabelList;
					
					for(auto blobIt = blobDivision.begin(); blobIt != blobDivision.end(); ++blobIt)
					{						
						ObjectModel* om = new ObjectModel(mContext, nullptr);
						for(auto subBlobIt = blobIt->begin(); subBlobIt != blobIt->end(); ++subBlobIt)
						{
							int label = subBlobIt->first;
							std::vector<FeaturePoint> keyPointList;
							cv::Mat descriptorList;
							mPointBlobAssociation[mCurrentFrameIdx].getKpDesc(label, keyPointList, descriptorList);					
							const cv::Rect_<float>& newBlobBoundingBox = mIdBoundingBoxMap[mCurrentFrameIdx][label].mBoundingBox;
							om->addPoint(mContext->getTimestamp(), keyPointList, descriptorList);
							om->addMergeBlob(mContext->getTimestamp(), Blob(newBlobBoundingBox, HYPOTHESIS, mContext));
							indexToLabelList[om].push_back(label);
						}
						modelList.push_back(om);
					}

					std::vector<ObjectModel*> splittedModel;
					ObjectModel* keptModel = obj->split(modelList, splittedModel);
					//Update new object blob reference
					const std::vector<int>& labelToUpdate = indexToLabelList[keptModel];
					for(auto it = labelToUpdate.begin(); it != labelToUpdate.end(); ++it)
						updateBlobFrameObjectMapping(obj, *it);

					for(auto blobIt = splittedModel.begin(); blobIt != splittedModel.end(); ++blobIt)
					{
						Object* newObj = new Object(mContext);
						const std::vector<int>& labelToUpdate = indexToLabelList[*blobIt];
						for(auto it = labelToUpdate.begin(); it != labelToUpdate.end(); ++it)
						{
							int newBlobLabel = *it;
							updateBlobFrameObjectMapping(newObj, newBlobLabel);
							const cv::Rect_<float>& newBlobBoundingBox = mIdBoundingBoxMap[mCurrentFrameIdx][newBlobLabel].mBoundingBox;
							ObjectState state = Utils::OpenCV::IsOutsideROI(mContext->getMask(),newBlobBoundingBox) ? ENTERING : HYPOTHESIS;
							newObj->setState(state);
							updateModelWithBlob(newObj, newBlobLabel, currentFrame);
							
						}
						mObjectList.push_back(newObj);
					}
					for(auto it = modelList.begin(); it != modelList.end(); ++it)
					{
						delete *it;
					}


					//TODO: Create new object. Create history. Update Blob reference
					
					
					/*
					int label = (*it);
					ObjectModel* om = new ObjectModel(mContext, nullptr);
					std::vector<FeaturePoint> keyPointList;
					cv::Mat descriptorList;
					mPointBlobAssociation[mCurrentFrameIdx].getKpDesc(label, keyPointList, descriptorList);					
					const cv::Rect_<float>& newBlobBoundingBox = mIdBoundingBoxMap[mCurrentFrameIdx][label];
					om->addPoint(mContext->getTimestamp(), keyPointList, descriptorList);
					om->updateHistogram(currentFrame, mBlobExtractor->getLabelMask(), label, newBlobBoundingBox); 
					om->addMergeBlob(mContext->getTimestamp(), Blob(newBlobBoundingBox, HYPOTHESIS, mContext));
					omList.push_back(om);
					modelToBlobLabel[om] = label;*/


					//We will have to create the blobs and provide them to the object
					//The object shall return the new object
					//ResolveObject
					//give list to object return idx of most similar 
					//use a method to extract history

					//We will split some part of the object and we will want to assign the most similar of both blob to the object and create a new blob.
				}
			}
		}
		else
		{
			LOGERROR(mContext->getTimestamp() << ": Object " << oldBlobLabel << " should exist3?");
		}
		
	}
}

void Tracker::mergeObjectWithSimilarStart(const std::list<IObject*>& enteringList, std::list<IObject*>& mergedList, int maxTimeDistance, int newBlobLabel)
{
	if(enteringList.size() == 1)
	{
		mergedList.push_back(*enteringList.begin());
	}
	else if(enteringList.size() > 1)
	{
		for(auto enteringIt = enteringList.begin(); enteringIt != enteringList.end(); ++enteringIt)
		{
			bool merged = false;
			for(auto mergedObjectIt = mergedList.begin(); mergedObjectIt != mergedList.end() && merged==false; ++mergedObjectIt)
			{
				if(abs((*enteringIt)->getIObjectModel()->getFirstTimestamp() - (*mergedObjectIt)->getIObjectModel()->getFirstTimestamp()) < maxTimeDistance)
				{
					(*mergedObjectIt)->getIObjectModel()->moveObjectModel((*enteringIt)->getIObjectModel());
					updateObjectReference((*enteringIt), (*mergedObjectIt));
					(*enteringIt)->setState(DELETED);
					merged = true;
				}
			}
			if(!merged)
			{
				IObject* mainObject = *enteringIt;
				updateBlobFrameObjectMapping(mainObject, newBlobLabel);
				mergedList.push_back(mainObject);
			}
		}
	}
}



void Tracker::handleMerge(const FrameAssociation& association, const cv::Mat& currentFrame)
{
		//Handle merges
	const std::set<std::pair<std::set<int>, int>>& mergesList = association.getBlobMerge();
	for(auto mergeIt = mergesList.begin(); mergeIt != mergesList.end(); ++mergeIt)
	{
		std::list<IObject*> objectList;
		int newBlobLabel = mergeIt->second;
		
		const std::set<int>& mergingBlob = mergeIt->first;
		for(auto mergedBlobIt = mergingBlob.begin(); mergedBlobIt != mergingBlob.end(); ++mergedBlobIt)
		{
			auto objectIt = mBlobLabelToObject[mLastFrameIdx].find(*mergedBlobIt);
			if(objectIt != mBlobLabelToObject[mLastFrameIdx].end())
			{
				objectList.push_back(objectIt->second);
				//updateBlobFrameObjectMapping(objectIt->second,newBlobLabel);
			}
			else
			{
				LOGERROR(mContext->getTimestamp() << ": Object " << *mergedBlobIt << " should exist2?");
			}
		}

		std::list<IObject*> enteringList = getObject(objectList, ENTERING);
		std::list<IObject*> hypothesisList = getObject(objectList, HYPOTHESIS);
		std::list<IObject*> leavingList = getObject(objectList, LEAVING);
		std::list<IObject*> otherObjectList = getObject(objectList, ALL^(HYPOTHESIS|LOST|ENTERING|LEAVING));

		//If we are in the leaving state, we want don't want to merge with a new object unless we are sure its not a LEAVING-ENTERING id change.

		for(auto leavingObjectIt = leavingList.begin(); leavingObjectIt != leavingList.end(); ++leavingObjectIt)
		{
			std::vector<IObject*> newObjects;
			IObject* object = *leavingObjectIt;
			object->getIObjectModel()->handleLeaving(newObjects);
			if(newObjects.empty())
			{
				object->setState(OBJECT);
				otherObjectList.push_back(object);
			}
			else
			{
				object->setState(SAVEANDELETE);
				if(newObjects.size() == 1)
				{
					objectList.remove(object);
					newObjects[0]->setState(OBJECT);

					updateObjectReference(object,newObjects[0]);
					mObjectList.push_back(newObjects[0]);
					otherObjectList.push_back(newObjects[0]);
					objectList.push_back(newObjects[0]);
				}
				else
					LOGWARNING("handleLeaving does not support multiple object output");
			}
		}
	
		IObject* resultObject = nullptr;
		
		//If we have multiple object entering, we want to merge them and add them to the object list
		if(!enteringList.empty())
		{
			std::list<IObject*> mergedObjects;
			mergeObjectWithSimilarStart(enteringList, mergedObjects, 10, newBlobLabel);
			for(auto mergedBlobIt = mergedObjects.begin(); mergedBlobIt != mergedObjects.end(); ++mergedBlobIt)
			{ 
				IObject* mainObject = *mergedBlobIt;
				if((mainObject->getIObjectModel()->getLastTimestamp() - mainObject->getIObjectModel()->getFirstTimestamp() +1) >= mContext->getTrackerAlgorithm().maxFrameHypothesis)
					otherObjectList.push_back(mainObject);			
				else
					hypothesisList.push_back(mainObject);
			}
			if(!mergedObjects.empty())
				resultObject = (*mergedObjects.begin());
		}
		
		//Plusieurs objet, on crée un groupe. On fusionne les hypothèses avec les objets les plus approprié
		if(otherObjectList.size()>1)
		{
			LOGINFO("Creating group");
			ObjectGroup* objectGroup = nullptr;
			std::list<IObject*> groupList = getObject(objectList, OBJECTGROUP);
			std::list<IObject*> remainingObjectList = getObject(objectList, ALL^OBJECTGROUP); //TODO: This should be remaining objectList
			if(groupList.empty())
			{
				objectGroup = new ObjectGroup(mContext);
				mObjectList.push_back(objectGroup);				
			}
			else
			{ // Si il y a déjà un groupe, on prends celui-là
				auto groupIt = groupList.begin();
				objectGroup = dynamic_cast<ObjectGroup*>(*groupIt);
				LOGASSERT(objectGroup != nullptr, "Wrong state set to object. Object is not a group");
				++groupIt;
				for(; groupIt != groupList.end(); ++groupIt)
					remainingObjectList.push_back(*groupIt);
			}			
			for(auto it = remainingObjectList.begin(); it != remainingObjectList.end(); ++it)
			{
				IObject* object = (*it);
				objectGroup->addObject(object);				
				mObjectList.remove(object);				
				updateObjectReference(object, objectGroup);
			}
			resultObject = objectGroup;
		}
		else //On a un objet ou moins et des hypothèses
		{
			auto hypothesisIt = hypothesisList.begin();
			IObject* mainObject = nullptr;
			if(!otherObjectList.empty())			
				mainObject = *otherObjectList.begin();
			else
			{
				mainObject = *hypothesisIt;
				++hypothesisIt;
			}
			updateBlobFrameObjectMapping(mainObject, newBlobLabel);
			for(; hypothesisIt != hypothesisList.end(); ++hypothesisIt)
			{
				mainObject->getIObjectModel()->moveObjectModel((*hypothesisIt)->getIObjectModel());
				updateObjectReference((*hypothesisIt), mainObject);
				(*hypothesisIt)->setState(DELETED);					
			}		
			resultObject = mainObject;
		}
		updateModelWithBlob(resultObject, newBlobLabel, currentFrame);
		//Update non updated reference
		for(auto mergedBlobIt = mergingBlob.begin(); mergedBlobIt != mergingBlob.end(); ++mergedBlobIt)
		{
			auto objectIt = mBlobLabelToObject[mLastFrameIdx].find(*mergedBlobIt);
			if(objectIt != mBlobLabelToObject[mLastFrameIdx].end())
			{			
				updateBlobFrameObjectMapping(objectIt->second,newBlobLabel);
			}
		}
	}
}


void Tracker::handleOneToOne(const FrameAssociation& association, const cv::Mat& currentFrame)
{
	const std::set<std::pair<int, int>>& oneOnOneAssociation = association.getDirectMatchBlob();
	for(auto oneOnOneIt  = oneOnOneAssociation.begin(); oneOnOneIt != oneOnOneAssociation.end(); ++oneOnOneIt)
	{
		int oldBlobId = oneOnOneIt->first;
		int newBlobId = oneOnOneIt->second;

		auto objectIt = mBlobLabelToObject[mLastFrameIdx].find(oldBlobId);
		if(objectIt != mBlobLabelToObject[mLastFrameIdx].end())
		{
			IObject* obj = objectIt->second;
			updateModelWithBlob(obj, newBlobId,currentFrame);			
		}
		else
		{
			LOGERROR(mContext->getTimestamp() << ": Object blobid " << oldBlobId << " should exist?");
		}	
	}
}


void Tracker::updateModel(const FrameAssociation& association, const cv::Mat& currentFrame)
{
	handleNewObjects(association, currentFrame);
	handleOneToOne(association, currentFrame);
	handleMerge(association, currentFrame);
	handleSplits(association, currentFrame);
}


bool Tracker::blobListOverlap(const std::list<std::pair<int, BlobRect>>& A, const std::list<std::pair<int, BlobRect>>& B)
{
	bool overlap = false;
	for(auto itA = A.begin(); itA != A.end() && !overlap; ++itA)
	{
		for(auto itB = B.begin(); itB != B.end() && !overlap; ++itB)
		{
			if(BlobRect::getAreaIntersection((*itA).second,(*itB).second) > 0)
				overlap = true;
		}
	}
	return overlap;
}


void Tracker::updateBlobFrameObjectMapping(IObject* object, int blobLabel)
{
	mBlobLabelToObject[mCurrentFrameIdx][blobLabel] = object;
	mObjectToBlobLabel[mCurrentFrameIdx][object].insert(blobLabel);
}


void Tracker::updateObjectReference(IObject* oldObject, IObject* newObject)
{
	for(unsigned int frameIdx = 0; frameIdx <= 1; ++frameIdx) //Last and current frame
	{
		for(auto it = mBlobLabelToObject[frameIdx].begin(); it != mBlobLabelToObject[frameIdx].end(); ++it)
		{
			if(it->second == oldObject)
				it->second = newObject;
		}	
		auto it2 = mObjectToBlobLabel[frameIdx].find(oldObject);
		if(it2 != mObjectToBlobLabel[frameIdx].end())
		{
			std::set<int> labels = it2->second;
			mObjectToBlobLabel[frameIdx].erase(it2);
			mObjectToBlobLabel[frameIdx][newObject].insert(labels.begin(), labels.end());
		}
	}
}

void Tracker::updateState()
{
	int currentTimestamp = mContext->getTimestamp();
	//Pour chacun des objets
	std::map<IObject*, bool> objectsToDelete; //Boolean at true say we should save the object and false delete without saving
	for(auto it = mObjectList.begin(); it != mObjectList.end(); ++it)
	{
		IObject* object = *it;		
		object->getIObjectModel()->simplifyModel();
		//LOGINFO(object->getObjectId() << " " <<object->getState() << " " << mContext->getTimestamp());

		switch(object->getState())
		{
		case ENTERING:
			// L'objet a une association avec la trame courante:
			if(object->getIObjectModel()->getLastTimestamp() == currentTimestamp)
			{
				//	On vérifie si l'objet touche encore à la bordure
				if(!Utils::OpenCV::IsOutsideROI(mContext->getMask(), object->getIObjectModel()->getLastBoundingBox()))
				{
					//		On change l'état à hypothèse (a0)
					object->setState(HYPOTHESIS);
					//	L'objet est là depuis plus de N trame
					if(object->getIObjectModel()->getLastTimestamp()-object->getIObjectModel()->getFirstTimestamp() > mContext->getTrackerAlgorithm().maxFrameHypothesis)
						object->setState(OBJECT); //		On change l'état à objet (a1)
				}	
				else
				{
					//std::cout << currentTimestamp << " Object " << object->getObjectId() << " is still entering\n";
					/*const auto& blobList = object->getIObjectModel()->getBlobs();
					if(blobList.size() > 1)
					{
						auto blobIt = blobList.rbegin();
						int lastArea = blobIt->second.getBoundingBox().area();
						++blobIt;
						int previousArea = blobIt->second.getBoundingBox().area();
						if(previousArea > lastArea)
							object->setState(LEAVING);
					}		*/			
				}
			}
			else 
			{
				if(object->getIObjectModel()->getLastTimestamp()-object->getIObjectModel()->getFirstTimestamp() > mContext->getTrackerAlgorithm().maxFrameHypothesis)
				{
					object->setState(LOST);
				}
				else // On supprime l'objet (a2)			
					objectsToDelete.insert(std::make_pair(object, true));
				
				
			}
		
			break;

		case HYPOTHESIS:
			// 1) L'objet a une association avec la trame courante:
			if(object->getIObjectModel()->getLastTimestamp() == currentTimestamp)
			{
				// L'objet est là depuis plus de N trame
				if(mContext->getTrackerAlgorithm().maxFrameHypothesis < object->getIObjectModel()->getLastTimestamp()-object->getIObjectModel()->getFirstTimestamp())
				{
					//On vérifie si il s'agit d'un objet perdu précédemment
					cv::Rect bb = object->getIObjectModel()->getBlobs().begin()->second.getBoundingBox();
					cv::Point hypothesisCentroid(bb.x+bb.width/2, bb.y+bb.height/2);
					int objectFirstTimestamp = object->getIObjectModel()->getFirstTimestamp();
					
					std::list<IObject*> lostObjectList = getObject(LOST);
					IObject* matchingObjects = nullptr;
					cv::Rect lastStableBoundingBox;
					int currentNbMatches = 0;
					//double medianDisplacement = 0;
					int minDistance = 0;
					for(auto lostIt = lostObjectList.begin(); lostIt != lostObjectList.end(); ++lostIt)
					{
						Object* lostObj = dynamic_cast<Object*>(*lostIt);
						if(lostObj)
						{
							cv::Rect tmpBB = lostObj->getObjectModel()->getLastStableBoundingBox();
							
							
							cv::Rect dilatedLostObjBB = Utils::OpenCV::dilate(tmpBB, 1.5);
							//cv::Point posCentroidLostObj(tmpBB.x+tmpBB.width/2, tmpBB.y+tmpBB.height/2);

							if((dilatedLostObjBB & bb).area() > 0)
							{
								//Vérifier critère spatial //aide pour les très petits objet
								/*if(abs(objectFirstTimestamp-(*lostIt)->getIObjectModel()->getLastTimestamp()) <=  15)
								{
									int tmpDist = cv::norm(hypothesisCentroid-posCentroidLostObj);
									if(currentNbMatches == 0 &&  tmpDist < minDistance && tmpDist < 20)
									{
										matchingObjects = (*lostIt);
										lastStableBoundingBox = tmpBB;
										//currentNbMatches = 0;
										minDistance = tmpDist;
									}
								}*/

								//Vérifier les points
								std::vector<double> pointsDelta = ObjectModel::getMatchingPointMovement(object->getIObjectModel(), (*lostIt)->getIObjectModel(), mContext);
								if(pointsDelta.size() >= 3 && pointsDelta.size() > currentNbMatches)
								{
									std::sort(pointsDelta.begin(), pointsDelta.end());
									double medianDisplacementTmp = pointsDelta[(int)std::floor(pointsDelta.size()/2.f)];
									//std::cout << "Median displacement:" << medianDisplacementTmp << std::endl;
									if(medianDisplacementTmp > 3) //TODO: Exteriorise param
									{
										currentNbMatches = pointsDelta.size();
										matchingObjects = (*lostIt);
										lastStableBoundingBox = tmpBB;
										//medianDisplacement = medianDisplacementTmp;
									}								
								}
							}

						}
						else
							LOGERROR("Hypothesis should be of Object Type");
							
					}
					if(matchingObjects && currentNbMatches > 3)
					{
						//1) On vérifie que les points ont bel et bien bougé. Si ils ont bougé, alors on fait une interpolation et on attache

						//cv::Rect lastBB = matchingObjects->getIObjectModel()->getLastBoundingBox();
						int lastTimestampMovement = matchingObjects->getIObjectModel()->getLastTimestamp();
						int firstTimestampMovement = object->getIObjectModel()->getFirstTimestamp();
						for(int j = lastTimestampMovement+1; j < firstTimestampMovement; ++j)
						{
							matchingObjects->getIObjectModel()->addMergeBlob(j, Blob(lastStableBoundingBox, STOPPED, mContext));
						}
						//std::cout << "LastStable: " << lastStableBoundingBox.x << " " << lastStableBoundingBox.y << " " <<  lastStableBoundingBox.width << " " << lastStableBoundingBox.height << std::endl;
						LOGINFO("Added " << firstTimestampMovement-lastTimestampMovement+1 << " static entry");

						matchingObjects->setState(OBJECT);						
						matchingObjects->getIObjectModel()->moveObjectModel(object->getIObjectModel());
						updateObjectReference(object, matchingObjects);
						objectsToDelete.insert(std::make_pair(object, false));
						//Merge
						LOGINFO("Merged " << matchingObjects->getObjectId() << " " << object->getObjectId() << " with " << currentNbMatches << " matches.");
					}
					else
						object->setState(OBJECT);//		On change l'état à objet (a3)
						
				}
				else
				{
					//Rien
				}
			}
			else //(a4)	
				objectsToDelete.insert(std::make_pair(object, false));

			break;

		case OBJECT:
			//	L'objet a t'il un blob courant ?
			if(object->getIObjectModel()->getLastTimestamp() == currentTimestamp)
			{
				//L'objet est-il sur la bordure ?
				if(Utils::OpenCV::IsOutsideROI(mContext->getMask(), object->getIObjectModel()->getLastBoundingBox()))
				{
					//Changer l'état à sortant (a6) et effectuer une estimation du nombre de trame Z nécessiare pour que l'objet sorte
					object->setState(LEAVING);
					//TODO: Estimate time left
				}
			}	
			else // Changer l'état de l'objet à perdu (a7)
			{
				Object* obj = dynamic_cast<Object*>(object);


				if(obj && isGoodObject(obj))
					object->setState(LOST);
				else
					objectsToDelete.insert(std::make_pair(object, false));
			}
			break;

		case LOST:
			// L'objet a une nouvelle observation
			// Il tombe à l'état Object. (a11)
			if(object->getIObjectModel()->getLastTimestamp() == currentTimestamp)
			{
				object->setState(OBJECT);
			}
			else
			{
				//L'objet est disparu depuis plus de M (150) trames ?
				int deltaFrame = currentTimestamp-object->getIObjectModel()->getLastTimestamp();
				if(deltaFrame > mContext->getTrackerAlgorithm().maximumLostFrame)
				{ //		On sauvegarde et supprime l'objet (a14)		
					
					objectsToDelete.insert(std::make_pair(object, true));
				}
				else
				{
					//Nothing
				}
			}
			break;

		case OBJECTGROUP:
			{
				ObjectGroup* og = dynamic_cast<ObjectGroup*>(object);
				//std::cout << "Updating object group state";
				if(og)
				{
					const std::vector<Object*>& objectList = og->getObjects();
					//	L'objet a t'il été matché cette trame ?
					if(object->getIObjectModel()->getLastTimestamp() == currentTimestamp)
					{

						//	L'objet contient t'il plus 1 objet ?
						if(objectList.size() == 0)//(a8)
						{
							LOGINFO("objectList.size() == 0");
							objectsToDelete.insert(std::make_pair(object, false));
							//On supprime le groupe
						}
						else if(objectList.size() == 1) //(a8)
						{
							LOGINFO("objectList.size() == 1");
							//On transforme en objet
							Object* obj = objectList[0];
							mObjectList.push_back(obj);
							updateObjectReference(og, obj);	
							og->clearObjectList();
							//On supprime le groupe
							objectsToDelete.insert(std::make_pair(object, false));
						}
						else //Oui
						{
							//		Rien (a13)
						}
					}
					else //		On sépare les objets du groupe et on les met dans les liste des objets perdus (a15)
					{
						LOGINFO("Group leaving " << og->getObjectId());
						//Permet d'ajouter les blobs qui ne coreesponde à aucun objet aux objets existants
						bool isLeaving =  Utils::OpenCV::IsOutsideROI(mContext->getMask(), object->getIObjectModel()->getLastBoundingBox());						
						og->getObjectModelGroup()->addUnmatchedGroupBlobToExistingObjects();

						for(auto it = objectList.begin(); it != objectList.end(); ++it)
						{
							Object* obj = *it;
							if(isGoodObject(obj))
							{
								LOGINFO(obj->getObjectId() << " is good object");
								if(isLeaving)
								{									
									objectsToDelete.insert(std::make_pair(obj, true));
									obj->setState(OBJECT);
								}
								else
								{
									obj->setState(LOST);
								}
							}
							else
							{
								LOGINFO(obj->getObjectId() << " is a bad object");
								objectsToDelete.insert(std::make_pair(obj, false));
								obj->setState(LOST);
							}
							mObjectList.push_back(obj); //Must be in the object list to be deleted with objectsToDelete
							
						}
						og->clearObjectList();
						objectsToDelete.insert(std::make_pair(og, false));
					}	
				}
				else			
					LOGASSERT(false,"Wrong type of object with type 'ObjectGroup'");
			}
			break;

		case INGROUP:
			LOGASSERT(false, "Object should never be in this state in there. They should be remove from the list");
			break;

		case LEAVING:
			//	L'objet a t'il été matché cette trame ?
			if(object->getIObjectModel()->getLastTimestamp() == currentTimestamp)
			{
				//		Est-ce que l'objet est encore dans la zone 'Leaving' ?
				// Oui:
				
				bool isEntering = false;
				
				const auto& blobs = object->getIObjectModel()->getBlobs();
				if(blobs.size() > 1)
				{
					auto itBlob = blobs.rbegin();
					const auto& currentBlob = (*itBlob).second.getBoundingBox();
					isEntering = !Utils::OpenCV::IsOutsideROI(mContext->getMask(), currentBlob);
				}

				if(mContext->getTrackerAlgorithm().verifyReEnteringObject && isEntering)
				{
					//Si le blob courant ne match pas avec le modèle d'apparance précédent, on sépare les tracks
					std::vector<IObject*> newObjects;
					object->getIObjectModel()->handleLeaving(newObjects);
					if(newObjects.empty())
						object->setState(OBJECT);
					else
					{
						////object->setState(LOST);
						objectsToDelete.insert(std::make_pair(object, true));
						

						if(newObjects.size() == 1)
						{
							updateObjectReference(object,newObjects[0]);
							mObjectList.push_back(newObjects[0]);
						}
						else
							LOGWARNING("handleLeaving does not support multiple object output");
					}
				}
				else if(isEntering)
					object->setState(OBJECT);
			}
			else//		On sauvegarde et on supprime l'objet (a9)
				objectsToDelete.insert(std::make_pair(object, true));

			

			break;
		case DELETED:
			{
				objectsToDelete.insert(std::make_pair(object, false));
			}
			break;

		case SAVEANDELETE:
			objectsToDelete.insert(std::make_pair(object, true));
			break;

		case UNDEFINED:
			LOGERROR("Undefined state. Object was not initialised properly" << mContext->getTimestamp());
			break;

		default:
			LOGERROR("Unknown state");
		}
	}

	std::list<IObject*> objectToKeep;
	//Save and delete the objects
	for(auto it = mObjectList.begin(); it != mObjectList.end(); ++it)
	{
		auto objectIt = objectsToDelete.find(*it);
		if(objectIt != objectsToDelete.end())
		{
			IObject* obj = objectIt->first;
			//LOGINFO("Delete object " << obj->getObjectId());
			if(objectIt->second)
			{
				saveGoodObjects(obj);
			}

			auto existingRef = mObjectToBlobLabel[mCurrentFrameIdx].find(obj);
			if(existingRef != mObjectToBlobLabel[mCurrentFrameIdx].end())
			{
				LOGERROR("Dangerous trace of deleted object at " << mContext->getTimestamp());					
				mObjectToBlobLabel[mCurrentFrameIdx].erase(existingRef);
			}
			for(auto it2 = mBlobLabelToObject[mCurrentFrameIdx].begin(); it2 != mBlobLabelToObject[mCurrentFrameIdx].end(); )
			{
				if(it2->second == obj)
				{
					it2 = mBlobLabelToObject[mCurrentFrameIdx].erase(it2);
					LOGERROR("Deleted dangerous object trace!");
				}
				else
					++it2;
			}
			//LOGINFO("Simulated delete of " << obj->getObjectId());
			delete obj;			
		}
		else		
			objectToKeep.push_back(*it);
		
	}
	mObjectList = objectToKeep;
}


bool Tracker::isGoodObject(Object* obj)
{
	bool success = false;

	int nbFrame = obj->getIObjectModel()->getBlobs().size();
	bool isShadow = false;
	bool verifyShadow = mContext->getTrackerAlgorithm().removeBirdShadows;
	if(verifyShadow)
	{
		isShadow = obj->isShadow();
		if(isShadow)					
			LOGWARNING("Object " << obj->getObjectId() << " not saved because its a shadow !");
	}

	bool enoughFrame = nbFrame > 10;
	if(!enoughFrame)
		LOGWARNING("Object " << obj->getObjectId() << " not saved because it only appear in " << nbFrame);

	bool hasMinimalMovement = obj->getIObjectModel()->hasMinimalMovement(30); // extérioriser. Valuer par défaut = 60
	if(!hasMinimalMovement)
		LOGWARNING("Object " << obj->getObjectId() << " not saved because there's not enough movement");
	success = !isShadow && enoughFrame && hasMinimalMovement;


	return success;
}

void Tracker::saveGoodObjects(IObject* obj)
{
	Object* objectToSave = dynamic_cast<Object*>(obj);

	if(!objectToSave)
		LOGERROR("An object group should never be save. This state is invalid");

	//TODO: Threshold objet moins important
	if(objectToSave && isGoodObject(objectToSave))
	{


		int maximumGap = 3;
		objectToSave->getObjectModel()->interpolateMissingFrame(maximumGap);
		objectToSave->getObjectModel()->correctGroupObservation();

		mContext->getObjectDatabaseManager()->save(objectToSave);		
	}
}


std::list<IObject*> Tracker::getObject(int state) const
{
	return getObject(mObjectList, state);
}

std::list<IObject*> Tracker::getObject(const std::list<IObject*>& objectList, int state) const
{
	std::list<IObject*> objectListByState;
	for(auto objectIt = objectList.begin(); objectIt != objectList.end(); ++objectIt)
	{
		if((*objectIt)->getState() & state)
			objectListByState.push_back((*objectIt));
	}
	return objectListByState;
}

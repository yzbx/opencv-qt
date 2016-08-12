#include "Object.h"
#include "ApplicationContext.h"
#include "Tracker.h"
#include "IFeatureDetectorExtractorMatcher.h"

Object::Object(ApplicationContext* context)
: IObject(context, cv::Scalar(rand()%255,rand()%255, rand()%255))
, mModel(context, this)
{
	
}


bool Object::isShadow()
{
	const auto& blobs = mModel.getBlobs();
	int borderSize = getContext()->getTracker()->getFeatureManager()->getNoPointBorderSize();
	int r = getContext()->getMask().rows-borderSize;
	int c = getContext()->getMask().cols-borderSize;
	
	cv::Rect_<float> middlePart(borderSize,borderSize, c-borderSize,r-borderSize);

	int nbBlobWithFeatures = 0;
	for(auto it = blobs.begin(); it != blobs.end(); ++it)
	{
		const cv::Rect_<float>& r = it->second.getBoundingBox();
		cv::Rect_<float> intersect = middlePart&r;
		if(intersect.area() > 200) //Minimum
			++nbBlobWithFeatures;
	}
	double avgTrackBlob = 100;
	LOGINFO(mModel.getNumberRemovedFeatures() << " removed features and " << mModel.getTracks().size() << " tracks.");
	if(nbBlobWithFeatures > 0)
	{
		avgTrackBlob = ((double)(mModel.getTracks().size() + mModel.getNumberRemovedFeatures())) / (double) nbBlobWithFeatures;
		LOGINFO(this->getObjectId() << " has an average of " << avgTrackBlob << " feature points.");
	}
	
	return avgTrackBlob < 10; //TODO: Move this to param featureless object threshold
}

void Object::draw(cv::Mat& image) 
{
	if(!mModel.getBlobs().empty())
	{
		ObjectState state = mModel.getBlobs().rbegin()->second.getState();
		if(state != INGROUP && state != OBJECTGROUP)
		{
			if(getContext()->getDrawingFlags()->mDrawBoundingBox)
			{
				std::string info(getObjectId());
				//Affichage de la vitesse des véhicules
				/*if(mModel.getBlobs().size() > 1)
				{
					auto it = mModel.getBlobs().rbegin();
					int timeB = it->first;
					cv::Point2f centroidB = it->second.getProjectedCentroid();
					int iter = 1;

					while(iter < 4 && iter < mModel.getBlobs().size())
					{
						++it;
						++iter;
					}

				
					int timeA = it->first;
					cv::Point2f centroidA = it->second.getProjectedCentroid();

					
					float deltaT = (float)(timeB-timeA)/getContext()->getFPS();
					float distPixel = cv::norm(centroidB-centroidA);

					float distM = distPixel*getContext()->getNbMeterByPixel();

					//float vMParS = distM/deltaT;
					std::stringstream ss;
					ss << info;
					ss << "@";
					ss << int(vMParS*3.6f);
					ss << "km/h";
					info = ss.str();
				}*/


				



				cv::Rect_<float> r = mModel.getLastBoundingBox();
				cv::rectangle(image, r, getColor(), 2);

				cv::rectangle(image, cv::Rect_<float>(r.x - 5, r.y - 7, 12*info.size(), 14), cv::Scalar(0,0,0), CV_FILLED);
				cv::putText(image, info, cv::Point(r.x-5, r.y+4), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255,255,255),1,8, false);


				std::string stateStr = getStateRepresentativeString(getState());
				cv::Point startPt(r.x-5, r.height+r.y);
				cv::rectangle(image, cv::Rect_<float>(startPt.x, startPt.y-12, 12*stateStr.size(), 14), cv::Scalar(0,0,0), CV_FILLED);
				cv::putText(image, stateStr, startPt, cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255,255,255),1,8, false);

			}

			if(getContext()->getDrawingFlags()->mDrawCentroid)
			{
				const std::map<int, Blob>& blobs = mModel.getBlobs();
				if(blobs.size() > 1)
				{
					auto it = blobs.begin();
					cv::Point2f lastPt = it->second.getCentroid();
					++it;
					for(; it != blobs.end(); ++it)
					{
						cv::Point2f currentPt = it->second.getCentroid();
						cv::line(image, lastPt, currentPt, getColor(), 3);
						lastPt = currentPt;

					}
				}
			}

		}
		if(getContext()->getDrawingFlags()->mDrawTrajectories)
		{
			const auto& trackList = mModel.getTracks();
			for(auto t = trackList.begin(); t != trackList.end(); ++t)
			{
				std::map<int, FeaturePoint>& kpList = (*t)->getPointList();
				if(kpList.size() > 1)
				{
					std::map<int, FeaturePoint>::iterator lastKpIt = kpList.begin();
					auto newKpIt = lastKpIt;
					++newKpIt;
					for(; newKpIt != kpList.end(); ++newKpIt)
					{
						cv::line(image, lastKpIt->second.getPoint(), newKpIt->second.getPoint(),getColor());
					}
				}
			}
		}
	}

}


bool Object::getExpectedPosition(int timestamp, cv::Point2f& expectedPosition)
{
	return mModel.getExpectedPosition(timestamp, expectedPosition);
}


//modelList contains the list of object splitting. We should only keep one
//outSplittedModel contains the unmatched models
// return selected object
ObjectModel* Object::split(const std::vector<ObjectModel*>& modelList, std::vector<ObjectModel*>& outSplittedModel)
{
	int maxPoints = 0;
	int index = 0;
	
	for(unsigned int i = 0; i < modelList.size(); ++i)
	{		
		int nbPoints = ObjectModel::getMatchingPointNumber(&mModel, modelList[i], getContext());
		if(nbPoints > maxPoints)
		{
			maxPoints = nbPoints;
			index = i;
		}
		//std::cout << i << " " << nbPoints << std::endl;
	}
	//If there is no point, they must be border object. In this case, it would be difficult to know anyway which one is more relevant
	ObjectModel* keep = modelList[index];
	//mModel.split(modelList);

	for(unsigned int i = 0; i < modelList.size(); ++i)
	{	
		if(i != index)
		{
			//extract matching model
			mModel.extractObjectModel(modelList[i], getContext());
			modelList[i]->updateInGroupBlobs();
			outSplittedModel.push_back(modelList[i]);
		}
	}



	mModel.moveObjectModel(keep); //This is the new blob model
	mModel.updateInGroupBlobs();
	return keep;
}
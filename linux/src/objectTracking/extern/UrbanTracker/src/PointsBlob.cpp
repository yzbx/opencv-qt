#include "PointsBlob.h"
#include "ApplicationContext.h"
#include "Logger.h"
void PointsBlob::getKpDesc(int blobId, std::vector<FeaturePoint>& kp, cv::Mat& desc) 
{
	auto it = mBlobIdToKpDesc.find(blobId);
	if(it != mBlobIdToKpDesc.end())
	{
		kp = it->second.mKeyPoints;
		desc = it->second.mDescriptors;
	}
}



void PointsBlob::calculatePointBlobAssociation(const cv::Mat& mask, ApplicationContext* c)
{	
	for(unsigned int i = 0; i < mkeyPoints.size(); ++i)
	{
		const auto& pt = mkeyPoints[i].pt;
		int val = Utils::OpenCV::GetValue(mask, pt);
		//Gestion des cas situé sur les bordures
		if(val == 0)
		{
			val = Utils::OpenCV::GetValue(mask, cv::Point((int)ceil(pt.x),(int)ceil(pt.y)));
			if(val == 0)
				val = Utils::OpenCV::GetValue(mask, cv::Point((int)floor(pt.x),(int)floor(pt.y)));
			if(val == 0)
				val = Utils::OpenCV::GetValue(mask, cv::Point((int)floor(pt.x),(int)ceil(pt.y)));
			if(val == 0)
				val = Utils::OpenCV::GetValue(mask, cv::Point((int)ceil(pt.x),(int)floor(pt.y)));
			if(val == 0)
				LOGERROR("Le point ne fait pas partie d'un blob");
		}

		mBlobId.push_back(val);
		mBlobIdToKpDesc[val].mKeyPoints.push_back(FeaturePoint(mkeyPoints[i].pt,c));
		mBlobIdToKpDesc[val].mDescriptors.push_back(mDescriptors.row(i));
	}
}	

void PointsBlob::reset()
{
	mkeyPoints.clear();
	mBlobIdToKpDesc.clear();
	mBlobId.clear();
}

void PointsBlob::updateBlobId(int oldId, int newId)
{
	mBlobIdToKpDesc[newId].mKeyPoints.insert(mBlobIdToKpDesc[newId].mKeyPoints.end(), mBlobIdToKpDesc[oldId].mKeyPoints.begin(), mBlobIdToKpDesc[oldId].mKeyPoints.end());
	mBlobIdToKpDesc[newId].mDescriptors.push_back(mBlobIdToKpDesc[oldId].mDescriptors);
	mBlobIdToKpDesc.erase(oldId);
	for(auto it = mBlobId.begin(); it != mBlobId.end(); ++it)
	{
		if(*it == oldId)
			*it = newId;
	}
}
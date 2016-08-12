#include "FrameAssociation.h"
#include "OpenCVHelpers.h"
#include "Logger.h"
#include "IObject.h"


FrameAssociation::FrameAssociation(const std::map<int, BlobRect>& oldBlobAssociation, const std::map<int, BlobRect>& newBlobAssociation, std::map<std::pair<int,int>, int> nbMatches)
{
	updateAssociation(oldBlobAssociation, newBlobAssociation, nbMatches);
}




void FrameAssociation::updateAssociation(const std::map<int, BlobRect>& oldBlobAssociation, const std::map<int, BlobRect>& newBlobAssociation, std::map<std::pair<int,int>, int> nbMatches)
{
	//Point matching
	
	std::map<int, std::set<int>> oldBlobToNewBlob;
	std::map<int, std::set<int>> newBlobToOldBlob;
	std::set<int> matchedOldBlob;
	std::set<int> matchedNewBlob;

	for(auto it = nbMatches.begin(); it != nbMatches.end(); ++it)
	{
		if(it->second >= 4)
		{
			oldBlobToNewBlob[it->first.first].insert(it->first.second);
			newBlobToOldBlob[it->first.second].insert(it->first.first);
			matchedOldBlob.insert(it->first.first);
			matchedNewBlob.insert(it->first.second);
		}
	}
	

	std::map<int, std::pair<int, int>> oldToNewBestMatch;
	for(auto oldBlobIt = oldBlobAssociation.begin(); oldBlobIt != oldBlobAssociation.end(); ++oldBlobIt)
	{
		int oldBlobId = (*oldBlobIt).first;
		if(matchedOldBlob.find(oldBlobId) == matchedOldBlob.end())
		{
			bool matchedOld = false;
			int previousArea = 0;
			for(auto newBlobIt = newBlobAssociation.begin(); newBlobIt != newBlobAssociation.end(); ++newBlobIt)
			{
				int newBlobId = (*newBlobIt).first;
				int area = BlobRect::getAreaIntersection((*oldBlobIt).second, (*newBlobIt).second);
				if(area > 0)
				{				
					if(area > previousArea)
					{
						oldToNewBestMatch[oldBlobId] = std::make_pair(newBlobId,area);
						previousArea = area;
					}
					matchedOld = true;
				}
			} //1-0
			if(!matchedOld)
				mNoMatchBlobs.insert(oldBlobId);
			else
			{
				int newBlobId = oldToNewBestMatch[oldBlobId].first;
				oldBlobToNewBlob[oldBlobId].insert(newBlobId);
				newBlobToOldBlob[newBlobId].insert(oldBlobId);
				matchedOldBlob.insert(oldBlobId);
				matchedNewBlob.insert(newBlobId);
			}
		}
	}

	std::map<int, std::pair<int, double>> newToOldBestMatch;
	//0-1
	for(auto it = newBlobAssociation.begin(); it != newBlobAssociation.end(); ++it)
	{
		int newBlobId = it->first;
		//TODO: Calculer ceux-ci tout en évitant de dupliquer les associations
		if(matchedNewBlob.find(newBlobId) == matchedNewBlob.end())	
		{
			bool matchedOld = false;
			int previousArea = 0;
			for(auto oldBlobIt = oldBlobAssociation.begin(); oldBlobIt != oldBlobAssociation.end(); ++oldBlobIt)
			{
				int oldBlobId = oldBlobIt->first;
				
				int area = BlobRect::getAreaIntersection((*oldBlobIt).second, it->second);
				if(area > 0)
				{				
					if(area > previousArea)
					{
						newToOldBestMatch[newBlobId] = std::make_pair(oldBlobId,area);
						previousArea = area;
					}
					matchedOld = true;
				}
			}
			if(matchedOld)
			{
				int oldBlobId = newToOldBestMatch[newBlobId].first;
				oldBlobToNewBlob[oldBlobId].insert(newBlobId);
				newBlobToOldBlob[newBlobId].insert(oldBlobId);
				matchedOldBlob.insert(oldBlobId);
				matchedNewBlob.insert(newBlobId);
			}
			else
				mNewBlobs.insert(newBlobId);
		}
	}
	
	for(auto oldBlobIt = oldBlobToNewBlob.begin(); oldBlobIt != oldBlobToNewBlob.end(); ++ oldBlobIt)
	{		
		int oldBlobId = (*oldBlobIt).first;

		if((*oldBlobIt).second.size()==1)
		{			
			int newBlobId = *(*oldBlobIt).second.begin();
			if(newBlobToOldBlob[newBlobId].size() == 1)
			{
				mOneToOne.insert(std::make_pair(oldBlobId, newBlobId));	
				matchedOldBlob.insert(oldBlobId);
				matchedNewBlob.insert(newBlobId);
			}
			else //N-1		
				mNToOne.insert(std::make_pair(newBlobToOldBlob[newBlobId],  newBlobId));
		}
		else //1-N		
			mOneToN.insert(std::make_pair(oldBlobId,(*oldBlobIt).second));		
	}
}
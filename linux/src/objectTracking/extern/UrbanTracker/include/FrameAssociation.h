#ifndef FRAME_ASSOCIATION_H
#define FRAME_ASSOCIATION_H

#include <opencv2/opencv.hpp>
#include <map>
#include <vector>
#include "BlobRect.h"

class IObject;

	
	struct CentroidDiagonal
	{
		CentroidDiagonal(cv::Point2f c, double d, bool u)
		: centroid(c)
		, diagonal(d)
		, used(u)		
		{

		}
		cv::Point2f centroid;
		double diagonal;
		bool used;
	};


class FrameAssociation
{
public:
	FrameAssociation(const std::map<int, BlobRect>& oldBlobAssociation, const std::map<int, BlobRect>& newBlobAssociation, std::map<std::pair<int,int>, int> nbMatches);
	const std::set<int>& getNewBlob() const {return mNewBlobs;}
	const std::set<int>& getUnmatchedBlob() const {return mNoMatchBlobs;}
	const std::set<std::pair<int, int>>& getDirectMatchBlob() const {return mOneToOne;}
	const std::set<std::pair<int, std::set<int>>>& getBlobSplit() const {return mOneToN;}
	const std::set<std::pair<std::set<int>, int>>& getBlobMerge() const {return mNToOne;}



private:
	void updateAssociation(const std::map<int, BlobRect>& oldBlobAssociation, const std::map<int, BlobRect>& newBlobAssociation, std::map<std::pair<int,int>, int> nbMatches);
	
	

	std::set<int> mNewBlobs;	 // 0-1
	std::set<int> mNoMatchBlobs; // 1-0
	std::set<std::pair<int, int>> mOneToOne; //1-1
	std::set<std::pair<int, std::set<int>>> mOneToN; //1-N
	std::set<std::pair<std::set<int>, int>> mNToOne; //N-1
};


#endif
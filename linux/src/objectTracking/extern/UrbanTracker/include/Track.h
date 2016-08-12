#ifndef TRACK_H
#define TRACK_H

#include "FeaturePoint.h"
#include <map>

class Track
{
public:
	Track(const cv::Mat& descriptor)
	: mDescriptor(descriptor)
	{

	}

	void updateDescriptor(const cv::Mat& descriptor)
	{
		mDescriptor = descriptor.clone();
	}


	void addPoint(int timestamp, const FeaturePoint& pt)
	{
		mPointList[timestamp] = pt;
	}

	const cv::Mat& getDescriptor() const {return mDescriptor;}

	const std::map<int, FeaturePoint>& getPointList() const { return mPointList;}
	std::map<int, FeaturePoint>& getPointList() { return mPointList;}

	unsigned int getFirstTimestamp() const;
	unsigned int getLastTimestamp() const;
	bool getProjectedSpeed(int timestamp, cv::Point2f& speed)
	{
		bool success = false;
		if(mPointList.size() > 1)
		{
			success = true;
			cv::Point2f posA, posB;
			auto firstElement = mPointList.begin();
			auto lastElement = mPointList.rbegin();
			int timeDelta = 0;
			if(lastElement->first < timestamp) //timestamp is before all observation
			{
				posB = lastElement->second.getProjectedPoint();
				int timeB = lastElement->first;
				++lastElement; // This is a reverse iterator
				posA = lastElement->second.getProjectedPoint();
				int timeA = lastElement->first;
				timeDelta = timeB-timeA;
				speed = (posB-posA);
			}
			else if(firstElement->first > timestamp)
			{
				posB = firstElement->second.getProjectedPoint();
				int timeB = firstElement->first;
				--firstElement;
				posA = firstElement->second.getProjectedPoint();
				int timeA = firstElement->first;
				timeDelta = timeB-timeA;
				speed = (posB-posA);
			}
			else //Point is part of the current position list
			{
				auto upper = mPointList.lower_bound(timestamp);
				int timeB = upper->first;
				posB = upper->second.getProjectedPoint();
				int timeDelta;
				if(upper == mPointList.begin())							
					++upper;
				else				
					--upper;				
				
				int timeA = upper->first;
				posA = upper->second.getProjectedPoint();
				speed = (posB-posA);
				timeDelta = timeB-timeA;
			}
					
			speed.x /= timeDelta;
			speed.y /= timeDelta;
			success = true;
		}
		else
			speed = cv::Point2f(0,0);

		return success;
		
		
		

	}

private:
	std::map<int, FeaturePoint> mPointList;
	cv::Mat mDescriptor;
};

#endif
#include "Track.h"



unsigned int Track::getFirstTimestamp() const
{
	return mPointList.empty() ? 0 : mPointList.begin()->first; //Map sorté en ordre croissant
}

unsigned int Track::getLastTimestamp() const
{
	return mPointList.empty() ? 0 : mPointList.rbegin()->first; 
}
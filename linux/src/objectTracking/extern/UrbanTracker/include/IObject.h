#ifndef IOBJECT_H
#define IOBJECT_H

#include <string>
#include "ObjectState.h"
#include <opencv2/opencv.hpp>
#include "Logger.h"
//Object interface
class ApplicationContext;
class ObjectModel;
class IObjectModel;

class IObject
{
public:
	IObject(ApplicationContext* context, cv::Scalar color = cv::Scalar(0,0,255));
	virtual ~IObject(){ /*LOGINFO("Destructor of " << getObjectId());*/}

	ApplicationContext* getContext() const { return mContext;}
	void setState(ObjectState state) { mState = state;}
	void setObjectId(const std::string& id) { mObjectId = id;}
	ObjectState getState() const { return mState;}
	const std::string& getObjectId() const { return mObjectId;}
	const cv::Scalar& getColor() const { return mColor;}
	void setColor(cv::Scalar& color) { mColor = color;}
	virtual IObjectModel* getIObjectModel() = 0;

	virtual void draw(cv::Mat& image) = 0;
	


private:
	ApplicationContext* mContext;
	ObjectState mState;
	std::string mObjectId;
	cv::Scalar mColor;
	
};

#endif

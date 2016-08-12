#include "ObjectGroup.h"
#include "ApplicationContext.h"
#include "Logger.h"
#include "Object.h"
ObjectGroup::ObjectGroup(ApplicationContext* context)
: IObject(context, cv::Scalar(0,0,255))
, mModel(new ObjectModelGroup(context, this))
{
	setState(OBJECTGROUP);
}

void ObjectGroup::clearObjectList()
{
	mModel->clearObjectModel();
	mObjectToObjectModel.clear();
	mObjectModelToObject.clear();
	mObjectList.clear();
}

void ObjectGroup::removeObject(Object* o)
{
	for(auto it = mObjectList.begin(); it != mObjectList.end(); )
	{
		if(*it == o)
			it = mObjectList.erase(it);
		else
			++it;
	}
	mObjectToObjectModel.erase(o);
	mObjectModelToObject.erase(o->getIObjectModel());
}

ObjectGroup::~ObjectGroup()
{
	delete mModel;
	LOGASSERT(mObjectList.empty(), "Object should be release from group before deletion");
}


void ObjectGroup::addObject(IObject* object)
{
	ObjectGroup* group = dynamic_cast<ObjectGroup*>(object);
	if(group)
	{
		std::vector<Object*>& objectList = group->getObjects();
		for(auto it = objectList.begin(); it != objectList.end(); ++it)		
			addObject(*it);
		
		objectList.clear();
	}
	else
	{
		Object* obj = dynamic_cast<Object*>(object);
		if(obj)
			addObject(obj);
		else
			LOGERROR("Cast fail: Object type should only be Object*");
	}

	
}


void ObjectGroup::addObject(Object* object)
{
	mObjectList.push_back(object);
	object->setState(INGROUP);
	mModel->addObjectModel(object->getObjectModel());
	mObjectToObjectModel.insert(std::make_pair(object, object->getObjectModel()));
	mObjectModelToObject.insert(std::make_pair(object->getObjectModel(), object));
	std::string name;
	for(unsigned int i = 0; i < mObjectList.size(); ++i)	
		name += mObjectList[i]->getObjectId() + ", ";
	setObjectId(name);
}

void ObjectGroup::draw(cv::Mat& image)
{
	
	if(getContext()->getDrawingFlags()->mDrawBoundingBox)
	{
		cv::Rect_<float> r = mModel->getLastBoundingBox();

		cv::rectangle(image, r, getColor(), 2);

		cv::rectangle(image, cv::Rect_<float>(r.x - 5, r.y - 7, 12*getObjectId().size(), 14), cv::Scalar(0,0,0), CV_FILLED);
		cv::putText(image, getObjectId(), cv::Point(r.x-5, r.y+4), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255,255,255),1,8, false);

		std::string stateStr = getStateRepresentativeString(getState());
		cv::Point startPt(r.x-5, r.height+r.y);
		cv::rectangle(image, cv::Rect_<float>(startPt.x, startPt.y-12, 12*stateStr.size(), 14), cv::Scalar(0,0,0), CV_FILLED);
		cv::putText(image, stateStr, startPt, cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255,255,255),1,8, false);
	}

	//We don't want to show the Centroid of the blob
	/*if(getContext()->getDrawingFlags()->mDrawCentroid)
	{
		const std::map<int, Blob>& blobs = mModel->getBlobs();
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
	}*/
	for(auto objectIt = mObjectList.begin(); objectIt != mObjectList.end(); ++objectIt)	
		(*objectIt)->draw(image);
	
}

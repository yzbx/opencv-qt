#ifndef OBJECT_GROUP_H
#define OBJECT_GROUP_H

#include <vector>
#include <map>
#include "IObject.h"
#include "ObjectModelGroup.h"

class ObjectModelGroup;
class IObjectModel;
class Object;


class ObjectGroup: public IObject
{
public:
	ObjectGroup(ApplicationContext* context);
	~ObjectGroup();
	void addObject(IObject* object);
	void addObject(Object* object);
	ObjectModelGroup* getObjectModelGroup() { return mModel; }
	//ObjectModel& getIObjectModel() {return mModel;}
	virtual IObjectModel* getIObjectModel() { return mModel; }
	const std::vector<Object*>& getObjects() const { return mObjectList;}
	std::vector<Object*>& getObjects() { return mObjectList;}

	void clearObjectList();
	virtual void draw(cv::Mat& image);
	void removeObject(Object* o);
private:
	
	ObjectModelGroup* mModel;
	std::vector<Object*> mObjectList;
	std::map<Object*, IObjectModel*> mObjectToObjectModel;
	std::map<IObjectModel*, Object*> mObjectModelToObject;
};

#endif
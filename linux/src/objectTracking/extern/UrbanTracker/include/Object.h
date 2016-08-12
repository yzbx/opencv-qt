#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include "ObjectState.h"
#include "ObjectModel.h"
#include <opencv2/opencv.hpp>
#include "IObject.h"
#include <vector>
class ApplicationContext;

class Object: public IObject
{
public:
	Object(ApplicationContext* context, ObjectModel model);
	Object(ApplicationContext* context);
	IObjectModel* getIObjectModel() {return &mModel;}
	ObjectModel* getObjectModel() { return &mModel; }
	bool isShadow();
	virtual void draw(cv::Mat& image);
	virtual bool getExpectedPosition(int frame, cv::Point2f& expectedPosition);
	ObjectModel* split(const std::vector<ObjectModel*>& modelList, std::vector<ObjectModel*>& outSplittedModel);
private:
	
	ObjectModel mModel;
};

#endif
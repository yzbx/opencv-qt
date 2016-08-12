#include "IObject.h"
#include "ApplicationContext.h"

IObject::IObject(ApplicationContext* context, cv::Scalar color)
: mContext(context)
, mState(UNDEFINED)
, mObjectId(context->getUniqueId())
, mColor(color)
{

}

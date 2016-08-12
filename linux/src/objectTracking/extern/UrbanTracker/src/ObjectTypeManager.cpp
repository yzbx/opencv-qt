#include "ObjectTypeManager.h"
#include "StringHelpers.h"


ObjectTypeManager::ObjectTypeManager()
{
	addDefaultType();
}

ObjectTypeManager::~ObjectTypeManager()
{

}

void ObjectTypeManager::addDefaultType()
{
	addType(0, "unknown");
	addType(1, "car");
	addType(2, "pedestrians");
	addType(3, "motorcycle");
	addType(4, "bicycle");
	addType(5, "bus");
	addType(6, "truck");
}

void ObjectTypeManager::addType(const std::string& type)
{
	if(getIdx(type) == -1)
	{
		int key = getUnusedIndex(); 
		addType(key, type);
	}
}

int ObjectTypeManager::getUnusedIndex()
{
	return mTypeMap.rbegin()->first + 1;
}


void ObjectTypeManager::addType(int key, const std::string& type )
{
	if(getIdx(type) == -1)
	{
		mTypeMap.insert(std::make_pair(key, type));
		mReverseTypeMap.insert(std::make_pair(type, key));
	}
}

int ObjectTypeManager::getIdx(const std::string& typeName)
{

	auto it = mReverseTypeMap.find(typeName);
	return it != mReverseTypeMap.end() ? it->second : -1;
}

std::string ObjectTypeManager::getName(int key)
{
	auto it = mTypeMap.find(key);
	return it != mTypeMap.end() ? it->second : Utils::String::toString(key);
}

const std::map<int, std::string>& ObjectTypeManager::getTypeList() const
{
	return mTypeMap;
}

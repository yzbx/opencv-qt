#ifndef OBJECT_TYPE_MANAGER_H
#define OBJECT_TYPE_MANAGER_H

#include <map>
#include <string>


class ObjectTypeManager
{
public:
	ObjectTypeManager();
	~ObjectTypeManager();
	void addType(const std::string& type);
	void addType(int key, const std::string& type); 
	void clear()
	{
		mTypeMap.clear();
		mReverseTypeMap.clear();
		addDefaultType();
	}
	const std::map<int, std::string>& getTypeList() const;
	
	std::string getName(int key);
	int getIdx(const std::string& typeName);
	
private:
	
	int getUnusedIndex();
	void addDefaultType();
	std::map<int, std::string> mTypeMap;
	std::map<std::string, int> mReverseTypeMap;
};

#endif

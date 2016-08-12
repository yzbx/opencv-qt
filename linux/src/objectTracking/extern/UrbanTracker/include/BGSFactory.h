#ifndef BGS_FACTORY_H
#define BGS_FACTORY_H

#include <string>
#include <map>
class IBGS;

typedef std::map<std::string, IBGS*(*)()> map_type;
template<typename T> IBGS* createInstance() { return new T; }
class BGSFactory
{
public:
	BGSFactory();
	~BGSFactory();
	IBGS* getBGSInstance(const std::string& bgsMethodName) const;
	std::string getBGSMethodList() const;
	
private:
	

	map_type nameToInstanceMap;

	

};


#endif
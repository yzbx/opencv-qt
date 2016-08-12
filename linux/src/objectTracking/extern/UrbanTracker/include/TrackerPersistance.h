#ifndef TRACKER_PERSISTANCE_H
#define TRACKER_PERSISTANCE_H

#include "SQLiteManager.h"

#include <string>

class ApplicationContext;
class Object;

class TrackerPersistance : public SQLiteManager
{
public:
	TrackerPersistance(ApplicationContext* ctx, const std::string& trackerOutputName);
	virtual ~TrackerPersistance();
	bool init();
	bool save(Object* object);

private:
	bool saveTypeManager();
	bool createTables();
	bool dropTables();
	ApplicationContext* mContext;
	unsigned int mCurrentIdObject;
	bool mIsInit;

};
#endif
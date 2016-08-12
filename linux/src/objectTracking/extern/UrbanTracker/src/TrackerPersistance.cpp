#include "TrackerPersistance.h"
#include "ApplicationContext.h"
#include "Object.h"
#include "ObjectTypeManager.h"
#include "Logger.h"
#include "Blob.h"

TrackerPersistance::TrackerPersistance(ApplicationContext* ctx, const std::string& trackerOutputName)
: SQLiteManager(trackerOutputName) 
, mContext(ctx)
, mCurrentIdObject(0)
, mIsInit(false)
{
	ctx->setObjectDatabaseManager(this);
}

TrackerPersistance::~TrackerPersistance()
{

}

bool TrackerPersistance::init()
{
	mIsInit = true;
	mIsInit &= dropTables();
	mIsInit &= createTables();
	mIsInit &= saveTypeManager();
	
	return mIsInit;
}


bool TrackerPersistance::save(Object* o)
{
	if(!mIsInit)
	{
		init();
	}
	
	bool success = beginTransaction();
	

	unsigned int objectId = mCurrentIdObject++;
	
	LOGINFO("Saving " << o->getObjectId() << " as " << objectId);
	//We add an entry to the Objects Table

	//TODO: This should come from the type manager when we will have added classification
	std::stringstream objectStream;
	objectStream << "insert into objects (object_id, road_user_type, description) values ('" << objectId << "', '" <<  "0" << "', '" << "" << "');";
	success &= executeStatement(objectStream.str());

	//We add all position bounding box of the cluster

	ObjectModel* om = o->getObjectModel();
	

	std::stringstream ss;
	const auto& blobList = om->getBlobs();
	for(auto it = blobList.begin(); it != blobList.end(); ++it)
	{
		int timestamp = (*it).first;

		const Blob& b = (*it).second;
		const cv::Rect_<float>& r = b.getBoundingBox();
			ss << "insert into bounding_boxes (object_id, frame_number, x_top_left, y_top_left, x_bottom_right, y_bottom_right) values ('"
				<< objectId << "', " << timestamp-1 <<", " << r.x << ", " << r.y << ", "
				<< r.x+r.width << ", " << r.y+r.height << ");";
			
	}

	success &= executeStatement(ss.str());	
	success &= endTransaction();
	if(!success)	
		LOGERROR(o->getObjectId() << " was not saved !");
	
	return success;

}

bool TrackerPersistance::createTables()
{
	bool success = beginTransaction();
	success &= executeStatement("create table objects_type ( road_user_type INTEGER, type_string TEXT, PRIMARY KEY( road_user_type) )");
	success &= executeStatement("create table objects( object_id INTEGER, road_user_type INTEGER, description TEXT, PRIMARY KEY( object_id) )");
	success &= executeStatement("create table bounding_boxes ( object_id INTEGER, frame_number INTEGER, x_top_left REAL, y_top_left REAL, x_bottom_right REAL, y_bottom_right REAL,  PRIMARY KEY( object_id, frame_number ) )");
	success &= endTransaction();
	return success;
}

bool TrackerPersistance::dropTables()
{
	bool success = beginTransaction();
	success &= executeStatement("drop table if exists bounding_boxes;" );
	success &= executeStatement("drop table if exists objects;" );
	success &= executeStatement("drop table if exists objects_type;");
	success &= endTransaction();
	return success;
}


bool TrackerPersistance::saveTypeManager()
{
	bool success = beginTransaction();
	ObjectTypeManager manager;
	auto typeList = manager.getTypeList();
	std::stringstream ss;
	for(auto it = typeList.begin(); it != typeList.end();++it)
		ss << "insert into objects_type (road_user_type, type_string) values ('" << (*it).first << "', '" <<(*it).second << "');";
	success &= executeStatement(ss.str());
	success &= endTransaction();
	return success;
}
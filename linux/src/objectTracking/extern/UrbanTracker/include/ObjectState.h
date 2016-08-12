#ifndef OBJECT_STATE_H
#define OBJECT_STATE_H

#include <string>

enum ObjectState
{
	UNDEFINED = 0x1,
	ENTERING = 0x2,
	HYPOTHESIS = 0x4,
	OBJECT = 0x8,
	LOST = 0x10,
	OBJECTGROUP = 0x20,
	INGROUP = 0x40,
	LEAVING = 0x80,
	DELETED = 0x100,
	SAVEANDELETE = 0x200,
	ESTIMATED = 0x400, //Only valid for bounding box
	STOPPED = 0x800,
	ALL = 0x8FF
};




static std::string getStateRepresentativeString(ObjectState state)
{
	std::string tmp;
	switch(state)
	{
	case UNDEFINED:
		tmp="U";
		break;
	case ENTERING:
		tmp="In";
		break;
	case HYPOTHESIS:
		tmp="H";
		break;
	case OBJECT:
		tmp="O";
		break;
	case LOST:
		tmp="L";
		break;
	case OBJECTGROUP:
		tmp="G";
		break;
	case INGROUP:
		tmp="";
		break;
	case LEAVING:
		tmp="Out";
		break;
	case DELETED:
		tmp="D";
		break;
	case SAVEANDELETE:
		tmp="S";
		break;
	}
	return tmp;
}



#endif
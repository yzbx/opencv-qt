#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include<string>

struct Color
{
	Color(unsigned int red, unsigned int green, unsigned int blue)
	: R(red)
	, G(green)
	, B(blue)
	{}

	Color()
	: R(rand()%255)
	, G(rand()%255)
	, B(rand()%255)
	{

	}

	unsigned int R;
	unsigned int G;
	unsigned int B;
};

class SceneObject
{
public:
	SceneObject(const std::string id = "", const std::string description = "", const std::string type = "");
	~SceneObject();
	const std::string& getDescription() const {return mDescription;}
	void setDescription(const std::string& description) { mDescription = description;}
	const std::string& getId() const {return mId;}
	void setId(const std::string& id) { mId = id;}
	const std::string& getType() const {return mType;}
	void setType(const std::string& type) { mType = type;}
	const Color& getColor() { return mColor;}
	void setColor(Color c) { mColor = c;}

private:
	std::string mId;
	std::string mDescription;
	std::string mType;
	//Not persisted
	Color mColor;

};


#endif
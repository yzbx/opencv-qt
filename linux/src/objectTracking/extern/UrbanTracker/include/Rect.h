#ifndef RECT_H
#define RECT_H

#include <utility>

class Rect 
{
public:
	Rect();
	Rect(unsigned int X, unsigned Y, unsigned Width, unsigned Height);
	~Rect();
	static double overlap(const Rect& A, const Rect& B);
	static unsigned int intersection(const Rect& A, const Rect& B);
	static unsigned int unionRect(const Rect& A, const Rect& B);
	static double dist(const Rect& A, const Rect& B);
	
	void setTopLeftCorner(unsigned int X, unsigned int Y)
	{
		mTopLeftX = X;
		mTopLeftY = Y;
	}

	void setWidth(unsigned int width) { mWidth = width;}
	void setHeight(unsigned int height) {mHeight = height;}
	unsigned int getArea() const;
	std::pair<double, double> getCentroid() const;
	unsigned int getTopLeftX() const {return mTopLeftX;}
	unsigned int getTopLeftY() const {return mTopLeftY;}
	unsigned int getBottomRightX() const {return mWidth+mTopLeftX;}
	unsigned int getBottomRightY() const {return mHeight+mTopLeftY;}
	unsigned int getWidth() const {return mWidth;}
	unsigned int getHeight() const {return mHeight;}

private:
	unsigned int mTopLeftX;
	unsigned int mTopLeftY;
	unsigned int mWidth;
	unsigned int mHeight;
};


#endif
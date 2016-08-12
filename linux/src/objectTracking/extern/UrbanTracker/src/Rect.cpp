#include "Rect.h"
#include <math.h>
Rect::Rect()
: mTopLeftX(0)
, mTopLeftY(0)
, mWidth(1)
, mHeight(1)
{

}
Rect::Rect(unsigned int x, unsigned y, unsigned width, unsigned height)
: mTopLeftX(x)
, mTopLeftY(y)
, mWidth(width)
, mHeight(height)
{

}

Rect::~Rect()
{

}


double Rect::overlap(const Rect& A, const Rect& B)
{
	return ((double)intersection(A,B)) / ((double)unionRect(A,B));
}

unsigned int Rect::intersection(const Rect& A, const Rect& B)
{
	unsigned int x0 = A.getTopLeftX() < B.getTopLeftX() ? B.getTopLeftX() : A.getTopLeftX();
	unsigned int y0 = A.getTopLeftY() < B.getTopLeftY() ? B.getTopLeftY() : A.getTopLeftY();
	unsigned int x1 = A.getBottomRightX() < B.getBottomRightX() ?  A.getBottomRightX(): B.getBottomRightX();
	unsigned int y1 = A.getBottomRightY() < B.getBottomRightY() ?  A.getBottomRightY(): B.getBottomRightY();
	int yIntersection = (y1-y0);
	int xIntersection = (x1-x0);
	return yIntersection >0 && xIntersection > 0 ? xIntersection*yIntersection : 0;
}
unsigned int Rect::unionRect(const Rect& A, const Rect& B)
{
	//L'union est l'aire des deux rectangle - leur intersection
	return A.getArea()+B.getArea()-Rect::intersection(A,B);
}

//Euclidean distance between Rect Centroid 
double Rect::dist(const Rect& A, const Rect& B)
{
	std::pair<double, double> posCentroA = A.getCentroid();
	std::pair<double, double> posCentroB = B.getCentroid();
	return sqrt(pow(posCentroA.first-posCentroB.first,2)+pow(posCentroA.second-posCentroB.second,2));
}

std::pair<double, double> Rect::getCentroid() const
{
	return std::pair<double, double>(mTopLeftX+mWidth/2.0, mTopLeftY+mHeight/2.0);
}

unsigned int Rect::getArea() const
{
	return mWidth*mHeight;
}

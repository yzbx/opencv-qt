#include "OpenCVHelpers.h"
#include "Logger.h"

bool Utils::OpenCV::IsOutsideROI(const cv::Mat& mask, const cv::Point2f& p)
{
	int x = (int) p.x;
	int y = (int) p.y;
	
	bool result =  x >= (mask.cols-2) || y >= (mask.rows-2) || x <= 2 || y <= 2 || ((int)mask.ptr<unsigned char>(y)[x]) == 0; //ShortCircuit evaluation so no out of bound access
	return result;
}

bool Utils::OpenCV::IsOutsideROI(const cv::Mat& mask, const cv::Rect_<float>& r)
{
	cv::Point p1(r.x, r.y);
	cv::Point p2(r.x+r.width, r.y);
	cv::Point p3(r.x, r.y+r.height);
	cv::Point p4(r.x+r.width, r.y+r.height);
	
	/*cv::Mat test = mask.clone();
	cv::rectangle(test, r, cv::Scalar(255));
	cv::imshow("Masque", test);*/
	return IsOutsideROI(mask, p1) || IsOutsideROI(mask, p2) || IsOutsideROI(mask, p3) || IsOutsideROI(mask, p4);
}
		
BlobRect Utils::OpenCV::mergeBlobRect(const BlobRect& A, const BlobRect& B)
{

	BlobRect newBlobRect;
	newBlobRect.mBoundingBox = mergeRect(A.mBoundingBox, B.mBoundingBox);
	newBlobRect.mBlob = cv::Mat::zeros(newBlobRect.mBoundingBox.height, newBlobRect.mBoundingBox.width, A.mBlob.type());
	int deltaX = A.mBoundingBox.x-newBlobRect.mBoundingBox.x;
	int deltaY = A.mBoundingBox.y-newBlobRect.mBoundingBox.y;
	for(unsigned int c = 0; c < A.mBlob.cols; ++c)
	{
		for(unsigned int r = 0; r < A.mBlob.rows; ++r)
		{
			if(A.mBlob.ptr<unsigned char>(r)[c] > 0)
			{
				newBlobRect.mBlob.ptr<unsigned char>(r+deltaY)[c+deltaX] = 1;
			}
		}
	}
	deltaX = B.mBoundingBox.x-newBlobRect.mBoundingBox.x;
	deltaY = B.mBoundingBox.y-newBlobRect.mBoundingBox.y;
	for(unsigned int c = 0; c < B.mBlob.cols; ++c)
	{
		for(unsigned int r = 0; r < B.mBlob.rows; ++r)
		{
			if(B.mBlob.ptr<unsigned char>(r)[c] > 0)
			{
				newBlobRect.mBlob.ptr<unsigned char>((r+deltaY))[(c+deltaX)] = 1; 
			}
		}
	}

	return newBlobRect;
}



cv::Rect_<float> Utils::OpenCV::mergeRect(const cv::Rect_<float>& A, const cv::Rect_<float>& B)
{
	int x1 = cv::min(A.x, B.x);
	int y1 = cv::min(A.y, B.y);
	int x2 = cv::max(A.x+A.width, B.x+B.width);
	int y2 = cv::max(A.y+A.height, B.y+B.height);

	return cv::Rect_<float>(x1, y1, x2-x1, y2-y1);
}

cv::Point2f Utils::OpenCV::getCentroid(const cv::Rect_<float>& A)
{
	return cv::Point2f(A.x+A.width/2.f, A.y+A.height/2.f);
}

bool Utils::OpenCV::IsPartOfForeground(const cv::Mat& mask, const cv::Point2f& p)
{
	if(mask.cols == 0 || mask.rows == 0)
		return true;
	
	bool isForeground = false;
	int x = (int) p.x;
	int y = (int) p.y;
	//Point is outside mask boundaries
	if(!(x >= mask.cols || x <0 || y < 0 || y >= mask.rows))
	{
		isForeground |= !mask.ptr<unsigned char>(y)[x]==0;
	}
	return isForeground;
}


bool Utils::OpenCV::IsPartOfForeground(const cv::Mat& mask, const cv::Point2f& p, unsigned int radius)
{
	if(mask.cols == 0 || mask.rows == 0)
		return true;

	bool isForeground = false;
	//Point is outside mask boundaries
	if(!(p.x > mask.cols || p.x <0 || p.y < 0 || p.y > mask.rows))
	{
		int x1 = (int) p.x - radius;
		x1 = x1 >=0 ? x1:0;
		int y1 = (int) p.y - radius;
		y1 = y1 >=0 ? y1:0;
		int x2 = (int) p.x + radius;
		x2 = x2 < mask.cols ? x2:mask.cols-1;
		int y2 =(int)  p.y + radius;
		y2 = y2 < mask.rows ? y2:mask.rows-1;
		
		for(int i = x1; i <=x2 && !isForeground;  ++i)
		{
			for(int j = y1; j <=y2 && !isForeground;  ++j)
			{
				isForeground |= !mask.ptr<unsigned char>(j)[i]==0;
			}
		}
	}
	return isForeground;
}


unsigned int Utils::OpenCV::GetValue(const cv::Mat& mask, const cv::Point2f& p)
{
	return GetValue(mask, cv::Point((int)p.x, (int)p.y));
}

unsigned int Utils::OpenCV::GetValue(const cv::Mat& mask, const cv::Point& p)
{
	int x = p.x;
	int y = p.y;
	bool insideBoundary = (mask.rows > y && mask.cols > x && x>=0 && y>=0);
	LOGASSERT(insideBoundary, "Matrix value should be in bound");
	return insideBoundary ? mask.ptr<unsigned char>(y)[x] : -1;
}

bool Utils::OpenCV::overlap(const cv::Rect_<float>& A, const cv::Rect_<float>& B)
{
	cv::Rect_<float> intersection = A & B;
	return intersection.width > 0 && intersection.height>0;
}


void Utils::OpenCV::getProjectedPoint(const cv::Mat& homography, const cv::Point2f& in, cv::Point2f& out)
{
	std::vector<cv::Point2f> beforeHomography;
	std::vector<cv::Point2f> afterHomography;
	beforeHomography.push_back(in);
	cv::perspectiveTransform(beforeHomography,afterHomography, homography);
	out = afterHomography[0];
}

cv::Rect_<float> Utils::OpenCV::dilate(const cv::Rect_<float>& r, double dilatation)
{
	int width = r.width;
	int height = r.height;
	double xAdd = (dilatation/2)*(float)width;
	double yAdd = (dilatation/2)*(float)height;
	int posX = floor(r.x-xAdd);
	posX = posX > 0 ? posX : 0;
	int posY = floor(r.y-yAdd);
	posY = posY > 0 ? posY : 0;
	return cv::Rect_<float>(posX, posY, ceil(width+2*xAdd), ceil(height+2*yAdd));
}


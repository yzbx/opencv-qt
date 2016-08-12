#ifndef MATRIX_IO_H
#define MATRIX_IO_H

#include <string>
#include <opencv2/opencv.hpp>
#include "Logger.h"
#include "StringHelpers.h"
namespace Utils
{
	
namespace IO
{

	template <class T>
	bool SaveMatrix(std::string fileName, const cv::Mat& inMatrix)
	{
		bool success = false;
		std::ofstream out(fileName.c_str());
		if(out.is_open())
		{
			int rows = inMatrix.rows;
			int cols = inMatrix.cols;

			for(int r=0; r < rows; ++r)			
			{				
				for(int c = 0 ; c < cols; ++c)
				{
					out << inMatrix.ptr<T>(r)[c] << " ";
				}
				out << "\n";
			}
			out.close();
			success = true;
		}
		else
			LOGERROR("Can't open file " << fileName);
		return success;
	}


	template <class T>
	bool SaveScalar(const std::string& fileName, const T& scalar)
	{
		std::vector<T> scalarList;
		scalarList.push_back(scalar);
		return SaveScalarList(fileName, scalarList);
	}


	template <class T>
	bool SaveScalarList(const std::string& fileName, const std::vector<T>& scalarList)
	{
		bool success = false;
		int cols = scalarList.size();
		std::ofstream out(fileName.c_str());
		if(out.is_open() && !scalarList.empty())
		{			
			for(int c = 0 ; c < cols-1; ++c)
			{
				out << scalarList[c] << " ";
			}
			out << scalarList[cols-1];
			out.close();
			success = true;
		}
		else
			LOGERROR("Can't open file " << fileName);
		return success;
	}
	

	template <class T>
	bool LoadMatrix(const std::string& fileName, cv::Mat& outMatrix)
	{
		bool success = false;
		std::ifstream in(fileName.c_str());
		if(in.is_open())
		{
			//read file
			std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
			in.close();
			StringArray lines = Utils::String::SplitInLines(str);
			if(!lines.empty())
			{
				int rows = lines.size();
				int cols = Utils::String::Split(Utils::String::EnleveEspaceDebutFin(lines.at(0)), " ").size();
				outMatrix = cv::Mat(rows, cols, CV_MAKETYPE(cv::DataDepth<T>::value ,1));
				for(int r = 0 ; r < rows; ++r)
				{
					StringArray elements = Utils::String::Split(Utils::String::EnleveEspaceDebutFin(lines.at(r)), " ");
					assert(elements.size() == rows);
					for(int c=0; c < cols; ++c)
					{
						T element;
						if(Utils::String::StringToType(elements.at(c), element))
							outMatrix.ptr<T>(r)[c] = element;

					}
				}
				success = true;
			}
		}
		else
			LOGERROR("Can't open file " << fileName);

		return success;
	}

	

	template <class T>
	bool LoadScalarList(const std::string& fileName, std::vector<T>& scalarList)
	{
		bool success = false;
		std::ifstream in(fileName.c_str());
		if(in.is_open())
		{
			//read file
			std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
			in.close();
			StringArray nbStr = Utils::String::Split(Utils::String::EnleveEspaceDebutFin(str), " ");
			for(unsigned int c=0; c < nbStr.size(); ++c)
			{
				T element;
				if(Utils::String::StringToType(nbStr.at(c), element))
					scalarList.push_back(element);
			}	
			success = true;
		}
		else
			LOGERROR("Can't open file " << fileName)	;

		return success && !scalarList.empty();
	}

template <class T>
	bool LoadScalar(const std::string& fileName, T& scalar)
	{
		std::vector<T> list;
		bool success = LoadScalarList(fileName, list);
		if(success && !list.empty())
		{
			scalar = list[0];
		}
		return success;
	}
}
}

#endif

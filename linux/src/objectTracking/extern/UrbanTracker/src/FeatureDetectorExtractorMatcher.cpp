#include "FeatureDetectorExtractorMatcher.h"
#include "Logger.h"
#include "Timer.h"


FeatureDetectorExtractorMatcher::FeatureDetectorExtractorMatcher(cv::FeatureDetector* detector, cv::DescriptorExtractor* extractor, cv::DescriptorMatcher* matcher, float ratio)
: mDetector(detector)
, mExtractor(extractor)
, mMatcher(matcher)
, mMatchRatio(ratio)
, mDescriptorSize(extractor->descriptorSize())
, mDescriptorType(mExtractor->descriptorType())
{

}

FeatureDetectorExtractorMatcher::~FeatureDetectorExtractorMatcher()
{
	delete mDetector;
	delete mExtractor;
	delete mMatcher;
	
}

void FeatureDetectorExtractorMatcher::detect(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, cv::Mat& descriptors, const cv::Mat& mask)
{
	cv::Mat imgGray;
	if(image.channels() == 3)
		cv::cvtColor(image, imgGray, CV_BGR2GRAY);
	else if(image.channels() == 1)
		imgGray = image;
	else
		LOGASSERT(false, "Unknown number of channel in image " << image.channels());
	//Timer perfTimer;
	//perfTimer.start();
	mDetector->detect(imgGray, keypoints, mask);
	//perfTimer.printTimeSinceLastPrint("Detection");
	mExtractor->compute(imgGray,keypoints,descriptors);
	//perfTimer.printTimeSinceLastPrint("Extraction");
}

void FeatureDetectorExtractorMatcher::match(const cv::Mat& d1, const cv::Mat& d2, std::vector< cv::DMatch >& matches)
{
	if(!d1.empty() && !d2.empty())
	{
		std::vector<std::vector< cv::DMatch >> multipleMatches1, multipleMatches2;
		/*#pragma omp sections
		{
			#pragma omp section
			{*/ 
				mMatcher->knnMatch(d1, d2, multipleMatches1, 2);
				ratioTest(multipleMatches1, mMatchRatio);
			//}
			/*#pragma omp section
			{ */
				mMatcher->knnMatch(d2, d1, multipleMatches2, 2);
				ratioTest(multipleMatches2, mMatchRatio);
			/*}
		}*/

		//To make the matching for discriminative, we can use the ratio of the distance of the given feature from its nearest match to distance from the second nearest match as a metric.
		
		
		symmetryTest(multipleMatches1,multipleMatches2, matches);
	}
}




void FeatureDetectorExtractorMatcher::match(const cv::Mat& d1, const cv::Mat& d2, std::vector< cv::DMatch >& matches, const cv::Mat& mask)
{	
	if(!d1.empty() && !d2.empty())
	{
		LOGASSERT(d1.rows == mask.rows && d2.rows == mask.cols, "Wrong mask size: " << d1.rows << "==" << mask.rows << " " << d2.rows << "==" << mask.cols);
		std::vector<std::vector< cv::DMatch >> multipleMatches1, multipleMatches2;
		if(mask.empty())
		{
			mMatcher->knnMatch(d1, d2, multipleMatches1, 2);	
			mMatcher->knnMatch(d2, d1, multipleMatches2, 2);
		}
		else
		{
			cv::Mat transposeMask;
			cv::transpose(mask, transposeMask);
			mMatcher->knnMatch(d1, d2, multipleMatches1, 2, mask);	
			mMatcher->knnMatch(d2, d1, multipleMatches2, 2, transposeMask);
		}

		//To make the matching for discriminative, we can use the ratio of the distance of the given feature from its nearest match to distance from the second nearest match as a metric.
		//SPECIFIC_CONFIG

		ratioTest(multipleMatches1, mMatchRatio);
		ratioTest(multipleMatches2, mMatchRatio);
		symmetryTest(multipleMatches1,multipleMatches2, matches);
	}
}

	

int FeatureDetectorExtractorMatcher::ratioTest(std::vector<std::vector<cv::DMatch>>& matches, float ratio) 
{
	int removed=0;

	// for all matches
	for (std::vector<std::vector<cv::DMatch>>::iterator matchIterator= matches.begin();	matchIterator!= matches.end(); ++matchIterator) 
	{
		// if 2 NN has been identified
		if (matchIterator->size() > 1) 
		{
			// check distance ratio
			if ((*matchIterator)[0].distance/(*matchIterator)[1].distance > ratio) 
			{
				matchIterator->clear(); // remove match
				removed++;
			}
		} 
		else 
		{ // does not have 2 neighbours

			matchIterator->clear(); // remove match
			removed++;
		}
	}

	return removed;
}


void FeatureDetectorExtractorMatcher::symmetryTest(const std::vector<std::vector<cv::DMatch>>& matches1,const std::vector<std::vector<cv::DMatch>>& matches2,std::vector<cv::DMatch>& symMatches) 
{
	// for all matches image 1 -> image 2
	for (std::vector<std::vector<cv::DMatch>>::const_iterator matchIterator1= matches1.begin();	matchIterator1!= matches1.end(); ++matchIterator1) 
	{

		if (matchIterator1->size() >= 2) // ignore deleted matches 
		{
			// for all matches image 2 -> image 1
			for (std::vector<std::vector<cv::DMatch>>::const_iterator matchIterator2= matches2.begin();	matchIterator2!= matches2.end(); ++matchIterator2) 
			{
				if (matchIterator2->size() >= 2) // ignore deleted matches 
				{
					// Match symmetry test
					if ((*matchIterator1)[0].queryIdx == (*matchIterator2)[0].trainIdx  && (*matchIterator2)[0].queryIdx == (*matchIterator1)[0].trainIdx) 
					{
						// add symmetrical match
						symMatches.push_back(cv::DMatch((*matchIterator1)[0].queryIdx, (*matchIterator1)[0].trainIdx,(*matchIterator1)[0].distance));
						break; // next match in image 1 -> image 2
					}
				}
			}
		}
	}
}
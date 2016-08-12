#ifndef BLOB_TRACKER_ALGORITHM_PARAMS_H
#define BLOB_TRACKER_ALGORITHM_PARAMS_H


#include <boost/program_options.hpp> 

//TODO: We should have a separate part for the feature detector
struct BlobTrackerAlgorithmParams
{
public:
	BlobTrackerAlgorithmParams(int btMinimumMatchBetweenBlob, int btMinimumBlobSize, int btMaxOcclusionFrame, int briskThreshold, int briskOctave, double matchRatio, double btMaxSegDist, int nbFrameHypothesis, bool btVerifyReEnteringObject, bool btRemoveBirdShadow, bool btRemoveGhosts, int bgsCamNoise, double bgsMovingPixelRatio)
	: minimumMatchBetweenBlob(btMinimumMatchBetweenBlob)
	, minimumBlobsize(btMinimumBlobSize)
	, maximumLostFrame(btMaxOcclusionFrame)
	, detectorThreshold(briskThreshold)
	, detectorOctave(briskOctave)
	, matchRatio(matchRatio)
	, maxSegDist(btMaxSegDist)
	, maxFrameHypothesis(nbFrameHypothesis)
	, verifyReEnteringObject(btVerifyReEnteringObject)
	, removeBirdShadows(btRemoveBirdShadow)
	, removeGhosts(btRemoveGhosts)
	, camNoise(bgsCamNoise)
	, movingPixelRatio(bgsMovingPixelRatio)	
	{
	}
	static boost::program_options::options_description getTrackerOptions()
	{
		boost::program_options::options_description tracker("Tracker options");
		tracker.add_options()
		("minimum-match-between-blob", boost::program_options::value<int>()->default_value(5), "Minimum number of point match between two blobs")		
		
		("max-lost-frame", boost::program_options::value<int>()->default_value(150), "Maximum number of frame an object can be occluded before being deleted")	
		("brisk-threshold", boost::program_options::value<int>()->default_value(10), "Threshold of the BRISK detector")	
		("brisk-octave", boost::program_options::value<int>()->default_value(3), "Number of octave use in the BRISK detector")	
		("match-ratio", boost::program_options::value<double>()->default_value(0.8), "Minimum ratio of the closest match on the second closest match.")	
		("max-seg-dist", boost::program_options::value<double>()->default_value(0.4), "Maximum segmentation distance between 2 merged blobs (in dilatation of bounding box).")	
		("max-hypothesis", boost::program_options::value<int>()->default_value(3), "Maximum number of frame an object can be an hypothesis")
		("urban-isolated-shadow-removal", boost::program_options::value<bool>()->default_value(true), "Remove projected shadow like bird shadows from the result (They will still be tracked, but filtered before saving)")
		("verify-reentering-object", boost::program_options::value<bool>()->default_value(true), "Verify object that object that touch the border an reenter are still the same")		
		("bgs-remove-ghost", boost::program_options::value<bool>()->default_value(true), "Remove automatically object that stop moving from the bgs. Only compatible with ViBe")
		("bgs-cam-noise",  boost::program_options::value<int>()->default_value(2), "If remove ghost option is used, then we use this as the minimum intensity change for a pixel to be considered as moving")
		("bgs-moving-pixel-ratio", boost::program_options::value<double>()->default_value(0.1), "Minimum pixel that have to be changing for a blob to be considered moving. To use with bgs-remove-ghost")
		("bgs-minimum-blob-size", boost::program_options::value<int>()->default_value(280), "Minimum blob size to be considered a real object");
		return tracker;
	}
	int minimumMatchBetweenBlob;		/*Minimum number of match we need to accept that two blob are the same. The principale use of this parameter is to filter lone outliers*/
	int minimumBlobsize;				/*Minimum blob size to be considered a real object*/
	int maximumLostFrame;			/*Maximum number of frame an object can be lost before being deleted*/
	int detectorThreshold;				/*Threshold of the BRISK detector*/
	int detectorOctave;					/*Number of octave use in the BRISK detector*/
	double matchRatio;					/*Minimum ratio of the closest match on the second closest match. */
	double maxSegDist;					/*Maximum segmentation distance between 2 merged blobs (in dilatation of bounding box)*/
	double maxFrameHypothesis;			/*Maximum number of frame an object can be an hypothesis*/
	bool verifyReEnteringObject;		/*Remove automatically object that stop moving from the bgs. Only compatible with ViBe*/
	bool removeBirdShadows;			/*Remove projected shadow like bird shadows from the result (They will still be tracked, but filtered before saving)*/
	bool removeGhosts;				/*Remove automatically object that stop moving from the bgs. Only compatible with ViBe*/
	int camNoise;					/*If remove ghost option is used, then we use this as the minimum intensity change for a pixel to be considered as moving*/
	double movingPixelRatio;			/*Minimum pixel that have to be changing for a blob to be considered moving. To use with bgs-remove-ghost*/



};
#endif
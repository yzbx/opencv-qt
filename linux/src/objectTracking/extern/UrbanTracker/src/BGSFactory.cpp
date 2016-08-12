#include "BGSFactory.h"
#include <package_bgs/IBGS.h>
#include "package_bgs/FrameDifferenceBGS.h"
#include "package_bgs/StaticFrameDifferenceBGS.h"
#include "package_bgs/WeightedMovingMeanBGS.h"
#include "package_bgs/WeightedMovingVarianceBGS.h"
#include "package_bgs/MixtureOfGaussianV1BGS.h"
#include "package_bgs/MixtureOfGaussianV2BGS.h"
#include "package_bgs/AdaptiveBackgroundLearning.h"
#if CV_MAJOR_VERSION >= 2 && CV_MINOR_VERSION >= 4 && CV_SUBMINOR_VERSION >= 3
#include "package_bgs/GMG.h"
#endif

#include "package_bgs/dp/DPAdaptiveMedianBGS.h"
#include "package_bgs/dp/DPGrimsonGMMBGS.h"
#include "package_bgs/dp/DPZivkovicAGMMBGS.h"
#include "package_bgs/dp/DPMeanBGS.h"
#include "package_bgs/dp/DPWrenGABGS.h"
#include "package_bgs/dp/DPPratiMediodBGS.h"
#include "package_bgs/dp/DPEigenbackgroundBGS.h"
#include "package_bgs/dp/DPTextureBGS.h"

#include "package_bgs/tb/T2FGMM_UM.h"
#include "package_bgs/tb/T2FGMM_UV.h"
#include "package_bgs/tb/T2FMRF_UM.h"
#include "package_bgs/tb/T2FMRF_UV.h"
#include "package_bgs/tb/FuzzySugenoIntegral.h"
#include "package_bgs/tb/FuzzyChoquetIntegral.h"

#include "package_bgs/lb/LBSimpleGaussian.h"
#include "package_bgs/lb/LBFuzzyGaussian.h"
#include "package_bgs/lb/LBMixtureOfGaussians.h"
#include "package_bgs/lb/LBAdaptiveSOM.h"
#include "package_bgs/lb/LBFuzzyAdaptiveSOM.h"

#include "package_bgs/ck/LbpMrf.h"

#include "package_bgs/jmo/MultiLayerBGS.h"
// The PBAS algorithm was removed from BGSLibrary because it is
// based on patented algorithm ViBE
// http://www2.ulg.ac.be/telecom/research/vibe/
//#include "package_bgs/pt/PixelBasedAdaptiveSegmenter.h"
#include "package_bgs/av/VuMeter.h"
#include "package_bgs/ae/KDE.h"
#include "package_bgs/db/IndependentMultimodalBGS.h"
#include "package_bgs/sjn/SJN_MultiCueBGS.h"
#include "package_bgs/bl/SigmaDeltaBGS.h"

#include "package_bgs/pl/SuBSENSE.h"
#include "package_bgs/pl/LOBSTER.h"
#ifdef USEVIBE
#include "VibeBGS.h"
#endif


#include "WeightedMovingMeanBGS.h"
#include "WeightedMovingVarianceBGS.h"
//#include "PlaybackBGS.h"
#include <sstream>
BGSFactory::BGSFactory()
{
	nameToInstanceMap["AdaptiveBackgroundLearning"]		= &createInstance<AdaptiveBackgroundLearning>;
	nameToInstanceMap["DPAdaptiveMedianBGS"]			= &createInstance<DPAdaptiveMedianBGS>;
	nameToInstanceMap["DPEigenbackgroundBGS"]			= &createInstance<DPEigenbackgroundBGS>;
	nameToInstanceMap["DPGrimsonGMMBGS"]				= &createInstance<DPGrimsonGMMBGS>;
	nameToInstanceMap["DPMeanBGS"]						= &createInstance<DPMeanBGS>;
	nameToInstanceMap["DPPratiMediodBGS"]				= &createInstance<DPPratiMediodBGS>;
    nameToInstanceMap["DPTextureBGS"]                   = &createInstance<DPTextureBGS>;
	nameToInstanceMap["DPWrenGABGS"]					= &createInstance<DPWrenGABGS>;
	nameToInstanceMap["DPZivkovicAGMMBGS"]				= &createInstance<DPZivkovicAGMMBGS>;
	nameToInstanceMap["FrameDifferenceBGS"]				= &createInstance<FrameDifferenceBGS>;
	nameToInstanceMap["FuzzyChoquetIntegral"]			= &createInstance<FuzzyChoquetIntegral>;
	nameToInstanceMap["FuzzySugenoIntegral"]			= &createInstance<FuzzySugenoIntegral>;
    nameToInstanceMap["IndependentMultimodalBGS"]       = &createInstance<IndependentMultimodalBGS>;
    nameToInstanceMap["KDE"]                            = &createInstance<KDE>;
    nameToInstanceMap["LbpMrf"]                         = &createInstance<LbpMrf>;
	nameToInstanceMap["LBAdaptiveSOM"]					= &createInstance<LBAdaptiveSOM>;
	nameToInstanceMap["LBFuzzyAdaptiveSOM"]				= &createInstance<LBFuzzyAdaptiveSOM>;
	nameToInstanceMap["LBFuzzyGaussian"]				= &createInstance<LBFuzzyGaussian>;
	nameToInstanceMap["LBMixtureOfGaussians"]			= &createInstance<LBMixtureOfGaussians>;
	nameToInstanceMap["LBSimpleGaussian"]				= &createInstance<LBSimpleGaussian>;
    nameToInstanceMap["LOBSTERBGS"]                     = &createInstance<LOBSTERBGS>;
	nameToInstanceMap["MixtureOfGaussianV1BGS"]			= &createInstance<MixtureOfGaussianV1BGS>;
	nameToInstanceMap["MixtureOfGaussianV2BGS"]			= &createInstance<MixtureOfGaussianV2BGS>;
	nameToInstanceMap["MultiLayerBGS"]					= &createInstance<MultiLayerBGS>;
    nameToInstanceMap["SigmaDeltaBGS"]                  = &createInstance<SigmaDeltaBGS>;
    nameToInstanceMap["StaticFrameDifferenceBGS"]		= &createInstance<StaticFrameDifferenceBGS>;
    nameToInstanceMap["SuBSENSEBGS"]                    = &createInstance<SuBSENSEBGS>;
    nameToInstanceMap["SJN_MultiCueBGS"]                = &createInstance<SJN_MultiCueBGS>;
    nameToInstanceMap["T2FGMM_UM"]                      = &createInstance<T2FGMM_UM>;
    nameToInstanceMap["T2FGMM_UV"]                      = &createInstance<T2FGMM_UV>;
    nameToInstanceMap["T2FMRF_UM"]                      = &createInstance<T2FMRF_UM>;
    nameToInstanceMap["T2FMRF_UV"]                      = &createInstance<T2FMRF_UV>;
#ifdef USEVIBE
	nameToInstanceMap["VibeBGS"]						= &createInstance<VibeBGS>;
#endif
    nameToInstanceMap["VuMeter"]						= &createInstance<VuMeter>;
	nameToInstanceMap["WeightedMovingMeanBGS"]			= &createInstance<WeightedMovingMeanBGS>;
	nameToInstanceMap["WeightedMovingVarianceBGS"]		= &createInstance<WeightedMovingVarianceBGS>;
    nameToInstanceMap["default"]                        = &createInstance<FrameDifferenceBGS>;

//	nameToInstanceMap["PlaybackBGS"]		= &createInstance<PlaybackBGS>;
}

BGSFactory::~BGSFactory()
{

}

IBGS* BGSFactory::getBGSInstance(const std::string& bgsMethodName) const
{
	IBGS* bgs = nullptr;

	map_type::const_iterator it = nameToInstanceMap.find(bgsMethodName);
	if(it != nameToInstanceMap.end())
		bgs = it->second();
	else
		std::cout << bgsMethodName << " unavailable. Please choose from the following list:\n" <<  getBGSMethodList();
	return bgs;
}


std::string BGSFactory::getBGSMethodList() const
{
	map_type::const_iterator it;
	std::stringstream ss;
	for(it = nameToInstanceMap.begin(); it != nameToInstanceMap.end(); ++it)
		ss << it->first << "\n";

	return ss.str();
}

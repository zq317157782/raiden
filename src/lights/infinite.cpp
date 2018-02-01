#include "infinite.h"
#include "paramset.h"
std::shared_ptr<InfiniteAreaLight> CreateInfiniteAreaLight(const Transform &light2world,
	const ParamSet &paramSet) {
	Spectrum L=paramSet.FindOneSpectrum("L",1);
	Float scale = paramSet.FindOneFloat("scale", 1.0);
	std::string mapname= paramSet.FindOneString("mapname", nullptr);

	int numSample = paramSet.FindOneInt("nsamples", 1);

	return std::make_shared<InfiniteAreaLight>(light2world,L*scale, numSample,mapname);
}
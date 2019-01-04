/*
 * sppm.cpp
 *
 *  Created on: 2017年2月15日
 *      Author: zhuqian
 */
#include "sppm.h"
#include "paramset.h"
SPPMIntegrator *CreateSPPMIntegrator(
	const ParamSet &params, std::shared_ptr<Sampler> sampler,
	std::shared_ptr<const Camera> camera) {
	int nIterations = params.FindOneInt("numiterations", 64);
	int depth = params.FindOneInt("depth", 5);
	Float radius = params.FindOneFloat("radius", 1.0f);
	uint64_t photonNumPreIteration = params.FindOneInt("photonsperiteration", -1);
	int writeFreq = params.FindOneInt("imagewritefrequency", 1 << 31);
	return new SPPMIntegrator(camera,nIterations,depth, radius, photonNumPreIteration, writeFreq);
}




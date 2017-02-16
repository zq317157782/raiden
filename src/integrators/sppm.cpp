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
	Debug("[CreateSPPMIntegrator]");
	return new SPPMIntegrator(camera,nIterations,depth);
}




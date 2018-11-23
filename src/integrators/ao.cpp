/*
 * ao.cpp
 *
 *  Created on: 2017年5月10日
 *      Author: zhuqian
 */
#include "ao.h"
#include "paramset.h"
AOIntegrator *CreateAOIntegrator(
	const ParamSet &params, std::shared_ptr<Sampler> sampler,
	std::shared_ptr<const Camera> camera) {
	int np;
	const int *pb = params.FindInt("pixelbounds", &np);
	Bound2i pixelBounds = camera->film->GetSampleBounds();
	if (pb) {
		if (np != 4)
			printf("need four\"pixelbounds\"param.actual:%d.",
				np);
		else {
			pixelBounds = Intersect(pixelBounds, Bound2i{ { pb[0], pb[2] },{ pb[1], pb[3] } });
			if (pixelBounds.Area() == 0) {
				Error("\"pixelbounds\"is a tuihua bound.");
			}
		}
	}

	uint32_t sampleNum = params.FindOneInt("samplenum", 32);
	
	return new AOIntegrator(camera, sampler, pixelBounds,sampleNum);
}



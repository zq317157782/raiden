﻿#include "vpt.h"
#include "paramset.h"
#include "film.h"
#include "logging.h"
VolPathIntegrator *CreateVolPathIntegrator(
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
				LError<<"\"pixelbounds\"is a tuihua bound.";
			}
		}
	}
	int depth = params.FindOneInt("depth", 5);
	Float rr = params.FindOneFloat("rrthreshold", 1.0);
	return new VolPathIntegrator(depth, camera, sampler, pixelBounds, rr);
}

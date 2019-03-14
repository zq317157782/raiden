/*
 * depth.cpp
 *
 *  Created on: 2016年12月15日
 *      Author: zhuqian
 */
#include "depth.h"
#include "paramset.h"
#include "camera.h"
#include "film.h"
#include "logging.h"
DepthIntegrator *CreateDepthIntegrator(const ParamSet &params,
		std::shared_ptr<Sampler> sampler,
		std::shared_ptr<const Camera> camera) {
	int np;
	const int *pb = params.FindInt("pixelbounds", &np);
	const Float farPanel = params.FindOneFloat("farpanel",1.0f);
	Bound2i pixelBounds = camera->film->GetSampleBounds();
	if (pb) {
		if (np != 4)
			printf("need four\"pixelbounds\"param.actual:%d.", np);
		else {
			pixelBounds = Intersect(pixelBounds, Bound2i { { pb[0], pb[2] }, {
					pb[1], pb[3] } });
			if (pixelBounds.Area() == 0) {
				LError<<"\"pixelbounds\"is a tuihua bound.";
			}
		}
	}
	return new DepthIntegrator(camera, sampler, pixelBounds,farPanel);
}


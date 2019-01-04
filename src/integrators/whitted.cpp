/*
 * whitted.cpp
 *
 *  Created on: 2017年1月4日
 *      Author: zhuqian
 */
#include "whitted.h"
#include "film.h"
#include "paramset.h"
WhittedIntegrator *CreateWhittedIntegrator(
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
            pixelBounds = Intersect(pixelBounds,Bound2i{{pb[0], pb[2]}, {pb[1], pb[3]}});
            if (pixelBounds.Area() == 0){
                LError<<"\"pixelbounds\"is a tuihua bound.";
            }
        }
    }

	int depth = params.FindOneInt("depth", 5);
    return new WhittedIntegrator(camera,sampler, pixelBounds, depth);
}




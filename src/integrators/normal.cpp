/*
 * normal.cpp
 *
 *  Created on: 2016年12月15日
 *      Author: zhuqian
 */
#include "normal.h"
#include "paramset.h"
#include "camera.h"
#include "film.h"
NormalIntegrator *CreateNormalIntegrator(
    const ParamSet &params, std::shared_ptr<Sampler> sampler,
    std::shared_ptr<const Camera> camera) {
    int maxDepth = params.FindOneInt("maxdepth", 5);
    int np;
    const int *pb = params.FindInt("pixelbounds", &np);
    Bound2i pixelBounds = camera->film->GetSampleBounds();
    if (pb) {
        if (np != 4)
            printf("需要4个\"pixelbounds\"参数.实际:%d.",
                  np);
        else {
            pixelBounds = Intersect(pixelBounds,Bound2i{{pb[0], pb[2]}, {pb[1], pb[3]}});
            if (pixelBounds.Area() == 0){
                Error("\"pixelbounds\"是一个退化了个bound.");
            }
        }
    }
    return new NormalIntegrator(camera,sampler, pixelBounds);
}




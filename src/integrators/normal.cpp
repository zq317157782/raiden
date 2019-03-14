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
#include "logging.h"
NormalIntegrator *CreateNormalIntegrator(
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
    auto typeStr = params.FindOneString("type","normal");
    NormalIntegratorType type;
    if(typeStr=="normal"){
        type=NormalIntegratorType::NORMAL;
    }
    else if(typeStr=="tangent"){
        type=NormalIntegratorType::TANGENT;
    }else {
         type=NormalIntegratorType::BINORMAL;
    }

    return new NormalIntegrator(camera,sampler, pixelBounds,type);
}




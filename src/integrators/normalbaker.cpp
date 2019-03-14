
#include "normalbaker.h"
#include "paramset.h"
#include "camera.h"
#include "film.h"
#include "logging.h"
NormalBakerIntegrator *CreateNormalBakerIntegrator(
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

    auto spaceStr = params.FindOneString("space","object");
	NormalSpace space;
	if(spaceStr=="object"){
		space=NormalSpace::OBJECT;
	}else {
		space=NormalSpace::WORLD;
	}
    return new NormalBakerIntegrator(camera,sampler, pixelBounds,space);
}




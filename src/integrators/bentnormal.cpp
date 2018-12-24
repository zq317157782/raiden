
#include "bentnormal.h"
#include "paramset.h"
BentNormalIntegrator *CreateBentNormalIntegrator(
	const ParamSet &params, std::shared_ptr<Sampler> sampler,
	std::shared_ptr<const Camera> camera) {
	int np;
	const int *pb = params.FindInt("pixelbounds", &np);
	auto modeStr = params.FindOneString("mode","scene");
	BentNormalMode mode;
	if(modeStr=="view"){
		mode=BentNormalMode::VIEW;
	}else {
		mode=BentNormalMode::SCENE;
	}

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

	uint32_t sampleNum = params.FindOneInt("samplenum", 32);
	Float maxDistance = params.FindOneFloat("maxdistance",1);
	return new BentNormalIntegrator(camera, sampler, pixelBounds,sampleNum,mode,maxDistance);
}



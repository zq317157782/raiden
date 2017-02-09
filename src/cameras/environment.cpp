#include "paramset.h"
#include "environment.h"

EnvironmentCamera *CreateEnvironmentCamera(const ParamSet &params,
	const Transform &cam2world,
	Film *film, const Medium *medium) {
	Float shutteropen = params.FindOneFloat("shutteropen", 0.f);
	Float shutterclose = params.FindOneFloat("shutterclose", 1.f);
	if (shutterclose < shutteropen) {
		std::swap(shutterclose, shutteropen);
	}
	Float frame = params.FindOneFloat("frameaspectratio", Float(film->fullResolution.x) / Float(film->fullResolution.y));
	Bound2f screen;
	if (frame > 1.f) {
		screen.minPoint.x = -frame;
		screen.maxPoint.x = frame;
		screen.minPoint.y = -1.0f;
		screen.maxPoint.y = 1.0f;
	}
	else {
		screen.minPoint.x = -1.0f;
		screen.maxPoint.x = 1.0f;
		screen.minPoint.y = -1.0f / frame;
		screen.maxPoint.y = 1.0f / frame;
	}
	int swi;
	const Float *sw = params.FindFloat("screenwindow", &swi);
	if (sw) {
		if (swi == 4) {
			screen.minPoint.x = sw[0];
			screen.maxPoint.x = sw[1];
			screen.minPoint.y = sw[2];
			screen.maxPoint.y = sw[3];
		}
		else
			Error("\"screenwindow\" should have four values");
	}
	return new EnvironmentCamera(cam2world, shutteropen, shutterclose, film, medium);
}
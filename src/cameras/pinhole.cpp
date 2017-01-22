/*
 * pinhole.cpp
 *
 *  Created on: 2016年11月28日
 *      Author: zhuqian
 */

#include "pinhole.h"
#include "paramset.h"
PinholeCamera *CreatePinholeCamera(const ParamSet &params,
		const Transform &cam2world, Film *film, const Medium* medium) {
	Float shutteropen = params.FindOneFloat("shutteropen", 0.0f);
	Float shutterclose = params.FindOneFloat("shutterclose", 1.0f);
	if (shutterclose < shutteropen) {
		Warning("shutterclose["<<shutterclose<<"] <shutteropen["<<shutteropen<<"]. switch.");
		std::swap(shutterclose, shutteropen);
	}
	Float distance = params.FindOneFloat("distance", 10.0f);

	return new PinholeCamera(cam2world, shutteropen, shutterclose,film, medium,
			distance);
}

/*
 * pinhole.cpp
 *
 *  Created on: 2016年11月28日
 *      Author: zhuqian
 */

#include "pinhole.h"
#include "paramset.h"
PinholeCamera *CreatePinholeCamera(const ParamSet &params,
		const Transform &cam2world, Film *film) {
	Float shutteropen = params.FindOneFloat("shutteropen", 0.f);
	Float shutterclose = params.FindOneFloat("shutterclose", 1.f);
	if (shutterclose < shutteropen) {
		printf("曝光结束时间[%f] <曝光开始时间[%f]. 交换它们.", shutterclose, shutteropen);
		std::swap(shutterclose, shutteropen);
	}
	Float distance = params.FindOneFloat("distance", 10.0f);

	return new PinholeCamera(cam2world, shutteropen, shutterclose, film,
			distance);
}

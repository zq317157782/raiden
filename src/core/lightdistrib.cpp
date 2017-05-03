/*
 * lightdistrib.cpp
 *
 *  Created on: 2017年5月3日
 *      Author: zhuqian
 */
#include "lightdistrib.h"

std::unique_ptr<LightDistribution> ComputeLightSampleDistribution(
		const std::string& lightStrategy, const Scene& scene) {
	if (lightStrategy == "uniform") {
		return std::unique_ptr<UniformLightDistribution>(new UniformLightDistribution(scene));
	}
	else {
		Warning("LightStrategy "<<lightStrategy<<" is unknown,use uniform");
		return std::unique_ptr<UniformLightDistribution>(new UniformLightDistribution(scene));
	}
}



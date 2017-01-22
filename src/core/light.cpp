/*
 * light.cpp
 *
 *  Created on: 2016年12月8日
 *      Author: zhuqian
 */

#include "light.h"
#include "scene.h"
bool VisibilityTester::Unoccluded(const Scene& scene) const {
	return !scene.IntersectP(_p0.SpawnRayTo(_p1));
}

Spectrum VisibilityTester::Tr(const Scene &scene, Sampler &sampler) const {
	Ray ray= _p0.SpawnRayTo(_p1);
	Spectrum Tr(1.0);
	while (true) {
		SurfaceInteraction  ref;
		bool isHit=scene.Intersect(ray, &ref);
		
		//说明到光源之间有遮挡物存在,没有能量能返回过来
		if (isHit&&ref.primitive->GetMaterial() != nullptr) {
			return Spectrum(0);
		}

		//累计Tr,如果medium为nullptr,则为真空环境
		if (ray.medium) {
			Tr = Tr*ray.medium->Tr(ray, sampler);
		}
		
		if (!isHit) {
			break;
		}

		ray = ref.SpawnRayTo(_p1);

	}
	return Tr;
}



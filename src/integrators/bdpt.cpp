/*
 * bdpt.cpp
 *
 *  Created on: 2017年5月7日
 *      Author: zhuqian
 */
#include "bdpt.h"
#include "geometry.h"
Float CorrectShadingNormal(const SurfaceInteraction& ref, const Vector3f& wo,
		const Vector3f& wi, TransportMode mode) {
	if (mode == TransportMode::Importance) {
		//向前传播
		return (AbsDot(ref.shading.n, wo) * AbsDot(ref.n, wi))
				/ (AbsDot(ref.n, wo) * AbsDot(ref.shading.n, wi));
	} else {
		//向后传播
		return 1.0;
	}
}


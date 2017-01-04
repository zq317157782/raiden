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



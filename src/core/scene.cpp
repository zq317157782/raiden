/*
 * scene.cpp
 *
 *  Created on: 2016年12月7日
 *      Author: zhuqian
 */
#include "scene.h"
#include "stats.h"

STAT_COUNTER("Intersections/Regular ray intersection tests",nIntersectTest);
bool Scene::Intersect(const Ray& ray,SurfaceInteraction* interaction) const{
	++nIntersectTest;
	Assert(ray.d!=Vector3f(0,0,0));//保证射线的方向没有退化
	return _aggregate->Intersect(ray,interaction);
}

STAT_COUNTER("Intersections/Shadow ray intersection tests",nShadowTest);
bool Scene::IntersectP(const Ray& ray) const{
	++nShadowTest;
	Assert(ray.d!=Vector3f(0,0,0));//保证射线的方向没有退化
	return _aggregate->IntersectP(ray);
}




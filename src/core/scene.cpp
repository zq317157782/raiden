/*
 * scene.cpp
 *
 *  Created on: 2016年12月7日
 *      Author: zhuqian
 */
#include "scene.h"
#include "stats.h"

bool Scene::Intersect(const Ray& ray,SurfaceInteraction* interaction) const{
	Assert(ray.d!=Vector3f(0,0,0));//保证射线的方向没有退化
	return _aggregate->Intersect(ray,interaction);
}

bool Scene::IntersectP(const Ray& ray) const{
	Assert(ray.d!=Vector3f(0,0,0));//保证射线的方向没有退化
	return _aggregate->IntersectP(ray);
}


//相比Intersect,IntersectTr还计算了beam Tr
bool Scene::IntersectTr(Ray ray, Sampler &sampler, SurfaceInteraction *isect,
	Spectrum *Tr) const {
	(*Tr) = Spectrum(1.0);
	
	while (true) {
		//判断射线有没有与啥东西相交，可能是普通物体，可能是MEDIUM边界
		bool isFound = Intersect(ray, isect);
		//如果射线所在的介质非真空
		if (ray.medium) {
			(*Tr) = (*Tr)*ray.medium->Tr(ray, sampler);
		}
		if (!isFound) {
			return false;
		}
		
		//如果相交的图元拥有材质，说明射线已经和具有表面反射属性的图元相交了，返回相交
		if (isect->primitive->GetMaterial() != nullptr) {
			return true;
		}

		//继续延伸射线
		ray = isect->SpawnRay(ray.d);
	}
}



/*
 * camera.cpp
 *
 *  Created on: 2016年11月28日
 *      Author: zhuqian
 */

#include "camera.h"
Camera::Camera(const Transform& c2w, Float shutterOpen, Float shutterEnd,
		Film * f) :
		film(f), cameraToWorld(c2w), shutterOpen(shutterOpen), shutterEnd(
				shutterEnd) {

}

float Camera::GenerateRayDifferential(const CameraSample &sample,
		RayDifferential *rd) const {
	float wt = GenerateRay(sample, rd);
	//生成x偏移射线
	CameraSample sshift = sample;
	++sshift.pFilm.x;
	Ray rx;
	float wtx = GenerateRay(sshift, &rx);
	if(wtx==0) return 0;
	rd->ox = rx.o;
	rd->dx = rx.d;
	//生成y偏移射线
	--sshift.pFilm.x;
	++sshift.pFilm.y;
	Ray ry;
	float wty = GenerateRay(sshift, &ry);
	if(wty==0) return 0;
	rd->oy = ry.o;
	rd->dy = ry.d;
	//设置射线为微分射线
	rd->hasDifferential=true;
	return wt;
}

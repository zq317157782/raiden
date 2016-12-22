/*
 * camera.cpp
 *
 *  Created on: 2016年11月28日
 *      Author: zhuqian
 */

#include "camera.h"
#include "film.h"
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
	if (wtx == 0)
		return 0;
	rd->ox = rx.o;
	rd->dx = rx.d;
	//生成y偏移射线
	--sshift.pFilm.x;
	++sshift.pFilm.y;
	Ray ry;
	float wty = GenerateRay(sshift, &ry);
	if (wty == 0)
		return 0;
	rd->oy = ry.o;
	rd->dy = ry.d;
	//设置射线为微分射线
	rd->hasDifferential = true;
	return wt;
}

ProjectiveCamera::ProjectiveCamera(const Transform& c2w, const Transform& proj,
		const Bound2f& screenWindow,Float shutterOpen, Float shutterEnd,
		Float lensr, Float focald, Film * f) :
		Camera(c2w, shutterOpen, shutterEnd, f) {
	_cameraToScreen = proj; //投影矩阵
	_lensRadius = lensr;
	_focalDistance = focald;
	//从底往上看1.把screen的原点挪到00位置,然后你懂得
	_screenToRaster = Scale(Float(film->fullResolution.x),
			Float(film->fullResolution.y), 1.f)
			* Scale(
					1.0f
							/ (screenWindow.maxPoint.x
									- screenWindow.minPoint.x),
					1.0f
							/ (screenWindow.minPoint.y
									- screenWindow.maxPoint.y), 1.0f)
			* Translate(
					Vector3f(-screenWindow.minPoint.x,
							-screenWindow.maxPoint.y, 0.0f));
	_rasterToScreen = Inverse(_screenToRaster);
	_rasterToCamera = Inverse(_cameraToScreen) * _rasterToScreen;

}

/*
 * perspective.cpp
 *
 *  Created on: 2016年12月23日
 *      Author: zhuqian
 */
#include "raiden.h"
#include "perspective.h"
#include "sampling.h"
#include "paramset.h"
#include "film.h"
PerspectiveCamera::PerspectiveCamera(const Transform& c2w,
		const Bound2f& screenWindow, Float shutterOpen, Float shutterEnd,
		Float lensr, Float focald, Float fov,Film * f, const Medium* medium) :
		ProjectiveCamera(c2w, Perspective(fov, 1e-3f, 1000.0f)/*c2s*/,
				screenWindow, shutterOpen, shutterEnd, lensr, focald,f,medium) {
	//获得光栅化空间下，相机空间对应的差分
	_dxCamera = _rasterToCamera(Vector3f(1, 0, 0))
			- _rasterToCamera(Vector3f(0, 0, 0));
	_dyCamera = _rasterToCamera(Vector3f(0, 1, 0))
			- _rasterToCamera(Vector3f(0, 0, 0));
}

Float PerspectiveCamera::GenerateRay(const CameraSample &sample,
		Ray *ray) const {
	Point3f pRas = Point3f(sample.pFilm.x, sample.pFilm.y, 0.0f);
	Point3f pCam = _rasterToCamera(pRas); //计算相机空间下的image panel样本值
	*ray = RayDifferential(Point3f(0,0,0), Vector3f(pCam));
	if (_lensRadius > 0.0f) {
		Point2f lens = ConcentricSampleDisk(sample.pLen) * _lensRadius; //采样Lens
		//计算焦距平面交点
		//其实这的z就是1
		Float ft = _focalDistance / ray->d.z;
		Point3f pFocus = (*ray)(ft);
		//原点设置成来自镜面
		ray->o = Point3f(lens.x, lens.y, 0);
		ray->d = Normalize(pFocus - ray->o);
	}
	ray->time = Lerp(sample.time, shutterOpen, shutterEnd);
	Info(medium);
	ray->medium = medium;
	*ray = cameraToWorld(*ray);
	return 1.0f;
}
Float PerspectiveCamera::GenerateRayDifferential(const CameraSample &sample,
		RayDifferential *ray) const {
	Point3f pRas = Point3f(sample.pFilm.x, sample.pFilm.y, 0.0f);
	Point3f pCam = _rasterToCamera(pRas); //计算相机空间下的image panel样本值
	*ray = RayDifferential(Point3f(0,0,0), Vector3f(pCam));
	if (_lensRadius > 0.0f) {
		Point2f lens = ConcentricSampleDisk(sample.pLen) * _lensRadius; //采样Lens
		//计算焦距平面交点
		//其实这的z就是1
		Float ft = _focalDistance / ray->d.z;
		Point3f pFocus = (*ray)(ft);
		//原点设置成来自镜面
		ray->o = Point3f(lens.x, lens.y, 0);
		ray->d = Normalize(pFocus - ray->o);
	}
	//处理微分信息
	if (_lensRadius > 0.0f) {
		Point2f lens = ConcentricSampleDisk(sample.pLen) * _lensRadius; //采样Lens
		//计算x射线
		Vector3f dx = Normalize(Vector3f(pCam) + _dxCamera);
		Float ft = _focalDistance / dx.z;
		Point3f pFocusX = Point3f(0, 0, 0) + (ft * dx);
		ray->ox = Point3f(lens.x, lens.y, 0);
		ray->dx = Normalize(pFocusX - ray->ox);
		//计算y射线
		Vector3f dy = Normalize(Vector3f(pCam) + _dyCamera);
		ft = _focalDistance / dy.z;
		Point3f pFocusY = Point3f(0, 0, 0) + (ft * dy);
		ray->oy = Point3f(lens.x, lens.y, 0);
		ray->dy = Normalize(pFocusY - ray->oy);
	} else {
		//镜片为奇点的情况
		ray->ox=ray->oy=ray->o;
		ray->dx =Vector3f(pCam)+_dxCamera;
		ray->dy =Vector3f(pCam)+_dyCamera;
	}
	ray->hasDifferential = true;
	ray->time = Lerp(sample.time, shutterOpen, shutterEnd);
	ray->medium = medium;
	*ray = cameraToWorld(*ray);
	return 1.0f;
}


PerspectiveCamera *CreatePerspectiveCamera(const ParamSet &params,
                                           const Transform &cam2world,
                                           Film *film, const Medium *medium) {
    // Extract common camera parameters from _ParamSet_
    Float shutteropen = params.FindOneFloat("shutteropen", 0.f);
    Float shutterclose = params.FindOneFloat("shutterclose", 1.f);
    if (shutterclose < shutteropen) {
        std::swap(shutterclose, shutteropen);
    }
    Float lensradius = params.FindOneFloat("lensradius", 0.f);
    Float focaldistance = params.FindOneFloat("focaldistance", 1e6);
    Float frame = params.FindOneFloat("frameaspectratio",Float(film->fullResolution.x) / Float(film->fullResolution.y));
    Bound2f screen;
    if (frame > 1.f) {
    		screen.minPoint.x = -frame;
    		screen.maxPoint.x = frame;
    		screen.minPoint.y = -1.0f;
    		screen.maxPoint.y = 1.0f;
    	} else {
    		screen.minPoint.x = -1.0f;
    		screen.maxPoint.x = 1.0f;
    		screen.minPoint.y = -1.0f / frame;
    		screen.maxPoint.y = 1.0f / frame;
    	}
    int swi;
    const Float *sw = params.FindFloat("screenwindow", &swi);
    if (sw) {
    		if (swi == 4) {
    			screen.minPoint.x = sw[0];
    			screen.maxPoint.x = sw[1];
    			screen.minPoint.y = sw[2];
    			screen.maxPoint.y = sw[3];
    		} else
    			Error("\"screenwindow\" should have four values");
    	}
    Float fov = params.FindOneFloat("fov", 90.0);
    Float halffov = params.FindOneFloat("halffov", -1.0f);
    if (halffov > 0.0f){
        fov = 2.0f * halffov;
    }
    return new PerspectiveCamera(cam2world, screen, shutteropen, shutterclose,
                                 lensradius, focaldistance, fov,film, medium);
}

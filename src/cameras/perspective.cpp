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
	_dxCamera = _rasterToCamera(Point3f(1, 0, 0))
			- _rasterToCamera(Point3f(0, 0, 0));
	_dyCamera = _rasterToCamera(Point3f(0, 1, 0))
			- _rasterToCamera(Point3f(0, 0, 0));

	//计算相机空间下，Z=1的情况下的film的面积
	const Point2i res = film->fullResolution;
	Point3f minP=_rasterToCamera(Point3f(0,0,0));
	Point3f maxP=_rasterToCamera(Point3f(res.x,res.y,0));
	minP=minP/minP.z;
	maxP=maxP/maxP.z;
	_A=std::abs((maxP.x-minP.x)*(maxP.y-minP.y));
}

Float PerspectiveCamera::GenerateRay(const CameraSample &sample,
		Ray *ray) const {
	Point3f pRas = Point3f(sample.pFilm.x, sample.pFilm.y, 0.0f);
	Point3f pCam = _rasterToCamera(pRas); //计算相机空间下的image panel样本值
	*ray = Ray(Point3f(0,0,0), Normalize(Vector3f(pCam)));
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
	
	*ray = RayDifferential(Point3f(0,0,0), Normalize(Vector3f(pCam)));
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
		ray->dx = Normalize(Vector3f(pCam)+_dxCamera);
		ray->dy = Normalize(Vector3f(pCam)+_dyCamera);
	}
	ray->hasDifferential = true;
	ray->time = Lerp(sample.time, shutterOpen, shutterEnd);
	ray->medium = medium;
	*ray = cameraToWorld(*ray);
	return 1.0;
}

//计算射线ray所携带的importance值
Spectrum PerspectiveCamera::We(const Ray& ray,Point2f* rasterPos) const{
	//TODO 使用AnimaTransfoem
	//首先判断射线时候和相机的朝向在同一个半球
	Vector3f camDirWorld=cameraToWorld(Vector3f(0,0,1));
	Float cosTheta=Dot(ray.d,camDirWorld);
	if(cosTheta<=0){
		return 0;
	}
	//再判断射线是否在film所在的bound中
	//首先计算射线和焦平面的交点(所有从Film某个点出发的射线必定在焦平面相交于1点)
	Float t=0;
	if(_lensRadius>0){
		t=_focalDistance/cosTheta;
	}
	else{
		//pinhole
		t=1.0/cosTheta;
	}
	Point3f posW=ray(t);//世界坐标系
	Point3f posC=Inverse(cameraToWorld)(posW);//相机坐标系
	Point3f posR=Inverse(_rasterToCamera)(posC);//光栅化坐标系
	if(rasterPos!=nullptr){
		*rasterPos=Point2f(posR.x,posR.y);
	}
	Bound2i bound=film->GetSampleBounds();
	if(posR.x<bound.minPoint.x||posR.y<bound.minPoint.y||posR.x>=bound.maxPoint.x||posR.y>=bound.maxPoint.y){
		return 0;
	}
	//计算importance
	//areaPdf=1.0/_A  面积空间下的pdf
	//d^2=1/cosTheta2 两种解释，第一种是直接根据COS的定义，第二种是cosTheta是向量的z分量
	Float cosTheta2=cosTheta*cosTheta;
	//lens的面积 pinhole的话面积为1，因为是Dirac Distribution
	Float lensArea=_lensRadius>0?(Pi*_lensRadius*_lensRadius):1;
	return 1.0/(_A*cosTheta2*cosTheta2*lensArea);//4个cos的几何意义不同 一个和转换到solid空间有关，一个为了cancel out ray-space中的cos,还有两个是距离平方和的一部分
}

void PerspectiveCamera::Pdf_We(const Ray& ray,Float* posPdf,Float* dirPdf) const{
	//几乎和We函数相同
	Vector3f camDirWorld=cameraToWorld(Vector3f(0,0,1));
	Float cosTheta=Dot(ray.d,camDirWorld);
	if(cosTheta<=0){
		*posPdf=0;
		*dirPdf=0;
		return;
	}

	Float t=0;
	if(_lensRadius>0){
		t=_focalDistance/cosTheta;
	}
	else{
		//pinhole
		t=1.0/cosTheta;
	}
	Point3f posW=ray(t);//世界坐标系
	Point3f posC=Inverse(cameraToWorld)(posW);//相机坐标系
	Point3f posR=Inverse(_rasterToCamera)(posC);//光栅化坐标系
	
	Bound2i bound=film->GetSampleBounds();
	if(posR.x<bound.minPoint.x||posR.y<bound.minPoint.y||posR.x>=bound.maxPoint.x||posR.y>=bound.maxPoint.y){
		*posPdf=0;
		*dirPdf=0;
		return;
	}
	Float lensArea=_lensRadius>0?(Pi*_lensRadius*_lensRadius):1;
	*posPdf = 1.0/lensArea;
	*dirPdf = 1.0/((_A*cosTheta)*(cosTheta*cosTheta));
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

/*
 * camera.h
 *
 *  Created on: 2016年11月28日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_CAMERA_H_
#define SRC_CORE_CAMERA_H_
#include "raiden.h"
#include "geometry.h"
#include "transform.h"
#include "medium.h"
//相机样本
struct CameraSample{
	Point2f pFilm;//image panel上的样本
	Point2f pLen; //镜面上的样本
	Float time;   //时间样本
};

class Camera {
public:
	Film * film;
	Transform cameraToWorld;
	const Float shutterOpen,shutterEnd;//曝光时间区间
	const Medium * medium;//中间介质
public:
	Camera(const Transform& c2w,Float shutterOpen,Float shutterEnd,Film * f, const Medium* medium);
	virtual Float GenerateRay(const CameraSample &sample, Ray *ray) const = 0;
	virtual Float GenerateRayDifferential(const CameraSample &sample,
			RayDifferential *rd) const;
	//返回ray方向上的importance
	virtual Spectrum We(const Ray& ray,Point2f* rasterPos=nullptr) const;
	//返回ray方向上采样到importance的pdf
	virtual void Pdf_We(const Ray& ray,Float* posPdf,Float* dirPdf) const;
	virtual Spectrum Sample_Wi(const Interaction& ref,const Point2f&sample,Vector3f* wi,Float* pdf,Point2f* rasterPos,VisibilityTester* tester) const;
	virtual ~Camera() {
	}
};

//投影相机
class ProjectiveCamera: public Camera {
protected:
	/*4个变换矩阵*/
	Transform _cameraToScreen, _rasterToCamera;
	Transform _screenToRaster, _rasterToScreen;
	Float _lensRadius, _focalDistance;//镜片半径,焦距
public:
	ProjectiveCamera(const Transform& c2w, const Transform& proj,const Bound2f& screenWindow,Float shutterOpen,Float shutterEnd,
			 Float lensr, Float focald, Film * f, const Medium* medium);
};


#endif /* SRC_CORE_CAMERA_H_ */

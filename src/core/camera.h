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
public:
	Camera(const Transform& c2w,Float shutterOpen,Float shutterEnd,Film * f);
	virtual Float GenerateRay(const CameraSample &sample, Ray *ray) const = 0;
	virtual Float GenerateRayDifferential(const CameraSample &sample,
			RayDifferential *rd) const;

	virtual ~Camera() {
	}
};



#endif /* SRC_CORE_CAMERA_H_ */

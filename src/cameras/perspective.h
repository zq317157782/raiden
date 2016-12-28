/*
 * prespective.h
 *
 *  Created on: 2016年12月23日
 *      Author: zhuqian
 */

#ifndef SRC_CAMERAS_PERSPECTIVE_H_
#define SRC_CAMERAS_PERSPECTIVE_H_

#include "raiden.h"
#include "camera.h"
class PerspectiveCamera:public ProjectiveCamera{
private:
	//相机空间和光栅化空间的差分
	Vector3f _dxCamera;
	Vector3f _dyCamera;
public:
	PerspectiveCamera(const Transform& c2w, const Bound2f& screenWindow,Float shutterOpen,Float shutterEnd,Float lensr,
			Float focald,Float fov,Film * f);

	virtual Float GenerateRay(const CameraSample &sample, Ray *ray) const override;
	virtual Float GenerateRayDifferential(const CameraSample &sample,
				RayDifferential *rd) const override;
};
PerspectiveCamera *CreatePerspectiveCamera(const ParamSet &params,
                                           const Transform &cam2world,
                                           Film *film);

#endif /* SRC_CAMERAS_PERSPECTIVE_H_ */
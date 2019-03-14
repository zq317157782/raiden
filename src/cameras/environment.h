/*
* environment.h
*
*  Created on: 2016年2月9日
*      Author: zhuqian
*/

#ifndef SRC_CAMERAS_ENVERONMENT_H_
#define SRC_CAMERAS_ENVERONMENT_H_

#include "raiden.h"
#include "camera.h"
#include "film.h"
#include "mmath.h"

class EnvironmentCamera :public Camera {
private:
	//相机空间和光栅化空间的差分
	Vector3f _dxCamera;
	Vector3f _dyCamera;
public:
	EnvironmentCamera(const Transform& c2w, Float shutterOpen, Float shutterEnd, Film * f, const Medium* medium):Camera(c2w, shutterOpen, shutterEnd,f, medium){}
	virtual Float GenerateRay(const CameraSample &sample, Ray *ray) const override {
		Float theta = Pi*sample.pFilm.y / (film->fullResolution.y);
		Float phi =  2*Pi*sample.pFilm.x / (film->fullResolution.x);
		Vector3f dir(std::sin(theta) * std::cos(phi), std::cos(theta), std::sin(theta) * std::sin(phi));
		*ray = Ray(Point3f(0, 0, 0),dir,Infinity, Lerp(sample.time, shutterOpen, shutterEnd));
		ray->medium = medium;
		*ray = cameraToWorld(*ray);
		return 1;
	}
};
EnvironmentCamera *CreateEnvironmentCamera(const ParamSet &params,
	const Transform &cam2world,
	Film *film, const Medium *medium);
#endif /* SRC_CAMERAS_ENVERONMENT_H_ */

/*
 * pinhole.h
 *
 *  Created on: 2016年11月28日
 *      Author: zhuqian
 */

#ifndef SRC_CAMERA_PINHOLE_H_
#define SRC_CAMERA_PINHOLE_H_
#include "raiden.h"
#include "camera.h"

class PinholeCamera: public Camera {
public:
	const Float distanceToView;
public:
	PinholeCamera(const Transform& c2w, Float shutterOpen, Float shutterEnd,
			Film * f, const float distance = 10) :
			Camera(c2w, shutterOpen, shutterEnd, f), distanceToView(distance) {
	}

	virtual Float GenerateRay(const CameraSample &sample, Ray *ray) const
			override {
		Point3f origin(0, 0, 0);
		Vector3f dir = Normalize(
				Vector3f(sample.pFilm.x, sample.pFilm.y, distanceToView));
		*ray = Ray(origin, dir);
		ray->time = Lerp(sample.time, shutterOpen, shutterEnd);
		*ray=cameraToWorld(*ray);
	}
};

#endif /* SRC_CAMERA_PINHOLE_H_ */

/*
 * pinhole.h
 *
 *  Created on: 2016年11月28日
 *      Author: zhuqian
 */

#ifndef SRC_CAMERAS_PINHOLE_H_
#define SRC_CAMERAS_PINHOLE_H_
#include "raiden.h"
#include "camera.h"
#include "film.h"
class PinholeCamera: public Camera {
public:
	const Float distanceToView;
public:
	PinholeCamera(const Transform& c2w, Float shutterOpen, Float shutterEnd,
			Film * f, const Medium* medium, const Float distance = 10) :
			Camera(c2w, shutterOpen, shutterEnd, f, medium), distanceToView(distance) {
	}

	virtual Float GenerateRay(const CameraSample &sample, Ray *ray) const
			override {
		Vector2i res=film->croppedPixelBound.maxPoint-film->croppedPixelBound.minPoint;
		Point3f origin(0, 0, 0);
		Vector3f dir = Normalize(
				Vector3f(sample.pFilm.x-res.x/2, sample.pFilm.y-res.y/2, distanceToView));
		*ray = Ray(origin, dir);
		ray->time = Lerp(sample.time, shutterOpen, shutterEnd);
		ray->medium = medium;
		*ray=cameraToWorld(*ray);
		return 1.0f;
	}
};
// {shutteropen:Float,shutterclose:Float,distance:Float}
PinholeCamera *CreatePinholeCamera(const ParamSet &params,
		const Transform &cam2world, Film *film, const Medium* medium);

#endif /* SRC_CAMERAS_PINHOLE_H_ */

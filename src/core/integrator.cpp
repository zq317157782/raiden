/*
 * integrator.cpp
 *
 *  Created on: 2016年12月7日
 *      Author: zhuqian
 */
#include "integrator.h"
#include "camera.h"
#include "geometry.h"
#include "film.h"
#include "scene.h"
#include "sampler.h"
void SamplerIntegrator::RenderScene(const Scene& scene) {
	for (Point2i pos : _pixelBound) {
		if (!InsideExclusive(pos, _camera->film->croppedPixelBound)) {
			continue;
		}
		_sampler->StartPixel(pos);
		do {
			CameraSample cs = _sampler->GetCameraSample(pos);
			Ray r;
			_camera->GenerateRay(cs, &r);
			if (scene.IntersectP(r)) {
				Float rgb[3] = { 1, 1, 1 };
				Float xyz[3];
				RGBToXYZ(rgb, xyz);
				_camera->film->GetPixel(pos).xyz[0] += xyz[0];
				_camera->film->GetPixel(pos).xyz[1] += xyz[1];
				_camera->film->GetPixel(pos).xyz[2] += xyz[2];

			} else {
				Float rgb[3] = { 0, 0, 0 };
				Float xyz[3];
				RGBToXYZ(rgb, xyz);
				_camera->film->GetPixel(pos).xyz[0] += xyz[0];
				_camera->film->GetPixel(pos).xyz[1] += xyz[1];
				_camera->film->GetPixel(pos).xyz[2] += xyz[2];
			}
			_camera->film->GetPixel(pos).filterWeightSum += 1;
		} while (_sampler->StartNextSample());
	}
	_camera->film->WriteImage();
}


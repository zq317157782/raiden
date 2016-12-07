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
	std::unique_ptr<FilmTile> tile=_camera->film->GetFilmTile(_pixelBound);
	for (Point2i pos : tile->GetPixelBound()) {
		_sampler->StartPixel(pos);
		do {
			CameraSample cs = _sampler->GetCameraSample(pos);
			Ray r;
			_camera->GenerateRay(cs, &r);
			if (scene.IntersectP(r)) {
				tile->AddSample(pos,Spectrum(1),1);
			} else {
				tile->AddSample(pos,Spectrum(0),1);
			}
		} while (_sampler->StartNextSample());
	}
	_camera->film->MergeFilmTile(std::move(tile));
	_camera->film->WriteImage();
}


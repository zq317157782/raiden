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
#include "interaction.h"
#include "light.h"
void SamplerIntegrator::RenderScene(const Scene& scene) {
	std::unique_ptr<FilmTile> tile=_camera->film->GetFilmTile(_pixelBound);
	for (Point2i pos : tile->GetPixelBound()) {
		_sampler->StartPixel(pos);
		do {
			CameraSample cs = _sampler->GetCameraSample(pos);
			Ray r;
			_camera->GenerateRay(cs, &r);
			SurfaceInteraction ref;
			if (scene.Intersect(r,&ref)) {
				for(int i=0;i<scene.lights.size();++i){
					std::shared_ptr<Light> light=scene.lights[i];
					Vector3f wi;
					Float pdf;
					Spectrum I=light->Sample_Li(ref,&wi,&pdf);
					Float cosln=Clamp(Dot(wi,ref.n),0,1);
					tile->AddSample(pos,I*cosln/Pi,1);
				}
			} else {
				tile->AddSample(pos,Spectrum(0),1);
			}
		} while (_sampler->StartNextSample());
	}
	_camera->film->MergeFilmTile(std::move(tile));
	_camera->film->WriteImage();
}


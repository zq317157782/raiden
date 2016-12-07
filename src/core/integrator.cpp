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
void SamplerIntegrator::RenderScene(const Scene& scene){
	Point2i startRes=_camera->film->croppedPixelBound.minPoint;
	Point2i endRes=_camera->film->croppedPixelBound.maxPoint;
	for (int j =startRes.y ; j < endRes.y; ++j)
			for (int i = startRes.x; i < endRes.x; ++i) {
				_sampler->StartPixel(Point2i(i, j));
				do {
					CameraSample cs = _sampler->GetCameraSample(Point2i(i, j));
					Ray r;
					_camera->GenerateRay(cs, &r);
					if (scene.IntersectP(r)) {
						Float rgb[3]={1,1,1};
						Float xyz[3];
						RGBToXYZ(rgb,xyz);
						_camera->film->GetPixel(Point2i(i, j)).xyz[0]+=xyz[0];
						_camera->film->GetPixel(Point2i(i, j)).xyz[1]+=xyz[1];
						_camera->film->GetPixel(Point2i(i, j)).xyz[2]+=xyz[2];
						_camera->film->GetPixel(Point2i(i, j)).filterWeightSum+=1;
					}
				}while (_sampler->StartNextSample()) ;
			}
		_camera->film->WriteImage();
}




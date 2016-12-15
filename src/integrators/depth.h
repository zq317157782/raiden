/*
 * depth.h
 *
 *  Created on: 2016年12月15日
 *      Author: zhuqian
 */

#ifndef SRC_INTEGRATORS_DEPTH_H_
#define SRC_INTEGRATORS_DEPTH_H_
#include "raiden.h"
#include "integrator.h"
//深度积分器，用来渲染深度贴图(depth texture)
class DepthIntegrator:public SamplerIntegrator{
private:
	Float _far,_invFar;
public:
	DepthIntegrator(const std::shared_ptr<const Camera>& camera,const std::shared_ptr<Sampler>& sampler,const Bound2i&pixelBound,Float farPlanel):
	SamplerIntegrator(camera,sampler,pixelBound),_far(farPlanel),_invFar(1.0f/farPlanel){}

	virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
		                        Sampler &sampler, MemoryArena &arena,
		                        int depth = 0) const override{
		SurfaceInteraction ref;//和表面的交互点
		if (scene.Intersect(ray, &ref)) {
			Float distance=(ref.p-ray.o).Magnitude()*_invFar;
			return Spectrum(std::min(distance,1.0f));
		}
		return 1.0f;
	}
};


#endif /* SRC_INTEGRATORS_DEPTH_H_ */

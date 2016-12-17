/*
 * normal.h
 *
 *  Created on: 2016年12月15日
 *      Author: zhuqian
 */

#ifndef SRC_INTEGRATORS_NORMAL_H_
#define SRC_INTEGRATORS_NORMAL_H_
#include "raiden.h"
#include "integrator.h"
#include "spectrum.h"
#include "interaction.h"
#include "scene.h"
//深度积分器，用来渲染深度贴图(depth texture)
class NormalIntegrator:public SamplerIntegrator{
private:

public:
	NormalIntegrator(const std::shared_ptr<const Camera>& camera,const std::shared_ptr<Sampler>& sampler,const Bound2i&pixelBound):
	SamplerIntegrator(camera,sampler,pixelBound){}

	virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
		                        Sampler &sampler, MemoryArena &arena,
		                        int depth = 0) const override{
		SurfaceInteraction ref;//和表面的交互点
		if (scene.Intersect(ray, &ref)) {
			Spectrum ret(0.0f);
			Normal3f nn=ref.n;
			nn=(nn+Normal3f(1.0f,1.0f,1.0f))*0.5f;
			ret[0]=nn.x;
			ret[1]=nn.y;
			ret[2]=nn.z;
			return ret;
		}
		return 0.0f;
	}
};


NormalIntegrator *CreateNormalIntegrator(
    const ParamSet &params, std::shared_ptr<Sampler> sampler,
    std::shared_ptr<const Camera> camera);


#endif /* SRC_INTEGRATORS_NORMAL_H_ */

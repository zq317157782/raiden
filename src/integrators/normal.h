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

enum class NormalMode{
	VIEW,SCENE
};

class NormalIntegrator:public SamplerIntegrator{
private:
	NormalMode _mode;
public:
	NormalIntegrator(const std::shared_ptr<const Camera>& camera,const std::shared_ptr<Sampler>& sampler,const Bound2i&pixelBound,NormalMode mode=NormalMode::SCENE):
	SamplerIntegrator(camera,sampler,pixelBound),_mode(mode){}

	virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
		                        Sampler &sampler, MemoryArena &arena,
		                        int depth = 0) const override{
		SurfaceInteraction ref;//和表面的交互点
		Spectrum ret(0.0f);
		if(_mode==NormalMode::VIEW){
			Normal3f nn=(Normal3f)ray.d;
			//映射到[0~1]范围
			nn=(nn+Normal3f(1.0f,1.0f,1.0f))*0.5f;
			ret[0]=nn.x;
			ret[1]=nn.y;
			ret[2]=nn.z;
			return ret;
		}else{
			if (scene.Intersect(ray, &ref)) {
				Normal3f nn=ref.shading.n;
				//映射到[0~1]范围
				nn=(nn+Normal3f(1.0f,1.0f,1.0f))*0.5f;
				ret[0]=nn.x;
				ret[1]=nn.y;
				ret[2]=nn.z;
				return ret;
			}
		}
		return 0.0f;
		
	}
};


NormalIntegrator *CreateNormalIntegrator(
    const ParamSet &params, std::shared_ptr<Sampler> sampler,
    std::shared_ptr<const Camera> camera);


#endif /* SRC_INTEGRATORS_NORMAL_H_ */

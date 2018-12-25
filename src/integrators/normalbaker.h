/*
 * normal.h
 *
 *  Created on: 2018年12月25日
 *      Author: zhuqian
 */

#ifndef SRC_INTEGRATORS_NORMALBAKER_H_
#define SRC_INTEGRATORS_NORMALBAKER_H_
#include "raiden.h"
#include "integrator.h"
#include "spectrum.h"
#include "interaction.h"
#include "scene.h"
#include "camera.h"
enum class NormalSpace{
    WORLD,OBJECT
};

class NormalBakerIntegrator:public SamplerIntegrator{
private:
    NormalSpace _space;
	inline Normal3f MapTo01(const Normal3f& n) const{
		return (n+Normal3f(1.0f,1.0f,1.0f))*0.5f;
	}


public:
	NormalBakerIntegrator(const std::shared_ptr<const Camera>& camera,const std::shared_ptr<Sampler>& sampler,const Bound2i&pixelBound,NormalSpace space=NormalSpace::OBJECT):
	SamplerIntegrator(camera,sampler,pixelBound),_space(space){}

	virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
		                        Sampler &sampler, MemoryArena &arena,
		                        int depth = 0) const override{
		SurfaceInteraction ref;//和表面的交互点
		RGBSpectrum ret(0.0f);
		Normal3f nn=Normalize((Normal3f)ray.d);
        if(_space==NormalSpace::OBJECT){
            nn=_camera->worldToCamera(nn);
        }
		//映射到[0~1]范围
		nn=MapTo01(nn);
		ret[0]=nn.x;
		ret[1]=nn.y;
		ret[2]=nn.z;
		return ret;
	}
};


NormalBakerIntegrator *CreateNormalBakerIntegrator(
    const ParamSet &params, std::shared_ptr<Sampler> sampler,
    std::shared_ptr<const Camera> camera);


#endif /* SRC_INTEGRATORS_NORMAL_H_ */

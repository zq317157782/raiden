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

enum class NormalIntegratorType{
	NORMAL,TANGENT,BINORMAL
};

class NormalIntegrator:public SamplerIntegrator{
private:
	inline Normal3f MapTo01(const Normal3f& n) const{
		return (n+Normal3f(1.0f,1.0f,1.0f))*0.5f;
	}

	inline Vector3f MapTo01(const Vector3f& n) const{
		return (n+Vector3f(1.0f,1.0f,1.0f))*0.5f;
	}
	
	NormalIntegratorType _type;
public:
	NormalIntegrator(const std::shared_ptr<const Camera>& camera,const std::shared_ptr<Sampler>& sampler,const Bound2i&pixelBound,const NormalIntegratorType& type=NormalIntegratorType::NORMAL):
	SamplerIntegrator(camera,sampler,pixelBound),_type(type){}

	virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
		                        Sampler &sampler, MemoryArena &arena,
		                        int depth = 0) const override{
		SurfaceInteraction ref;//和表面的交互点
		RGBSpectrum ret(0.0f);
		
		if (scene.Intersect(ray, &ref)) {
			if(_type==NormalIntegratorType::NORMAL){
				Normal3f nn=Normalize(ref.shading.n);
				//映射到[0~1]范围
				nn=MapTo01(nn);
				ret[0]=nn.x;
				ret[1]=nn.y;
				ret[2]=nn.z;
			} else if(_type==NormalIntegratorType::TANGENT){
				auto nn=Normalize(ref.shading.dpdu);
				//映射到[0~1]范围
				nn=MapTo01(nn);
				ret[0]=nn.x;
				ret[1]=nn.y;
				ret[2]=nn.z;
			} else {
				auto nn=Normalize(ref.shading.dpdv);
				//映射到[0~1]范围
				nn=MapTo01(nn);
				ret[0]=nn.x;
				ret[1]=nn.y;
				ret[2]=nn.z;
			}
			
			return ret;
		}
		return 0.0f;
	}
};


NormalIntegrator *CreateNormalIntegrator(
    const ParamSet &params, std::shared_ptr<Sampler> sampler,
    std::shared_ptr<const Camera> camera);


#endif /* SRC_INTEGRATORS_NORMAL_H_ */

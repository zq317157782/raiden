/*
 * integrator.h
 *
 *  Created on: 2016年12月7日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_INTEGRATOR_H_
#define SRC_CORE_INTEGRATOR_H_
#include "raiden.h"
#include "geometry.h"
class Integrator{
public:
	virtual void RenderScene(const Scene&)=0;
	virtual ~Integrator(){}
};

class SamplerIntegrator:public Integrator{
private:
	std::shared_ptr<Sampler> _sampler;//采样器
	Bound2i _pixelBound;
protected:
	std::shared_ptr<const Camera> _camera;
public:
	SamplerIntegrator(const std::shared_ptr<const Camera>& camera,const std::shared_ptr<Sampler>& sampler,const Bound2i&pixelBound):
		_sampler(sampler),_camera(camera),_pixelBound(pixelBound){

	}
	virtual void RenderScene(const Scene&) override;
	//返回ray射线上的radiance
	virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
	                        Sampler &sampler, MemoryArena &arena,
	                        int depth = 0) const = 0;
};


//使用双重重要性采样来计算直接光贡献
Spectrum EstimateDirect(const Interaction &it, const Point2f &uScattering,
	const Light &light, const Point2f &uLight,
	const Scene &scene, Sampler &sampler,
	MemoryArena &arena, bool handleMedia, bool specular);
#endif /* SRC_CORE_INTEGRATOR_H_ */

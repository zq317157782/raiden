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
	virtual void Render(const Scene&)=0;
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

	//样本积分器的预处理阶段
	virtual void Preprocess(const Scene& scene,Sampler& sampler){

	}

	virtual void Render(const Scene&) override;
	//返回ray射线上的radiance
	virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
	                        Sampler &sampler, MemoryArena &arena,
	                        int depth = 0) const = 0;
	//处理镜面反射的情况
	Spectrum SpecularReflect(const RayDifferential& ray,const SurfaceInteraction& isect,const Scene&scene,Sampler& sampler,MemoryArena& arena,int depth) const;
	//处理镜面折射的情况
	Spectrum SpecularTransmit(const RayDifferential& ray, const SurfaceInteraction& isect, const Scene&scene, Sampler& sampler, MemoryArena& arena, int depth) const;
};


//使用双重重要性采样来计算直接光贡献
Spectrum EstimateDirect(const Interaction &it, const Point2f &uScattering,
	const Light &light, const Point2f &uLight,
	const Scene &scene, Sampler &sampler,
	MemoryArena &arena, bool handleMedia=false, bool specular=false);

//均匀采样多个光源的radiance
Spectrum UniformSampleAllLights(const Interaction&it, const Scene& scene, MemoryArena &arena, Sampler &sampler,
	const std::vector<int> &nLightSamples,
	bool handleMedia=false);

//采样单个光源,这里是保证采样单个光源，并且除以它出现的PDF在样本足够多的情况下，近似采样多个光源的情况的统计学近似
Spectrum UniformSampleOneLight(const Interaction&it, const Scene& scene, MemoryArena &arena, Sampler &sampler,
	bool handleMedia = false,const Distribution1D* lightPowerDistribution=nullptr);

//根据场景中所用的光源，计算相应的光源能量的1D离散分布，用于采样光源.
std::unique_ptr<Distribution1D> ComputeLightPowerDistribution(Scene& scene);

#endif /* SRC_CORE_INTEGRATOR_H_ */

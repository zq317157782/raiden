/*
 * whitted.h
 *
 *  Created on: 2017年1月4日
 *      Author: zhuqian
 */

#ifndef SRC_INTEGRATORS_WHITTED_H_
#define SRC_INTEGRATORS_WHITTED_H_

#include "raiden.h"
#include "integrator.h"
#include "spectrum.h"
#include "interaction.h"
#include "scene.h"
#include "sampler.h"
#include "reflection.h"
class WhittedIntegrator: public SamplerIntegrator {
private:
	int _maxDepth; //反射和折射的最大次数
public:
	WhittedIntegrator(const std::shared_ptr<const Camera>& camera,
			const std::shared_ptr<Sampler>& sampler, const Bound2i&pixelBound,
			int maxdepth = 5) :
			SamplerIntegrator(camera, sampler, pixelBound), _maxDepth(maxdepth) {
	}

	virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
			Sampler &sampler, MemoryArena &arena, int depth = 0) const
					override {
		Spectrum L(0);
		SurfaceInteraction ref; //和表面的交互点
		//没有物体相交的情况，要考虑没有几何体的光源
		if (!scene.Intersect(ray, &ref)) {
			for (const auto& light : scene.lights) {
				L+=light->Le(ray);//考虑inf类型光源
			}
			return L;
		}
		Vector3f wo = ref.wo;
		Normal3f n = ref.shading.n;
		ref.ComputeScatteringFunctions(ray, arena);
		//这里考虑的是如果相交的是MediumInterface
		if (!ref.bsdf) {
			return Li(ref.SpawnRay(ray.d), scene, sampler, arena, depth);
		}
		//考虑相交的是area light
		L+=ref.Le(wo);

		//遍历所有的光源，采样光源的能量
		for (const auto& light : scene.lights) {
			Vector3f wi;
			Float pdf;
			VisibilityTester tester;
			//采样一个光源
			Spectrum Li = light->Sample_Li(ref, sampler.Get2DSample(), &wi,
					&pdf, &tester);
			if (Li.IsBlack() || pdf == 0) {
				continue;
			}
			Spectrum f = ref.bsdf->f(wo, wi);
			//先考虑f降低开销
			if(!f.IsBlack()&&tester.Unoccluded(scene)){
				
				L+=f*AbsDot(n,wi)*Li/pdf;
			}
		}
		//处理满没镜面反射和完美镜面折射
		if(depth+1<_maxDepth){
			L+=SpecularReflect(ray,ref,scene,sampler,arena,depth);
			L+= SpecularTransmit(ray, ref, scene, sampler, arena, depth);
		}
		return L;
	}
};

WhittedIntegrator *CreateWhittedIntegrator(const ParamSet &params,
		std::shared_ptr<Sampler> sampler, std::shared_ptr<const Camera> camera);

#endif /* SRC_INTEGRATORS_WHITTED_H_ */

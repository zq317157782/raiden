/*
* pt.h
*
*  Created on: 2017年1月16日
*      Author: zhuqian
*/

#ifndef SRC_INTEGRATORS_PT_H_
#define SRC_INTEGRATORS_PT_H_

#include "raiden.h"
#include "integrator.h"
#include "spectrum.h"
#include "interaction.h"
#include "scene.h"
#include "sampler.h"
#include "reflection.h"
class PathIntegrator : public SamplerIntegrator {
private:
	int _maxDepth; //路径的最大长度
	Float _rrThreshold;//开始考虑俄罗斯罗盘的阈值
public:
	PathIntegrator(int maxdepth,const std::shared_ptr<const Camera>& camera,
		const std::shared_ptr<Sampler>& sampler, const Bound2i&pixelBound, Float rrThreshold=1) :
		SamplerIntegrator(camera, sampler, pixelBound), _maxDepth(maxdepth), _rrThreshold(rrThreshold){
	}

	virtual Spectrum Li(const RayDifferential &r, const Scene &scene,
		Sampler &sampler, MemoryArena &arena, int depth = 0) const
		override {
		Spectrum L(0);
		RayDifferential ray(r);
		SurfaceInteraction ref; //和表面的交互点
		Spectrum beta = 1;//path throughoutput
		bool isSpecularBounce=false;//最后一次反射是否是镜面反射
		bool isHit=false;
		int bounces;//反射的次数
		for (bounces = 0;; ++bounces) {
			isHit = scene.Intersect(ray, &ref);

			//如果是第一次反射，或者是镜面反射，需要计算自发光成分
			if (bounces == 0 || isSpecularBounce) {
				if (isHit) {
					L+= beta*ref.Le(-ray.d);//考虑自发光
				}
				else {
					for (const auto& light : scene.lights) {
						L += beta*light->Le(ray);//考虑inf类型光源
					}
				}
			}

			//终止条件
			if (!isHit || bounces >= _maxDepth) {
				break;
			}

			ref.ComputeScatteringFunctions(ray, arena, true);
			//这里考虑的其实是media边界的情况
			//path 不考虑media,所以继续向下追踪
			if (!ref.bsdf) {
				ray = ref.SpawnRay(ray.d);
				--bounces;
				continue;
			}

			//判断bsdf中是否包含非specular成分，有的话，就要计算交点处的贡献
			if (ref.bsdf->NumComponents(BxDFType(BSDF_ALL&~BSDF_SPECULAR))) {
				Spectrum Ld= beta*UniformSampleOneLight(ref, scene, arena, sampler, false);
				Assert(Ld.y() >=0);
				L += Ld;
			}

			//开始采样新的方向
			Vector3f wo= -ray.d;
			Vector3f wi;
			Float pdf;
			BxDFType flag;
			Spectrum f= ref.bsdf->Sample_f(wo, &wi, sampler.Get2DSample(), &pdf, BSDF_ALL, &flag);
			
			//这个bsdf采样到的方向的贡献为0，直接跳出，因为接下来所有的贡献都没有意义了
			if (f.IsBlack() || pdf == 0) {
				break;
			}
			//判断这次反射是否是specular
			if ((flag&BSDF_SPECULAR) != 0) {
				isSpecularBounce = true;
			}
			else {
				isSpecularBounce = false;
			}

			beta = beta*(f*AbsDot(wi, ref.shading.n)/pdf);
			Assert(beta.y() >= 0);
			//生成新射线
			ray = ref.SpawnRay(wi);
			if (beta.MaxComponentValue()<_rrThreshold&&bounces>3) {
				Float q = std::max(0.05, 1.0 - beta.MaxComponentValue());
				if (sampler.Get1DSample() < q) {
					break;
				}
				//添加俄罗斯罗盘的weight
				beta=beta/(1 - q);
			}
		}

					
		return L;
	}
};

PathIntegrator *CreatePathIntegrator(const ParamSet &params,
	std::shared_ptr<Sampler> sampler, std::shared_ptr<const Camera> camera);

#endif /* SRC_INTEGRATORS_WHITTED_H_ */

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
#include "lightdistrib.h"
class PathIntegrator : public SamplerIntegrator {
private:
	int _maxDepth; //路径的最大长度
	Float _rrThreshold;//开始考虑俄罗斯罗盘的阈值
	std::string _lightStrategy;
	std::unique_ptr<LightDistribution> _lightDistribution;//光源的分布
public:
	PathIntegrator(int maxdepth,const std::shared_ptr<const Camera>& camera,
		const std::shared_ptr<Sampler>& sampler, const Bound2i&pixelBound, Float rrThreshold=1,const std::string& lightStrategy="uniform") :
		SamplerIntegrator(camera, sampler, pixelBound), _maxDepth(maxdepth), _rrThreshold(rrThreshold),_lightStrategy(lightStrategy){
	}
	//PathIntegrator这里主要是为了处理相应的光源分布选择
	//为啥是这两个参数是因为PBRT就用到这两个参数,哈哈哈哈
	virtual void Preprocess(const Scene& scene,Sampler& sampler) override{
		_lightDistribution=ComputeLightSampleDistribution(_lightStrategy,scene);
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
		
		
		//这里是PBRT第三版发布后加入的代码之一
		//PBRT的解释也放在这了
		// Added after book publication: etaScale tracks the accumulated effect
		// of radiance scaling due to rays passing through refractive
		// boundaries (see the derivation on p. 527 of the third edition). We
		// track this value in order to remove it from beta when we apply
		// Russian roulette; this is worthwhile, since it lets us sometimes
		// avoid terminating refracted rays that are about to be refracted back
		// out of a medium and thus have their beta value increased.
		Float etaScale = 1;
		
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

			ref.ComputeScatteringFunctions(ray, arena, false);
			//这里考虑的其实是media边界的情况
			//path 不考虑media,所以继续向下追踪
			if (!ref.bsdf) {
				ray = ref.SpawnRay(ray.d);
				--bounces;
				continue;
			}

			//通过相交点，计算相应的光源分布
			//这样子可以保证每个相交点使用的都是最好的光源分布策略
			const Distribution1D* distribution=_lightDistribution->Lookup(ref.p);

			//判断bsdf中是否包含非specular成分，有的话，就要计算交点处的贡献
			if (ref.bsdf->NumComponents(BxDFType(BSDF_ALL&~BSDF_SPECULAR))) {
				Spectrum Ld= beta*UniformSampleOneLight(ref, scene, arena, sampler, false,distribution);
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
		
			if((flag&BSDF_SPECULAR)&&((flag&BSDF_TRANSMISSION))){
				//拿到ior的比例
				Float eta= ref.bsdf->eta;
				etaScale=etaScale*(Dot(wo,ref.n)>0)?(eta*eta):(1/(eta*eta));
			}
		
			//生成新射线
			ray = ref.SpawnRay(wi);
			 // Factor out radiance scaling due to refraction in rrBeta.
			Spectrum rrBeta=beta*etaScale;
			if (rrBeta.MaxComponentValue()<_rrThreshold&&bounces>3) {
				Float q = std::max(0.05, 1.0 - rrBeta.MaxComponentValue());
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

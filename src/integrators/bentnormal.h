/*
 * bentnormal.h
 *
 *  Created on: 2017年5月10日
 *      Author: zhuqian
 */
#pragma once
#ifndef SRC_INTEGRATORS_BENTNORMAL_H_
#define SRC_INTEGRATORS_BENTNORMAL_H_
#include "raiden.h"
#include "integrator.h"
#include "sampler.h"
#include "film.h"
#include "scene.h"
#include "sampling.h"
#include "mmath.h"
enum class BentNormalMode{
	VIEW,SCENE
};

class BentNormalIntegrator : public SamplerIntegrator
{
  private:
	uint32_t _sampleNum;
	BentNormalMode _mode;
	Float _minDistance;
	Float _maxDistance;
  public:
	BentNormalIntegrator(const std::shared_ptr<const Camera>& camera,const std::shared_ptr<Sampler>& sampler,const Bound2i&pixelBound,uint32_t sampleNum,BentNormalMode mode=BentNormalMode::SCENE,Float minDistance=0,Float maxDistance=1) :SamplerIntegrator(camera,sampler,pixelBound), _sampleNum(sampleNum)
	{
		//申请相应的样本空间
		sampler->Request2DArray(sampleNum);
		_mode=mode;
		_minDistance=minDistance;
		_maxDistance=maxDistance;
	}

	virtual Spectrum Li(const RayDifferential &r, const Scene &scene,
						Sampler &sampler, MemoryArena &arena, int depth = 0) const
		override
	{
        RGBSpectrum L(0);
		Vector3f avgWi(0,0,0);
		RayDifferential ray(r);
		SurfaceInteraction ref; //和表面的交互点

		if(_mode==BentNormalMode::VIEW){
			//计算从切线坐标系到世界坐标系的FRAME
			//这里使用的是几何法线
			 auto n=(Normal3f)ray.d;
			 Vector3f t;
			 Vector3f b;
			 CoordinateSystem((Vector3f)n,&t,&b);

			 auto samples = sampler.Get2DArray(_sampleNum);
			 for(uint32_t i=0;i<_sampleNum;++i)
			 {
				auto sample=samples[i];
				//采样样本
				//切线空间
				auto wi = UniformSampleHemisphere(sample);
				auto pdf = UniformHemispherePdf();
				//转换到世界坐标系
				auto cosTheta=std::abs(wi.z);
				wi=Vector3f(
					t.x*wi.x+b.x*wi.y+n.x*wi.z,
					t.y*wi.x+b.y*wi.y+n.y*wi.z,
					t.z*wi.x+b.z*wi.y+n.z*wi.z
				);

				ray.d=Normalize(wi);
				ray.o=ray.o+ray.d*_minDistance;
				ray.tMax=_maxDistance-_minDistance;


				if (!scene.IntersectP(ray))
				{	
					avgWi += (wi / (pdf*_sampleNum*Pi));
				}
			 }
		}else {
			//这个goto是为了如果交点是PM的话，需要继续延申射线
	retry:
		bool isHit = scene.Intersect(ray, &ref);
		if (isHit)
		{
			//计算相应的BSDF
			ref.ComputeScatteringFunctions(ray, arena, false);
			//射到PM了
			if (!ref.bsdf)
			{
				ray = ref.SpawnRay(ray.d);
				goto retry;
			}

			//计算从切线坐标系到世界坐标系的FRAME
			//这里使用的是几何法线
			auto n=Faceforward(ref.n,-ray.d);
			auto t=Normalize(ref.dpdu);
			auto b=Cross(n,t);

			//到这里的话，已经和surface相交了
			auto samples = sampler.Get2DArray(_sampleNum);
			for(uint32_t i=0;i<_sampleNum;++i)
			{
				auto sample=samples[i];
				//采样样本
				//切线空间
				auto wi = UniformSampleHemisphere(sample);
				auto pdf = UniformHemispherePdf();
				//转换到世界坐标系
				auto cosTheta=std::abs(wi.z);
				wi=Vector3f(
					t.x*wi.x+b.x*wi.y+n.x*wi.z,
					t.y*wi.x+b.y*wi.y+n.y*wi.z,
					t.z*wi.x+b.z*wi.y+n.z*wi.z
				);

				auto rr = ref.SpawnRay(Normalize(wi));
				rr.o=ray.o+ray.d*_minDistance;
				rr.tMax=_maxDistance-_minDistance;
				if (!scene.IntersectP(rr))
				{
					avgWi += (wi / (pdf*_sampleNum*Pi));
				}
			}
		}
		}
        avgWi=Normalize(avgWi);
        avgWi=(avgWi+Vector3f(1.0f,1.0f,1.0f))*0.5f;
        L[0]=avgWi[0];
        L[1]=avgWi[1];
        L[2]=avgWi[2];
		return L;
	};
};

 BentNormalIntegrator *CreateBentNormalIntegrator(const ParamSet &params,
 	std::shared_ptr<Sampler> sampler, std::shared_ptr<const Camera> camera);
#endif 

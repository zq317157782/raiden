/*
* pt.h
*
*  Created on: 2017��1��16��
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
	int _maxDepth; //·������󳤶�
	Float _rrThreshold;//��ʼ���Ƕ���˹���̵���ֵ
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
		SurfaceInteraction ref; //�ͱ���Ľ�����
		Spectrum beta = 1;//path throughoutput
		bool isSpecularBounce=false;//���һ�η����Ƿ��Ǿ��淴��
		bool isHit=false;
		int bounces;//����Ĵ���
		for (bounces = 0;; ++bounces) {
			isHit = scene.Intersect(ray, &ref);

			//����ǵ�һ�η��䣬�����Ǿ��淴�䣬��Ҫ�����Է���ɷ�
			if (bounces == 0 || isSpecularBounce) {
				if (isHit) {
					L+= beta*ref.Le(-ray.d);//�����Է���
				}
				else {
					for (const auto& light : scene.lights) {
						L += beta*light->Le(ray);//����inf���͹�Դ
					}
				}
			}

			//��ֹ����
			if (!isHit || bounces >= _maxDepth) {
				break;
			}

			ref.ComputeScatteringFunctions(ray, arena, true);
			//���￼�ǵ���ʵ��media�߽�����
			//path ������media,���Լ�������׷��
			if (!ref.bsdf) {
				ray = ref.SpawnRay(ray.d);
				--bounces;
				continue;
			}

			//�ж�bsdf���Ƿ������specular�ɷ֣��еĻ�����Ҫ���㽻�㴦�Ĺ���
			if (ref.bsdf->NumComponents(BxDFType(BSDF_ALL&~BSDF_SPECULAR))) {
				Spectrum Ld= beta*UniformSampleOneLight(ref, scene, arena, sampler, false);
				Assert(Ld.y() >=0);
				L += Ld;
			}

			//��ʼ�����µķ���
			Vector3f wo= -ray.d;
			Vector3f wi;
			Float pdf;
			BxDFType flag;
			Spectrum f= ref.bsdf->Sample_f(wo, &wi, sampler.Get2DSample(), &pdf, BSDF_ALL, &flag);
			
			//���bsdf�������ķ���Ĺ���Ϊ0��ֱ����������Ϊ���������еĹ��׶�û��������
			if (f.IsBlack() || pdf == 0) {
				break;
			}
			//�ж���η����Ƿ���specular
			if ((flag&BSDF_SPECULAR) != 0) {
				isSpecularBounce = true;
			}
			else {
				isSpecularBounce = false;
			}

			beta = beta*(f*AbsDot(wi, ref.shading.n)/pdf);
			Assert(beta.y() >= 0);
			//����������
			ray = ref.SpawnRay(wi);
			if (beta.MaxComponentValue()<_rrThreshold&&bounces>3) {
				Float q = std::max(0.05, 1.0 - beta.MaxComponentValue());
				if (sampler.Get1DSample() < q) {
					break;
				}
				//��Ӷ���˹���̵�weight
				beta=beta/(1 - q);
			}
		}

					
		return L;
	}
};

PathIntegrator *CreatePathIntegrator(const ParamSet &params,
	std::shared_ptr<Sampler> sampler, std::shared_ptr<const Camera> camera);

#endif /* SRC_INTEGRATORS_WHITTED_H_ */

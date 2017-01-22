/*
* homogeneous.h
*
*  Created on: 2017年1月22日
*      Author: zhuqian
*/

#ifndef SRC_MEDIA_HOMOGENEOUS_H_
#define SRC_MEDIA_HOMOGENEOUS_H_
#include "raiden.h"
#include "medium.h"
#include "interaction.h"
#include "sampler.h"
#include "memory.h"
//同质介质
class HomogeneousMedium :public Medium {
private:
	const Spectrum _sigma_a;//吸收率
	const Spectrum _sigma_s;//散射率
	const Spectrum _sigma_t;
	const Float _g;
public: 
	HomogeneousMedium(const Spectrum& s_a, const Spectrum& s_s, Float g):_sigma_a(s_a), _sigma_s(s_s), _sigma_t(s_a+ s_s), _g(g){

	}
	virtual ~HomogeneousMedium(){}

	Spectrum Tr(const Ray &ray, Sampler &sampler) const override {
		return Exp(-_sigma_t*std::min(ray.tMax*ray.d.Length(), MaxFloat));
	}

	Spectrum Sample(const Ray &ray, Sampler &sampler,
		MemoryArena &arena,
		MediumInteraction *mi) const override {

		//選擇某個分量來計算pdf
		int c = std::min(sampler.Get1DSample()*Spectrum::numSample, sampler.Get1DSample()*Spectrum::numSample - 1);
		//根據選擇的分量，隨機采樣參數t 
		Float dist= -std::log(1 - sampler.Get1DSample()) / _sigma_t[c];
		Float t = std::min(dist*ray.d.Length(), ray.tMax);

		//判斷采樣的是介質還是表面
		bool isMedium = t < ray.tMax;
		//是介質的話，初始化MediumInteraction
		if (isMedium) {
			*mi = MediumInteraction(ray(t),Normalize(-ray.d),ray.time,this,ARENA_ALLOC(arena,HenyeyGreenstein)(_g));
		}

		//計算散射係數
		Spectrum Tr= Exp(-_sigma_t*std::min(t*ray.d.Length(), MaxFloat));
		//根據是否是介質，返回不同的密度函數
		Spectrum density = isMedium ? _sigma_t*Tr : Tr;
		Float pdf = 0.0;
		//以各個分量的平均值作爲pdf
		for (int i = 0; i < Spectrum::numSample; ++i) {
			pdf += density[i];
		}
		pdf /= Spectrum::numSample;
		if (pdf == 0) {
			pdf = 1;
		}
		
		return isMedium? (Tr * _sigma_s / pdf) : (Tr / pdf);
	}
};

#endif /* SRC_MEDIA_HOMOGENEOUS_H_ */

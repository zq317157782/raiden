/*
 * lambertian.h
 *
 *  Created on: 2016年12月30日
 *      Author: zhuqian
 */

#ifndef SRC_MATERIALS_LAMBERTIAN_H_
#define SRC_MATERIALS_LAMBERTIAN_H_
#include "raiden.h"
#include "material.h"
#include "memory.h"
#include "interaction.h"
class Lambertian: public Material {
private:
	std::shared_ptr<Texture<Spectrum>> _kd;//完美漫反射的反射率
public:

	Lambertian(const std::shared_ptr<Texture<Spectrum>>& kd):_kd(kd){
	}
	virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const override{
		//1.先为SurfaceInteraction分配bsdf
		si->bsdf=ARENA_ALLOC(arena,BSDF)(*si);
		//2.获取反射率，并且为BSDF添加BxDF
		Spectrum r=_kd->Evaluate(*si);
		si->bsdf->Add(ARENA_ALLOC(arena,LambertianReflection(r)));
	}
	virtual ~Lambertian(){};
};

#endif /* SRC_MATERIALS_LAMBERTIAN_H_ */

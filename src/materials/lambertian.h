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
#include "reflection.h"
#include "texture.h"
class LambertianMaterial: public Material {
private:
	std::shared_ptr<Texture<Spectrum>> _kd;//完美漫反射的反射率
public:

	LambertianMaterial(const std::shared_ptr<Texture<Spectrum>>& kd):_kd(kd){
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
	virtual ~LambertianMaterial(){};
};

LambertianMaterial* CreateLambertianMaterial(const TextureParams&mp);

#endif /* SRC_MATERIALS_LAMBERTIAN_H_ */

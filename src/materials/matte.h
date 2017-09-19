/*
 * matter.h
 *
 *  Created on: 2017年9月19日
 *      Author: zhuqian
 */

#pragma once

#include "raiden.h"
#include "material.h"
#include "memory.h"
#include "interaction.h"
#include "reflection.h"
#include "texture.h"
class MatteMaterial:public Material {
private:
    std::shared_ptr<Texture<Spectrum>> _kd;//反射率
    std::shared_ptr<Texture<Float>> _sigma;//OrenNayar分部使用的sigma参数
public:

	MatteMaterial(const std::shared_ptr<Texture<Spectrum>>& kd,const std::shared_ptr<Texture<Float>>& sigma):_kd(kd),_sigma(sigma){
	}
	virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const override{
		//1.先为SurfaceInteraction分配bsdf
		si->bsdf=ARENA_ALLOC(arena,BSDF)(*si);
		//2.获取反射率，并且为BSDF添加BxDF
		Float sigma=_sigma->Evaluate(*si);
		Spectrum r=_kd->Evaluate(*si);
		
		//当sigma为0的时候使用完美漫反射
		if(sigma==0){
			si->bsdf->Add(ARENA_ALLOC(arena,LambertianReflection(r)));
		}
		//不然使用OrenNayar分布
		else{
			si->bsdf->Add(ARENA_ALLOC(arena,OrenNayar(r,sigma)));
		}
		
	}
	virtual ~MatteMaterial(){};
};

MatteMaterial* CreateMatteMaterial(const TextureParams&mp);

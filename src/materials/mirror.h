/*
 * mirror.h
 *
 *  Created on: 2017年1月5日
 *      Author: zhuqian
 */

#ifndef SRC_MATERIALS_MIRROR_H_
#define SRC_MATERIALS_MIRROR_H_

#include "raiden.h"
#include "material.h"
#include "memory.h"
#include "interaction.h"
#include "reflection.h"
#include "texture.h"
class MirrorMaterial: public Material {
private:
	std::shared_ptr<Texture<Spectrum>> _kr;//完美漫反射的反射率
public:

	MirrorMaterial(const std::shared_ptr<Texture<Spectrum>>& kr):_kr(kr){
	}
	virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const override{
		//1.先为SurfaceInteraction分配bsdf
		si->bsdf=ARENA_ALLOC(arena,BSDF)(*si);
		//2.获取反射率，并且为BSDF添加BxDF
		Spectrum r=_kr->Evaluate(*si);
		if(r.IsBlack()){
			return;
		}
		si->bsdf->Add(ARENA_ALLOC(arena,SpecularReflection)(r , ARENA_ALLOC(arena,FresnelNop)()  ));
	}
	virtual ~MirrorMaterial(){};
};

MirrorMaterial* CreateMirrorMaterial(const TextureParams&mp);

#endif /* SRC_MATERIALS_MIRROR_H_ */

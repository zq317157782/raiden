/*
 * hair.h
 *
 *  Created on: 2019年1月9日
 *      Author: zhuqian
 */

#pragma once

#include "raiden.h"
#include "material.h"
#include "memory.h"
#include "interaction.h"
#include "reflection.h"
#include "texture.h"
#include "microfacet.h"
class HairMaterial:public Material {
private:
    std::shared_ptr<Texture<Float>> _betaM;
public:

	HairMaterial(const std::shared_ptr<Texture<Float>>& betaM):_betaM(betaM){
	}

	virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const override{

        si->bsdf=ARENA_ALLOC(arena,BSDF)(*si);
		//计算Mp的粗糙度
        auto betaM = _betaM->Evaluate(*si);
        auto hair=ARENA_ALLOC(arena,HairBSDF(0,1.55,0,betaM,1,0));
        si->bsdf->Add(hair);
	}
	virtual ~HairMaterial(){};
};

HairMaterial* CreateHairMaterial(const TextureParams&mp);
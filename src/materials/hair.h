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
    std::shared_ptr<Texture<Float>> _eta;
    std::shared_ptr<Texture<Float>> _betaM;
    std::shared_ptr<Texture<Float>> _betaN;
    std::shared_ptr<Texture<Spectrum>> _sigmaA;
public:

	HairMaterial(const std::shared_ptr<Texture<Float>>& eta,const std::shared_ptr<Texture<Float>>& betaM,const std::shared_ptr<Texture<Float>>& betaN,const std::shared_ptr<Texture<Spectrum>>& sigmaA):_eta(eta),_betaM(betaM),_betaN(betaN),_sigmaA(sigmaA){
	}

	virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const override{

        si->bsdf=ARENA_ALLOC(arena,BSDF)(*si);
        
        auto eta=_eta->Evaluate(*si);
		//计算Mp的粗糙度
        auto betaM = _betaM->Evaluate(*si);
        //计算Np的粗糙度
        auto betaN = _betaN->Evaluate(*si);

        //计算吸收率
        auto sigmaA=_sigmaA->Evaluate(*si);
        Float h=si->uv[1]*2-1;
        auto hair=ARENA_ALLOC(arena,HairBSDF(h,eta,sigmaA,betaM,betaN,0));
        si->bsdf->Add(hair);
	}
	virtual ~HairMaterial(){};
};

HairMaterial* CreateHairMaterial(const TextureParams&mp);
/*
 * grass.h
 *
 *  Created on: 2017年1月6日
 *      Author: zhuqian
 */

#ifndef SRC_MATERIALS_GLASS_H_
#define SRC_MATERIALS_GLASS_H_

#include "raiden.h"
#include "material.h"
#include "memory.h"
#include "interaction.h"
#include "reflection.h"
#include "texture.h"
#include "microfacet.h"

class GlassMaterial : public Material {
private:
	std::shared_ptr<Texture<Spectrum>> _kr;
	std::shared_ptr<Texture<Spectrum>> _kt;
	std::shared_ptr<Texture<Float>> _eta;
	std::shared_ptr<Texture<Float>> _roughnessX;
	std::shared_ptr<Texture<Float>> _roughnessY;
public:

	GlassMaterial(const std::shared_ptr<Texture<Spectrum>>& kr, const std::shared_ptr<Texture<Spectrum>>& kt, const std::shared_ptr<Texture<Float>>& eta,const std::shared_ptr<Texture<Float>>& roughnessX,const std::shared_ptr<Texture<Float>>& roughnessY):_kr(kr),_kt(kt),_eta(eta),_roughnessX(roughnessX),_roughnessY(roughnessY){
		
	}
	virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
		MemoryArena &arena, TransportMode mode,
		bool allowMultipleLobes) const override {
		//1.先为SurfaceInteraction分配bsdf
		Spectrum R = _kr->Evaluate(*si);
		Spectrum T = _kt->Evaluate(*si);
		Float eta = _eta->Evaluate(*si);
		si->bsdf = ARENA_ALLOC(arena, BSDF)(*si,eta);
		if(R.IsBlack()&&T.IsBlack()){
			return;	
		}

		//2.判断是否是镜面反射或者折射
		Float rx = _roughnessX->Evaluate(*si);
		Float ry = _roughnessY->Evaluate(*si);
		bool isSpecular=(rx==0)&&(ry==0);
		//3.判断是否需要微平面分布
		AnisotropyGGXDistribution *ggx=(isSpecular==true)?nullptr:ARENA_ALLOC(arena,AnisotropyGGXDistribution)(AnisotropyGGXDistribution::RoughnessToAlpha(rx),AnisotropyGGXDistribution::RoughnessToAlpha(ry));
		
		FresnelDielectric * fresnel=ARENA_ALLOC(arena, FresnelDielectric)(1.0,eta);

		if (isSpecular&&allowMultipleLobes) {
			si->bsdf->Add(ARENA_ALLOC(arena,FresnelSpecular)(R,T,1,eta,mode));
		}
		else {
			if (!R.IsBlack()) {
				if(isSpecular){
					si->bsdf->Add(ARENA_ALLOC(arena, SpecularReflection)(R,fresnel));
				}
				else{
					si->bsdf->Add(ARENA_ALLOC(arena,MicrofacetReflection)(R,ggx,fresnel));
				}
			}

			if (!T.IsBlack()) {
				if(isSpecular){
					si->bsdf->Add(ARENA_ALLOC(arena, SpecularTransmission)(T,1.0,eta,mode));
				}
				else{
					si->bsdf->Add(ARENA_ALLOC(arena,MicrofacetTransmission)(T,ggx,fresnel,1.0,eta,mode));
				}
			}
		}
		
	}
	virtual ~GlassMaterial() {};
};

GlassMaterial* CreateGlassMaterial(const TextureParams&mp);


#endif /* SRC_MATERIALS_GLASS_H_ */

/*
 * subsurface.h
 *
 *  Created on: 2019年1月30日
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
#include "bssrdf.h"
#include "primitive.h"
class SubsurfaceMaterial:public Material {
private:
    BSSRDFTable _table; 
    std::shared_ptr<Texture<Spectrum>> _sigmaS;
    std::shared_ptr<Texture<Spectrum>> _sigmaA;
    Float _eta;
    Float _g;
    std::shared_ptr<Texture<Spectrum>> _kr;
	std::shared_ptr<Texture<Spectrum>> _kt;
	std::shared_ptr<Texture<Float>> _roughness;
public:
	SubsurfaceMaterial(const std::shared_ptr<Texture<Spectrum>>& kr, const std::shared_ptr<Texture<Spectrum>>& kt,const std::shared_ptr<Texture<Float>>& roughness,const std::shared_ptr<Texture<Spectrum>>& sigmaS,const std::shared_ptr<Texture<Spectrum>>& sigmaA,Float eta,Float g):_table(100,64),_kr(kr),_kt(kt),_roughness(roughness),_sigmaS(sigmaS),_sigmaA(sigmaA),_eta(eta),_g(g){
        ComputeBeamDiffusionBSSRDF(_g,_eta,&_table);
	}
	virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const override{
                //1.先为SurfaceInteraction分配bsdf
		Spectrum R = _kr->Evaluate(*si);
		Spectrum T = _kt->Evaluate(*si);
        Float eta=_eta;
		si->bsdf = ARENA_ALLOC(arena, BSDF)(*si,eta);
		if(R.IsBlack()&&T.IsBlack()){
			return;	
		}

		//2.判断是否是镜面反射或者折射
		Float r = _roughness->Evaluate(*si);
		bool isSpecular=(r==0);
		//3.判断是否需要微平面分布
		IsotropyGGXDistribution *ggx=(isSpecular==true)?nullptr:ARENA_ALLOC(arena, IsotropyGGXDistribution)(GGXRoughnessToAlpha(r));
		
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
					si->bsdf->Add(ARENA_ALLOC(arena, SpecularTransmission)(T,1,eta,mode));
				}
				else{
					si->bsdf->Add(ARENA_ALLOC(arena,MicrofacetTransmission)(T,ggx,fresnel,1,eta,mode));
				}
			}
		}


            Spectrum sigmaS=_sigmaS->Evaluate(*si);
            Spectrum sigmaA=_sigmaA->Evaluate(*si);
            //const SurfaceInteraction &po,Float eta,const Spectrum& sigmaS,const Spectrum& sigmaA,const BSSRDFTable& table
            si->bssrdf=ARENA_ALLOC(arena,TabulatedBSSRDF)(*si,si->primitive->GetMaterial(), _eta,sigmaS,sigmaA,_table);
	}
	virtual ~SubsurfaceMaterial(){};
};

SubsurfaceMaterial* CreateSubsurfaceMaterial(const TextureParams&mp);

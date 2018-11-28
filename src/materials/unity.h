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
#include "microfacet.h"
class UnityMaterial:public Material {
private:
    std::shared_ptr<Texture<Spectrum>> _albedo;
    std::shared_ptr<Texture<Float>> _metallic;
    std::shared_ptr<Texture<Float>> _roughness;
public:

	UnityMaterial(const std::shared_ptr<Texture<Spectrum>>& albedo,const std::shared_ptr<Texture<Float>>& metallic,const std::shared_ptr<Texture<Float>>& roughness):_albedo(albedo),_metallic(metallic),_roughness(roughness){
	}
	virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const override{

        si->bsdf=ARENA_ALLOC(arena,BSDF)(*si);
		//1.计算金属度
        auto metallic_scale = _metallic->Evaluate(*si);
        //2.计算粗糙度
        auto sigma = GGXRoughnessToAlpha(_roughness->Evaluate(*si));
        //3.计算albedo
        auto albedo = _albedo->Evaluate(*si);
 
        //当sigma为0的时候使用完美漫反射
		if(sigma==0){
            auto diffuse=ARENA_ALLOC(arena,LambertianReflection(albedo));
            auto scaled_diffuse=ARENA_ALLOC(arena,ScaledBxDF(diffuse,1-metallic_scale));
			si->bsdf->Add(scaled_diffuse);
		}
		//不然使用OrenNayar分布
		else{
            auto diffuse= ARENA_ALLOC(arena,OrenNayar(albedo,sigma));
            auto scaled_diffuse=ARENA_ALLOC(arena,ScaledBxDF(diffuse,1-metallic_scale));
			si->bsdf->Add(scaled_diffuse);
		}
        
        //4.获得GGX分布
        IsotropyGGXDistribution *ggx=ARENA_ALLOC(arena,IsotropyGGXDistribution)(sigma);

        //5.获得Fresnel 使用Schlick近似
        Fresnel *fresnel=ARENA_ALLOC(arena,FresnelSchlick)(albedo);
		auto specular =ARENA_ALLOC(arena,MicrofacetReflection)(1.0,ggx,fresnel);
        auto scaled_specular =ARENA_ALLOC(arena,ScaledBxDF(specular,metallic_scale));
        si->bsdf->Add(scaled_specular);
	}
	virtual ~UnityMaterial(){};
};

UnityMaterial* CreateUnityMaterial(const TextureParams&mp);
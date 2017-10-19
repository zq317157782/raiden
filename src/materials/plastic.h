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
class PlasticMaterial:public Material {
private:
    std::shared_ptr<Texture<Spectrum>> _kd;//漫反射反射率
    std::shared_ptr<Texture<Spectrum>> _ks;//镜面反射反射率
    std::shared_ptr<Texture<Float>> _roughness;
public:

	PlasticMaterial(const std::shared_ptr<Texture<Spectrum>>& kd,const std::shared_ptr<Texture<Spectrum>>& ks,const std::shared_ptr<Texture<Float>>& roughness):_kd(kd),_ks(ks),_roughness(roughness){
	}
	virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const override{
		//1.先为SurfaceInteraction分配bsdf
		si->bsdf=ARENA_ALLOC(arena,BSDF)(*si);
        //2.Diffuse部分
        Spectrum kd=_kd->Evaluate(*si);
        if(!kd.IsBlack()){
            //使用Lambertian Diffuse
            si->bsdf->Add(ARENA_ALLOC(arena,LambertianReflection(kd)));
        }

        //3.Specular部分
        Spectrum ks=_ks->Evaluate(*si);
        if(!ks.IsBlack()){
            Float roughness=_roughness->Evaluate(*si);
            //使用各项同性GGX配置(虽然是各向异性分布,难道塑料没有各项异性？)
            roughness=AnisotropyGGXDistribution::RoughnessToAlpha(roughness);
            AnisotropyGGXDistribution *ggx=ARENA_ALLOC(arena,AnisotropyGGXDistribution)(roughness,roughness);
            //绝缘体的菲涅尔系数,为啥是1.5的IOR？塑料是1.5？
            FresnelDielectric * fresnel=ARENA_ALLOC(arena, FresnelDielectric)(1.0,1.5);
            si->bsdf->Add(ARENA_ALLOC(arena,MicrofacetReflection)(ks,ggx,fresnel));
        }
		
	
		
	}
	virtual ~PlasticMaterial(){};
};

PlasticMaterial* CreatePlasticMaterial(const TextureParams&mp);

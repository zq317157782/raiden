#pragma once
#include "raiden.h"
#include "material.h"
#include "memory.h"
#include "interaction.h"
#include "reflection.h"
#include "texture.h"
#include "microfacet.h"
class MetalMaterial : public Material {
private:
	std::shared_ptr<Texture<Spectrum>> _absorb;
	std::shared_ptr<Texture<Spectrum>> _eta;
    std::shared_ptr<Texture<Float>> _roughnessX;
    std::shared_ptr<Texture<Float>> _roughnessY;
public:

	MetalMaterial(const std::shared_ptr<Texture<Spectrum>>& eta,const std::shared_ptr<Texture<Spectrum>>& absorb,const std::shared_ptr<Texture<Float>>& roughnessX,const std::shared_ptr<Texture<Float>>& roughnessY) 
	:_absorb(absorb),_eta(eta),_roughnessX(roughnessX),_roughnessY(roughnessY){    
    }

    
	virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
		MemoryArena &arena, TransportMode mode,
		bool allowMultipleLobes) const override {
		//1.先为SurfaceInteraction分配bsdf
		Spectrum eta = _eta->Evaluate(*si);
		Spectrum absorb = _absorb->Evaluate(*si);
		Float rx = _roughnessX->Evaluate(*si);
		Float ry = _roughnessY->Evaluate(*si);
        si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
        
        //2.获得GGX分布
        AnisotropyGGXDistribution *ggx=ARENA_ALLOC(arena,AnisotropyGGXDistribution)(AnisotropyGGXDistribution::RoughnessToAlpha(rx),AnisotropyGGXDistribution::RoughnessToAlpha(ry));
        
        //3.获得Fresnel
        Fresnel *fresnel=ARENA_ALLOC(arena,FresnelConductor)(1.0,eta,absorb);
		
        si->bsdf->Add(ARENA_ALLOC(arena,MicrofacetReflection)(1.0,ggx,fresnel));
	}
	virtual ~MetalMaterial() {};
};

MetalMaterial* CreateMetalMaterial(const TextureParams&mp);

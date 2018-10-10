#include "metal.h"
#include "paramset.h"
MetalMaterial* CreateMetalMaterial(const TextureParams&mp){
    std::shared_ptr<Texture<Spectrum>>eta = mp.GetSpectrumTexture("eta", Spectrum(1.0f));
    std::shared_ptr<Texture<Spectrum>> absorb = mp.GetSpectrumTexture("absorb", Spectrum(1.0f));
    std::shared_ptr<Texture<Float>> roughness = mp.GetFloatTexture("roughness", 1.0f);
	Debug("[CreateMetalMaterial]");
	return new MetalMaterial(eta,absorb,roughness);
}
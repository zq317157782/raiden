#include "plastic.h"
#include "paramset.h"
PlasticMaterial* CreatePlasticMaterial(const TextureParams&mp){
    std::shared_ptr<Texture<Spectrum>> kd = mp.GetSpectrumTexture("Kd", Spectrum(1.0f));
    std::shared_ptr<Texture<Spectrum>> ks = mp.GetSpectrumTexture("Ks", Spectrum(1.0f));
    std::shared_ptr<Texture<Float>> roughness = mp.GetFloatTexture("roughness", 1.0f);
	Debug("[CreatePlasticMaterial]");
	return new PlasticMaterial(kd,ks,roughness);
}
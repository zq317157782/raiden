#include "unity.h"
#include "paramset.h"
UnityMaterial* CreateUnityMaterial(const TextureParams&mp){
    std::shared_ptr<Texture<Spectrum>> albedo=mp.GetSpectrumTexture("albedo",Spectrum(1.0));
    std::shared_ptr<Texture<Float>> metallic=mp.GetFloatTexture("metallic",Float(1.0));
    std::shared_ptr<Texture<Float>> roughness=mp.GetFloatTexture("roughness",Float(0.0));
	Debug("[CreateUnityMaterial]");
	return new UnityMaterial(albedo,metallic,roughness);
}
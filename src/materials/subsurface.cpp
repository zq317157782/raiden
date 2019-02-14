#include "subsurface.h"
#include "paramset.h"

SubsurfaceMaterial* CreateSubsurfaceMaterial(const TextureParams&mp){
    
	std::shared_ptr<Texture<Spectrum>> r = mp.GetSpectrumTexture("Kr", Spectrum(1.0f));
	std::shared_ptr<Texture<Spectrum>> t = mp.GetSpectrumTexture("Kt", Spectrum(1.0f));
	std::shared_ptr<Texture<Float>> roughness = mp.GetFloatTexture("roughness", 0.0f);
    
    std::shared_ptr<Texture<Spectrum>> sigmaS = mp.GetSpectrumTexture("sigmas", Spectrum(0.0f));
    std::shared_ptr<Texture<Spectrum>> sigmaA = mp.GetSpectrumTexture("sigmaa", Spectrum(0.0f));
    Float eta=mp.FindFloat("eta",1.5f);
    Float g=mp.FindFloat("g",0);


	return new SubsurfaceMaterial(r,t,roughness,sigmaS,sigmaA,eta,g);
}
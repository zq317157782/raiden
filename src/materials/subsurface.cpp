#include "subsurface.h"
#include "paramset.h"

SubsurfaceMaterial* CreateSubsurfaceMaterial(const TextureParams&mp){
    std::shared_ptr<Texture<Spectrum>> sigmaS = mp.GetSpectrumTexture("sigmas", Spectrum(0.0f));
    std::shared_ptr<Texture<Spectrum>> sigmaA = mp.GetSpectrumTexture("sigmaa", Spectrum(0.0f));
    Float eta=mp.FindFloat("eta",1);
    Float g=mp.FindFloat("g",1);
	return new SubsurfaceMaterial(sigmaS,sigmaA,eta,g);
}
#include "matte.h"
#include "paramset.h"
MatteMaterial* CreateMatteMaterial(const TextureParams&mp){
    std::shared_ptr<Texture<Spectrum>> r=mp.GetSpectrumTexture("Kd",Spectrum(1.0));
    std::shared_ptr<Texture<Float>> sigma=mp.GetFloatTexture("sigma",Float(0.0));
	return new MatteMaterial(r,sigma);
}



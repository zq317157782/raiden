/*
 * grass.cpp
 *
 *  Created on: 2017年1月6日
 *      Author: zhuqian
 */
#include <materials/glass.h>
#include "paramset.h"
GlassMaterial* CreateGlassMaterial(const TextureParams&mp) {
	std::shared_ptr<Texture<Float>>eta = mp.GetFloatTexture("eta", (1.5f));
	std::shared_ptr<Texture<Spectrum>> r = mp.GetSpectrumTexture("Kr", Spectrum(1.0f));
	std::shared_ptr<Texture<Spectrum>> t = mp.GetSpectrumTexture("Kt", Spectrum(1.0f));
	std::shared_ptr<Texture<Float>> roughness = mp.GetFloatTexture("roughness", 0.0f);
	return new GlassMaterial(r,t,eta,roughness);
}





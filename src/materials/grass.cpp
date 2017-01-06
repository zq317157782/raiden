/*
 * grass.cpp
 *
 *  Created on: 2017年1月6日
 *      Author: zhuqian
 */
#include "grass.h"
#include "paramset.h"
GrassMaterial* CreateGrassMaterial(const TextureParams&mp) {
	std::shared_ptr<Texture<Float>>eta = mp.GetFloatTexture("eta", (1.0f));
	std::shared_ptr<Texture<Spectrum>> r = mp.GetSpectrumTexture("Kr", Spectrum(1.0f));
	std::shared_ptr<Texture<Spectrum>> t = mp.GetSpectrumTexture("Kt", Spectrum(1.0f));
	Debug("[CreateGrassMaterial]");
	return new GrassMaterial(r,t,eta);
}





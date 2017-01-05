/*
 * mirror.cpp
 *
 *  Created on: 2017年1月5日
 *      Author: zhuqian
 */
#include "mirror.h"
#include "paramset.h"
MirrorMaterial* CreateMirrorMaterial(const TextureParams&mp){
	std::shared_ptr<Texture<Spectrum>> r=mp.GetSpectrumTexture("Kr",Spectrum(1.0f));
	Debug("[CreateMirrorMaterial]");
	return new MirrorMaterial(r);
}




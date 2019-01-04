/*
 * lambertian.cpp
 *
 *  Created on: 2016年12月30日
 *      Author: zhuqian
 */
#include "lambertian.h"
#include "paramset.h"
LambertianMaterial* CreateLambertianMaterial(const TextureParams&mp){
	std::shared_ptr<Texture<Spectrum>> r=mp.GetSpectrumTexture("Kd",Spectrum(1.0f));
	return new LambertianMaterial(r);
}



/*
 * constant.cpp
 *
 *  Created on: 2016年12月29日
 *      Author: zhuqian
 */
#include "constant.h"
#include "paramset.h"
#include "spectrum.h"
ConstantTexture<Float> *CreateConstantFloatTexture(const Transform &tex2world,
                                                   const TextureParams &tp){
	//从纹理参数中寻找一个Float变量
	return new ConstantTexture<Float>(tp.FindFloat("value",1.0f));
}

ConstantTexture<Spectrum> *CreateConstantSpectrumTexture(const Transform &tex2world,
                                                   const TextureParams &tp){
	return new ConstantTexture<Spectrum>(tp.FindSpectrum("value",Spectrum(1.0f)));
}


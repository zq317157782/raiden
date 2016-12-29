/*
 * constant.h
 *
 *  Created on: 2016年12月29日
 *      Author: zhuqian
 */

#ifndef SRC_TEXTURES_CONSTANT_H_
#define SRC_TEXTURES_CONSTANT_H_
#include "raiden.h"
#include "texture.h"
//常量纹理
template<typename T>
class ConstantTexture: public Texture<T> {
private:
	T _value;
public:
	ConstantTexture(const T& value):_value(value){

	}
	virtual T Evaluate(const SurfaceInteraction & is) const override {
		return _value;
	}
	virtual ~ConstantTexture(){}
};

ConstantTexture<Float> *CreateConstantFloatTexture(const Transform &tex2world,
                                                   const TextureParams &tp);

#endif /* SRC_TEXTURES_CONSTANT_H_ */

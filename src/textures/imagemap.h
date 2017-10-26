/*
 * imagetexture.h
 *
 *  Created on: 2017年10月26日
 *      Author: zhuqian
 */

#pragma once
#include "raiden.h"
#include "texture.h"
//ImageTexture
//PBRT是实现了储存使用一种格式，返回也使用一种格式的方式
//我目前不使用全频谱的格式，因此我这边只实现储存和查询是一种格式的纹理
//以后有需求再重构成两种格式的
template<typename T>
class ImageTexture: public Texture<T> {
private:
	const std::unique_ptr<TextureMapping2D> _mapping;//纹理映射函数

public:
	ImageTexture(std::unique_ptr<TextureMapping2D> mapping):_mapping(std::move(mapping)) {
	}
	virtual T Evaluate(const SurfaceInteraction & is) const override {
		
	}
	virtual ~ImageTexture(){}
};

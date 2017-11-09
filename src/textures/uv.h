/*
 * checkerboard2d.h
 *
 *  Created on: 2017年11月9日
 *      Author: zhuqian
 */

#pragma once
#include "raiden.h"
#include "texture.h"

//UV纹理
class UVTexture: public Texture<Spectrum> {
private:
	const std::unique_ptr<TextureMapping2D> _mapping;//纹理映射函数

public:
	UVTexture(std::unique_ptr<TextureMapping2D> mapping):_mapping(std::move(mapping)){

	}
	virtual Spectrum Evaluate(const SurfaceInteraction & is) const override {
		Vector2f dstdx, dstdy;
		Point2f st = _mapping->Map(is, &dstdx, &dstdy);
        Float rgb[3]={st[0]-std::floor(st[0]),st[1]-std::floor(st[1]),0};
        return Spectrum::FromRGB(rgb);
	}
	virtual ~UVTexture(){}
};

UVTexture *CreateUVTexture(const Transform &tex2world,
	const TextureParams &tp);
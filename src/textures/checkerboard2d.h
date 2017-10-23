/*
 * checkerboard2d.h
 *
 *  Created on: 2017年10月23日
 *      Author: zhuqian
 */

#pragma once
#include "raiden.h"
#include "texture.h"
//Checkerboard2D纹理
template<typename T>
class Checkerboard2DTexture: public Texture<T> {
private:
	const std::unique_ptr<TextureMapping2D> _mapping;//纹理映射函数
	const std::shared_ptr<Texture<T>> _t0;//纹理0
	const std::shared_ptr<Texture<T>> _t1;//纹理1

public:
	Checkerboard2DTexture(std::unique_ptr<TextureMapping2D> mapping, const std::shared_ptr<Texture<T>>& tex0, const std::shared_ptr<Texture<T>>& tex1):_mapping(std::move(mapping)), _t0(tex0), _t1(tex1) {

	}
	virtual T Evaluate(const SurfaceInteraction & is) const override {
		Vector2f dstdx, dstdy;
		Point2f st = _mapping->Map(is, &dstdx, &dstdy);
		if ((int(std::floor(st[0]) + std::floor(st[1]))) % 2 == 0) {
			return _t0->Evaluate(is);
		}
		else {
			return _t1->Evaluate(is);
		}
	}
	virtual ~Checkerboard2DTexture(){}
};


Checkerboard2DTexture<Float> *CreateCheckerboard2DFloatTexture(const Transform &tex2world,
	const TextureParams &tp);
Checkerboard2DTexture<Spectrum> *CreateCheckerboard2DSpectrumTexture(const Transform &tex2world,
	const TextureParams &tp);
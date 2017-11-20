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
public:
	//反走样方法
	enum class AAMethod {
		None, ClosedForm
	};
private:
	const std::unique_ptr<TextureMapping2D> _mapping;//纹理映射函数
	const std::shared_ptr<Texture<T>> _t0;//纹理0
	const std::shared_ptr<Texture<T>> _t1;//纹理1

	const AAMethod _aa;

public:
	Checkerboard2DTexture(std::unique_ptr<TextureMapping2D> mapping, const std::shared_ptr<Texture<T>>& tex0, const std::shared_ptr<Texture<T>>& tex1, AAMethod aa):_mapping(std::move(mapping)), _t0(tex0), _t1(tex1), _aa(aa){

	}
	virtual T Evaluate(const SurfaceInteraction & is) const override {
		Vector2f dstdx, dstdy;
		Point2f st = _mapping->Map(is, &dstdx, &dstdy);
		
		//不使用任何的AA技术
		if (_aa == AAMethod::None) {
			if ((int(std::floor(st[0]) + std::floor(st[1]))) % 2 == 0) {
				return _t0->Evaluate(is);
			}
			else {
				return _t1->Evaluate(is);
			}
		}
		//使用AA技术
		//使用轴对齐的过滤区域
		else {
			//获得过滤器的half width
			Float ds = std::max(std::abs(dstdx[0]), std::abs(dstdy[0]));
			Float dt = std::max(std::abs(dstdx[1]), std::abs(dstdy[1]));
			//计算过滤器的左下和右上坐标
			Float s0 = st[0] - ds;
			Float s1 = st[0] + ds;
			Float t0 = st[1] - dt;
			Float t1 = st[1] + dt;

			//判断是否在一个tex中，是的话使用point采样
			if (std::floor(s0) == std::floor(s1) && std::floor(t0) == std::floor(t1)) {
				if ((int(std::floor(st[0]) + std::floor(st[1]))) % 2 == 0) {
					return _t0->Evaluate(is);
				}
				else {
					return _t1->Evaluate(is);
				}
			}
			
			//过滤纹理

			//计算1D Step函数的积分[x的下取整是even的时候取0，不然取1]
			auto stepInt = [](Float x) {
				//后半段的意思大致是先取X一半，然后去除整数部分，然后小于0.5取0，大于0.5取减去0.5后的数，然后再从0~0.5的空间映射到0~1的空间
				return (int)std::floor(x / 2) + 2*std::max((Float)0, (Float)(x / 2 - std::floor(x / 2) - 0.5));
			};
			
			//取平均值
			Float averageS = (stepInt(s1) - stepInt(s0)) / (2 * ds);
			Float averageT = (stepInt(t1) - stepInt(t0)) / (2 * dt);
			//计算2D面积，这里没有理解，PBRT也没有详细的解释
			//到底为啥要用这个公式呢？？？？
			Float area2 = averageS*averageT - 2 * averageS*averageT;

			//在过滤器的某一个边大于2的时候，直接取0.5的面积
			//这里PBRT也没有解释，为啥不解释，难道是很简单的道理？我TM不懂啊！！！！！？
			//其实我大致是知道为啥的
			if (ds > 1 || dt > 1) {
				area2 = 0.5;
			}

			return (1- area2)*_t0->Evaluate(is)+area2*_t1->Evaluate(is);

		}
	}
	virtual ~Checkerboard2DTexture(){}
};


Checkerboard2DTexture<Float> *CreateCheckerboard2DFloatTexture(const Transform &tex2world,
	const TextureParams &tp);
Checkerboard2DTexture<Spectrum> *CreateCheckerboard2DSpectrumTexture(const Transform &tex2world,
	const TextureParams &tp);
/*
 * api.cpp
 *
 *  Created on: 2016年12月16日
 *      Author: zhuqian
 */
#include "api.h"
#include "transform.h"
#include "paramset.h"
//transform相关参数
constexpr int MaxTransforms = 2;
constexpr int StartTransformBits = 1 << 0;	//0x01
constexpr int EndTransformBits = 1 << 1;	//0x10
constexpr int AllTransformsBits = (1 << MaxTransforms) - 1;	//0x11
//transform集合
struct TransformSet {
private:
	Transform _t[MaxTransforms];
public:
	Transform& operator[](int i) {
		Assert(i >= 0 && i < MaxTransforms);
		return _t[i];
	}
	const Transform& operator[](int i) const {
		Assert(i >= 0 && i < MaxTransforms);
		return _t[i];
	}
	//判断是否包含不同的transform
	bool IsAnimated() const {
		for (int i = 0; i < MaxTransforms - 1; ++i) {
			if (_t[i] != _t[i + 1]) {
				return true;
			}
		}
		return false;
	}
	//返回transformSet的逆
	friend TransformSet Inverse(const TransformSet& ts) {
		TransformSet invSet;
		for (int i = 0; i < MaxTransforms; ++i) {
			invSet._t[i] = Inverse(ts._t[i]);
		}
		return invSet;
	}
};
//渲染参数
struct RenderOptions {
	Float transformStartTime = 0;
	Float transformEndTime = 1;
	//过滤器名字  默认 box
	std::string FilterName = "box";
	ParamSet FilterParams;
	//Film的名字
	std::string FilmName = "image";
	ParamSet FilmParams;
	//采样器的名字
	std::string SamplerName = "random";
	ParamSet SamplerParams;
	//加速结构的名字
	std::string AcceleratorName = "normal";
	ParamSet AcceleratorParams;
	//积分器的名字
	std::string IntegratorName = "normal";
	ParamSet IntegratorParams;
	//相机的名字
	std::string CameraName = "pinhole";
	ParamSet CameraParams;
	//光源
	std::vector<std::shared_ptr<Light>> lights;
	//图元
	std::vector<std::shared_ptr<Primitive>> primitives;
};

/*
 * api.cpp
 *
 *  Created on: 2016年12月16日
 *      Author: zhuqian
 */
#include "api.h"
#include "transform.h"
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
	bool IsAnimated() const{
		for(int i=0;i<MaxTransforms-1;++i){
			if(_t[i]!=_t[i+1]){
				return true;
			}
		}
		return false;
	}
	//返回transformSet的逆
	friend TransformSet Inverse(const TransformSet& ts){
		TransformSet invSet;
		for(int i=0;i<MaxTransforms;++i){
			invSet._t[i]=Inverse(ts._t[i]);
		}
		return invSet;
	}
};


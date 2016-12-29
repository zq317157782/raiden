/*
 * texture.h
 *
 *  Created on: 2016年12月29日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_TEXTURE_H_
#define SRC_CORE_TEXTURE_H_
#include "raiden.h"
#include "interaction.h"
//二维纹理映射
class TextureMapping2D {
public:
	virtual ~TextureMapping2D() {
	}
	;
	//返回相应的纹理坐标以及Texture(s,t)相对于Screen(x,y)的偏导
	virtual Point2f Map(const SurfaceInteraction &si, Vector2f *dstdx,
			Vector2f *dstdy) const = 0;
};
//uv参数坐标到st纹理坐标的映射
class UVMapping2D: TextureMapping2D {
private:
	Float _su; //dsdu
	Float _sv; //dtdv
	Float _du;
	Float _dv;
public:
	UVMapping2D(Float su = 1.0, Float sv = 1.0, Float du = 0, Float dv = 0) :
			_su(su), _sv(sv), _du(du), _dv(dv) {

	}

	virtual Point2f Map(const SurfaceInteraction &si, Vector2f *dstdx,
			Vector2f *dstdy) const override {
		*dstdx = Vector2f(_su * si.dudx, _sv * si.dvdx); //链式法则
		*dstdy = Vector2f(_su * si.dudy, _sv * si.dvdy); //链式法则
		return Point2f(_su * si.uv.x + _du, _sv * si.uv.y + _dv); //一阶近似
	}
};

//纹理接口
template<typename T>
class Texture {
public:
	virtual T Evaluate(const SurfaceInteraction & is) const = 0;
	virtual ~Texture() {
	}
};

#endif /* SRC_CORE_TEXTURE_H_ */

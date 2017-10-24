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
class UVMapping2D: public TextureMapping2D {
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

//把Object的坐标(x,y,z)投影到相应的Sphere坐标(theta,phi)上，然后再标准化到(s,t)空间中
class SphericalMapping2D:public TextureMapping2D{
private:
	const Transform _world2Tex;
	Float _su; //dsdu
	Float _sv; //dtdv
	Float _du;
	Float _dv;

	//把(x,y,z)转换到(s,t)
	Point2f PointToSphereToST(const Point3f& pointW) const {
		//首先转换到纹理空间
		Point3f p = _world2Tex(pointW);
		Vector3f v = Normalize(p - Point3f(0, 0, 0));
		//计算球面坐标
		Float theta = SphericalTheta(v);
		Float phi = SphericalPhi(v);
		//标准化到(s,t)空间
		return Point2f(theta*InvPi,phi*Inv2Pi);
	}
public:

	SphericalMapping2D(const Transform& w2t, Float su = 1.0, Float sv = 1.0, Float du = 0, Float dv = 0) :_world2Tex(w2t), _su(su), _sv(sv), _du(du), _dv(dv) {

	}
	virtual Point2f Map(const SurfaceInteraction &si, Vector2f *dstdx,
		Vector2f *dstdy) const override {
		
		//计算st
		Point2f st = PointToSphereToST(si.p);
		//计算近似偏导
		Float delta = 0.1;//dX&dY
		*dstdx = (PointToSphereToST(si.p + delta*si.dpdx) - st) / delta;
		*dstdy = (PointToSphereToST(si.p + delta*si.dpdy) - st) / delta;
		//考虑phi在0和2Pi之间的间断点情况
		//从0到2PI
		if ((*dstdx)[1] > 0.5) {
			(*dstdx)[1] = (1 - (*dstdx)[1]);
		}
		//从2PI到0
		else if ((*dstdx)[1] < -0.5) {
			(*dstdx)[1] = -(1 + (*dstdx)[1]);
		}
		
		//从0到2PI
		if ((*dstdy)[1] > 0.5) {
			(*dstdy)[1] = (1 - (*dstdy)[1]);
		}
		//从2PI到0
		else if ((*dstdy)[1] < -0.5) {
			(*dstdy)[1] = -(1 + (*dstdy)[1]);
		}
		return Point2f(_su * st.x + _du, _sv * st.y + _dv);
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

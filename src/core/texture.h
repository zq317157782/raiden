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
#include "mmath.h"
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

//把Object的坐标(x,y,z)投影到相应的圆柱体坐标(theta,z)上，然后再标准化到(s,t)空间中
class CylindricalMapping2D:public TextureMapping2D{
private:
	const Transform _world2Tex;
	Float _su; //dsdu
	Float _sv; //dtdv
	Float _du;
	Float _dv;

	//把(x,y,z)转换到(s,t)
	Point2f PointToCylinderToST(const Point3f& pointW) const {
		//首先转换到纹理空间
		Point3f p = _world2Tex(pointW);
		Vector3f v = Normalize(p - Point3f(0, 0, 0));
		//计算球面坐标
		Float theta = (Pi+std::atan2(v.y,v.x));
		//标准化到(s,t)空间
		return Point2f(theta*Inv2Pi,v.z);
	}
public:
		CylindricalMapping2D(const Transform& w2t,Float su = 1.0, Float sv = 1.0, Float du = 0, Float dv = 0) :_world2Tex(w2t), _su(su), _sv(sv), _du(du), _dv(dv){
	
		}
		virtual Point2f Map(const SurfaceInteraction &si, Vector2f *dstdx,
			Vector2f *dstdy) const override {
			
			//计算st
			Point2f st = PointToCylinderToST(si.p);
			//计算近似偏导
			Float delta = 0.01;//dX&dY
			*dstdx = (PointToCylinderToST(si.p + delta*si.dpdx) - st) / delta;
			*dstdy = (PointToCylinderToST(si.p + delta*si.dpdy) - st) / delta;
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

class PlanarMapping2D:public TextureMapping2D{
private:
	//组成平面坐标系的两根轴，可以非正交
	Vector3f _vs;
	Vector3f _vt;
	//scale
	Float _ss; 
	Float _st; 
	//delta
	Float _ds;
	Float _dt;
public:
	PlanarMapping2D(const Vector3f& vs,const Vector3f& vt,Float ss=1,Float st=1,Float ds=0,Float dt=0):_vs(vs),_vt(vt),_ss(ss),_st(st),_ds(ds),_dt(dt){

	}
	
	virtual Point2f Map(const SurfaceInteraction &si, Vector2f *dstdx,
		Vector2f *dstdy) const override {
			Vector3f vec(si.p);
			//计算偏导在平面空间下的值
			*dstdx=Vector2f(Dot(_vs,si.dpdx),Dot(_vt,si.dpdx));
			*dstdy=Vector2f(Dot(_vs,si.dpdy),Dot(_vt,si.dpdy));
			//转换到平面空间
			return Point2f(Dot(_vs,vec)*_ss+_ds,Dot(_vt,vec)*_st+_dt);
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

// Lanczos分布
//Sinc(X)*Lanczos
Float Lanczos(Float x, Float tau);
#endif /* SRC_CORE_TEXTURE_H_ */

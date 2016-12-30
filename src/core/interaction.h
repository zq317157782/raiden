/*
 * intersection.h
 *
 *  Created on: 2016年11月22日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_INTERACTION_H_
#define SRC_CORE_INTERACTION_H_
#include "raiden.h"
#include "geometry.h"
#include "shape.h"
class Interaction {
public:
	Point3f p; //交点
	Normal3f n; //表面法线
	Vector3f pErr; //累积的浮点数绝对误差
	Vector3f wo; //入射方向
	Float time; //时间
public:
	Interaction() :
			time(0) {
	}
	Interaction(const Point3f& pp, const Normal3f& nn, const Vector3f& perr,
			const Vector3f& wo, Float t) :
			p(pp), n(nn), pErr(perr), wo(wo), time(t){
	}
};

//表面交点，射线与几何体表面之间的交点
class SurfaceInteraction: public Interaction {
public:
	Point2f uv; //表面的参数坐标
	Vector3f dpdu, dpdv; //参数坐标和空间点之间的一阶导数/梯度
	Normal3f dndu, dndv; //参数坐标和法线之间的一阶导数/梯度
	const Shape* shape;
	const Primitive* primitive;
	BSDF* bsdf;
	//todo BSSRDF指针
	struct {
		Normal3f n;
		Vector3f dpdu, dpdv;
		Normal3f dndu, dndv;
	} shading;//非几何着色信息
	mutable Vector3f dpdx,dpdy;//空间点和屏幕坐标之间的导数/梯度
	mutable Float dudx,dudy,dvdx,dvdy;
public:
	SurfaceInteraction() {}
	SurfaceInteraction(const Point3f &p, const Vector3f &pError,
	                       const Point2f &uv, const Vector3f &wo,
	                       const Vector3f &dpdu, const Vector3f &dpdv,
	                       const Normal3f &dndu, const Normal3f &dndv, Float time,
	                       const Shape *sh);
	//计算差分信息
	void ComputeDifferentials(const RayDifferential &r) const;
};

#endif /* SRC_CORE_INTERACTION_H_ */

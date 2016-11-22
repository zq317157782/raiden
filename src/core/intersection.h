/*
 * intersection.h
 *
 *  Created on: 2016年11月22日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_INTERSECTION_H_
#define SRC_CORE_INTERSECTION_H_
#include "raiden.h"
#include "geometry.h"
class Intersection {
public:
	Point3f p; //交点
	Normal3f n; //表面法线
	Vector3f pErr; //累积的浮点数绝对误差
	Vector3f wo; //入射方向
	Float time; //时间
	MediumInterface mediumInterface;
public:
	Intersection() :
			time(0) {
	}
	Intersection(const Point3f& pp, const Normal3f& nn, const Vector3f& perr,
			const Vector3f& wo, Float t, const MediumInterface& mediumInterface) :
			p(pp), n(nn), pErr(perr), wo(wo), time(t), mediumInterface(
					mediumInterface) {
	}
};

//表面交点，射线与几何体表面之间的交点
class SurfaceIntersection: public Intersection {
public:
	Point3f uv; //表面的参数坐标
	Vector3f dpdu, dpdv; //参数坐标和空间点之间的一阶导数/梯度
	Normal3f dndu, dndv; //参数坐标和法线之间的一阶导数/梯度
	//todo Shape指针
	//todo Primitive指针
	//todo BSDF指针
	//todo BSSRDF指针
	struct {
		Point3f uv;
		Vector3f dpdu, dpdv;
		Normal3f dndu, dndv;
	} shading;//非几何着色信息
	mutable Vector3f dpdx,dpdy;//空间点和屏幕坐标之间的导数/梯度
	mutable Float dudx,dudy,dvdx,dvdy;
};

#endif /* SRC_CORE_INTERSECTION_H_ */

/*
 * sphere.h
 *
 *  Created on: 2016年11月23日
 *      Author: zhuqian
 */

#ifndef SRC_SHAPES_SPHERE_H_
#define SRC_SHAPES_SPHERE_H_
#include "shape.h"

class Sphere : public Shape
{
  private:
	const Float _radius;					   //球体的半径
	const Float _zMin, _zMax;				   //costheta最小值和最大值,如果_zMin和_zMax小于_radius的话
	const Float _thetaMin, _thetaMax, _phiMax; //球体坐标相关参数
  public:
	//构造函数主要的作用是计算出球体参数坐标，包括theta的范围，以及phi的范围
	Sphere(const Transform *objectToWorld, const Transform *worldToObject,
		   bool reverseOrientation, Float radius, Float zMin, Float zMax, Float phiMax /*角度*/)
		: Shape(objectToWorld, worldToObject, reverseOrientation),
		  _radius(radius),
		  _zMin(Clamp(std::min(zMin, zMax), -radius, radius)),
		  _zMax(Clamp(std::max(zMin, zMax), -radius, radius)),
		  _thetaMin(std::acos(Clamp(std::min(zMin, zMax) / radius, -1, 1))),
		  _thetaMax(std::acos(Clamp(std::max(zMin, zMax) / radius, -1, 1))),
		  _phiMax(Radians(Clamp(phiMax, 0, 360)))
	{
	}

	Bound3f ObjectBound() const override;
	Float Area() const override;
	bool Intersect(const Ray &ray, Float *tHit, SurfaceInteraction *surfaceIsect, bool testAlpha = true) const override;
	bool IntersectP(const Ray &ray, bool testAlpha = true) const override;
	Interaction Sample(const Point2f &uv, Float *pdf) const override;
	Interaction Sample(const Interaction &ref, const Point2f &u, Float *pdf) const override;
	inline friend std::ostream &operator<<(std::ostream &os, const Sphere &s)
	{
		os << "[sphere radius:" <<s._radius<< " , min z:" << s._zMin << " , max z:"<<s._zMax<<" , max phi:"<<s._phiMax<<"]";
		return os;
	}
};

std::shared_ptr<Shape> CreateSphereShape(const Transform *o2w,
										 const Transform *w2o,
										 bool reverseOrientation,
										 const ParamSet &params);

#endif /* SRC_SHAPES_SPHERE_H_ */

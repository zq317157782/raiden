/*
 * shape.h
 *
 *  Created on: 2016年11月22日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_SHAPE_H_
#define SRC_CORE_SHAPE_H_
#include "raiden.h"
#include "geometry.h"
#include "intersection.h"
#include "transform.h"

class Shape {
public:
	const Transform* objectToWorld, *worldToObject;
	const bool reverseOrientation; //是否改变法线方向
	const bool transformSwapsHandedness; //变换后是否会改变坐标系
public:
	Shape(const Transform *objectToWorld, const Transform *worldToObject,
			bool reverseOrientation);
	virtual ~Shape();

	//局部包围盒和世界包围盒
	virtual Bound3f ObjectBound() const=0;
	virtual Bound3f WorldBound() const;

	//相交测试函数
	virtual bool Intersect(const Ray& ray,Float* tHit,SurfaceIntersection* surfaceIsect,bool testAlpha=true) const=0;
	virtual bool IntersectP(const Ray& ray,bool testAlpha=true) const{return Intersect(ray,nullptr,nullptr,testAlpha);}

	//返回Shape表面面积
	virtual Float Area() const=0;

	//采样Shape上的一个点以及相应的PDF
	virtual Intersection Sample(const Point2f& uv,Float *pdf) const=0;
	virtual Float Pdf(const Intersection&) const{return 1.0f/Area();}//默认返回均匀分布Pdf

};

#endif /* SRC_CORE_SHAPE_H_ */

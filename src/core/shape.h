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


};

#endif /* SRC_CORE_SHAPE_H_ */

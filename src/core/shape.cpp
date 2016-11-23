/*
 * shape.cpp
 *
 *  Created on: 2016年11月22日
 *      Author: zhuqian
 */
#include "shape.h"
#include "stats.h"

STAT_COUNTER("Shape created",nShapesCreated);
Shape::Shape(const Transform *objectToWorld, const Transform *worldToObject,
			bool reverseOrientation) :
			objectToWorld(objectToWorld), worldToObject(worldToObject), reverseOrientation(
					reverseOrientation),transformSwapsHandedness(objectToWorld->SwapsHandedness()){
	nShapesCreated++;
}

Shape::~Shape(){};

Bound3f Shape::WorldBound() const{
	return (*objectToWorld)(ObjectBound());
}

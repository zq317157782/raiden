/*
 * shape.cpp
 *
 *  Created on: 2016年11月22日
 *      Author: zhuqian
 */
#include "shape.h"
#include "stats.h"

Shape::Shape(const Transform *objectToWorld, const Transform *worldToObject,
			bool reverseOrientation) :
			objectToWorld(objectToWorld), worldToObject(worldToObject), reverseOrientation(
					reverseOrientation),transformSwapsHandedness(objectToWorld->SwapsHandedness()){
}

Shape::~Shape(){};

Bound3f Shape::WorldBound() const{
	return (*objectToWorld)(ObjectBound());
}


Float Shape::Pdf(const Interaction& ref, const Vector3f& wi) const {
	SurfaceInteraction lightRef;
	Ray r = ref.SpawnRay(wi);
	Float tHit;
	if (!Intersect(r, &tHit, &lightRef, false)) {
		return 0;
	}

	Float pdf = DistanceSquared(lightRef.p, ref.p) / (AbsDot(lightRef.n, -wi)*Area());
	if (std::isinf(pdf)) {
		return 0;
	}
	return pdf;
}

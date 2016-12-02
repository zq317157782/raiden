/*
 * interaction.cpp
 *
 *  Created on: 2016年11月22日
 *      Author: zhuqian
 */
#include "interaction.h"
SurfaceInteraction::SurfaceInteraction(const Point3f &p, const Vector3f &pError,
		const Point2f &uv, const Vector3f &wo, const Vector3f &dpdu,
		const Vector3f &dpdv, const Normal3f &dndu, const Normal3f &dndv,
		Float time, const Shape *sh) :
		Interaction(p, Normal3f(Cross(dpdu, dpdv)), pError, wo, time), uv(uv), dpdu(dpdu), dpdv(dpdv), dndu(dndu), dndv(
				dndv), shape(sh) {
	//初始化着色数据
	shading.n=n;
	shading.dpdu=dpdu;
	shading.dpdv=dpdv;
	shading.dndu=dndu;
	shading.dndv=dndv;
	//判断是否要翻转法线信息
	if(shape&&(shape->reverseOrientation^shape->transformSwapsHandedness)){
		n*=-1;
		shading.n*=-1;
	}
}


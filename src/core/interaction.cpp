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
		Interaction(p, Normal3f(Cross(dpdu, dpdv)), pError, wo, time), uv(uv), dpdu(
				dpdu), dpdv(dpdv), dndu(dndu), dndv(dndv), shape(sh) {
	//初始化着色数据
	shading.n = n;
	shading.dpdu = dpdu;
	shading.dpdv = dpdv;
	shading.dndu = dndu;
	shading.dndv = dndv;
	//判断是否要翻转法线信息
	if (shape
			&& (shape->reverseOrientation ^ shape->transformSwapsHandedness)) {
		n *= -1;
		shading.n *= -1;
	}
}

//计算差分
void SurfaceInteraction::ComputeDifferentials(const RayDifferential &r) const {
	if (r.hasDifferential) {
		//需要计算差分
		Float d = -Dot(n, Vector3f(p.x, p.y, p.z)); //交点所在的平面为 Dot(n,p)+d=0
		//计算辅助射线和平面的交点
		Float tx = (-d - Dot(n, Vector3f(r.ox.x, r.ox.y, r.ox.z)))
				/ Dot(n, r.dx);
		if (std::isnan(tx)) {
			//射线和平面完全平行的情况
			goto fail;
		}
		Point3f px = r.ox + tx * r.dx;
		//同理计算py
		Float ty = (-d - Dot(n, Vector3f(r.oy.x, r.oy.y, r.oy.z)))
				/ Dot(n, r.dy);
		if (std::isnan(ty)) {
			//射线和平面完全平行的情况
			goto fail;
		}
		Point3f py = r.oy + ty * r.dy;
		//计算偏导
		dpdx = px - p;
		dpdy = py - p;
		//3个方程式，2个未知变量，所以有一个方程式是另外两个的线性组合
		int dim[2];
		if (std::abs(n.x) > std::abs(n.y) && std::abs(n.x) > std::abs(n.z)) {
			dim[0] = 1;
			dim[1] = 2;
		} else if (std::abs(n.y) > std::abs(n.z)) {
			dim[0] = 0;
			dim[1] = 2;
		} else {
			dim[0] = 0;
			dim[1] = 1;
		}

		//求解线性系统
		Float A[2][2] = { { dpdu[dim[0]], dpdv[dim[0]] }, { dpdu[dim[1]],
				dpdv[dim[1]] } };
		Float Bx[2] = { px[dim[0]] - p[dim[0]], px[dim[1]] - p[dim[1]] };
		if (!SolveLinearSystem2x2(A, Bx, &dudx, &dvdx)) {
			dudx = dvdx = 0;
		}
		Float By[2] = { py[dim[0]] - p[dim[0]], py[dim[1]] - p[dim[1]] };
		if (!SolveLinearSystem2x2(A, By, &dudy, &dvdy)) {
			dudy = dvdy = 0;
		}
	} else {
		//不需要计算差分
		fail: dudx = dvdx = 0;
		dudy = dvdy = 0;
		dpdx = dpdy = Vector3f(0, 0, 0);
	}
}

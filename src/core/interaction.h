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
#include "material.h"
#include "spectrum.h"
#include "medium.h"
class Interaction {
public:
	Point3f p; //交点
	Normal3f n; //表面法线
	Vector3f pErr; //累积的浮点数绝对误差
	Vector3f wo; //入射方向
	Float time; //时间
	MediumInterface mediumInterface;
public:
	Interaction() :
			time(0) {
	}
	Interaction(const Point3f& pp, const Normal3f& nn, const Vector3f& perr,
			const Vector3f& wo, Float t,const MediumInterface& mi) :
			p(pp), n(nn), pErr(perr), wo(wo), time(t), mediumInterface(mi){
	}

	Interaction(const Point3f &p, Float time, const MediumInterface& mi) :
			p(p), time(time), mediumInterface(mi) {
	}
	Interaction(const Point3f &p, const Vector3f &wo, Float time,
		const MediumInterface &mediumInterface)
		: p(p), time(time), wo(wo), mediumInterface(mediumInterface) {}

	bool IsSurfaceInteraction() const {
		return n != Normal3f();
	}
	bool IsMediumInteraction() const { 
		return !IsSurfaceInteraction(); 
	}
	//给予方向,生成新的射线
	Ray SpawnRay(const Vector3f& d) const {
		Point3f o = OffsetRayOrigin(p, pErr, n, d);
		return Ray(o, d, Infinity, time,GetMedium(d));
	}
	//给予空间点，生成射向目标点的射线
	Ray SpawnRayTo(const Point3f& p2) const {
		Point3f o = OffsetRayOrigin(p, pErr, n, p2 - p);
		Vector3f d = p2 - o;
		return Ray(o, d, 1 - ShadowEpsilon, time, GetMedium(d));
	}

	Ray SpawnRayTo(const Interaction &it) const {
		Point3f origin = OffsetRayOrigin(p, pErr, n, it.p - p);
		Point3f target = OffsetRayOrigin(it.p, it.pErr, it.n, origin - it.p);
		Vector3f d = target - origin;
		return Ray(origin, d, 1 - ShadowEpsilon, time, GetMedium(d));
	}

	const Medium *GetMedium(const Vector3f &w) const {
		return Dot(w, n) > 0 ? mediumInterface.outside : mediumInterface.inside;
	}
	const Medium *GetMedium() const {
		Assert(mediumInterface.inside==mediumInterface.outside);
		return mediumInterface.inside;
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
	BSDF* bsdf=nullptr;
	//todo BSSRDF指针
	struct {
		Normal3f n;
		Vector3f dpdu, dpdv;
		Normal3f dndu, dndv;
	} shading; //非几何着色信息
	mutable Vector3f dpdx, dpdy; //空间点和屏幕坐标之间的导数/梯度
	mutable Float dudx, dudy, dvdx, dvdy;
public:
	SurfaceInteraction() {
	}
	SurfaceInteraction(const Point3f &p, const Vector3f &pError,
			const Point2f &uv, const Vector3f &wo, const Vector3f &dpdu,
			const Vector3f &dpdv, const Normal3f &dndu, const Normal3f &dndv,
			Float time, const Shape *sh);
	//计算差分信息
	void ComputeDifferentials(const RayDifferential &r) const;
	void ComputeScatteringFunctions(const RayDifferential &ray,
			MemoryArena &arena, bool allowMultipleLobes = false,
			TransportMode mode = TransportMode::Radiance);

	Spectrum Le(const Vector3f& w) const ;

	void SetShadingGeometry(const Vector3f &dpdus,
			const Vector3f &dpdvs, const Normal3f &dndus, const Normal3f &dndvs,
			bool orientationIsAuthoritative);
};

//代表介质中的一个交点
class MediumInteraction : public Interaction {
public:
	const PhaseFunction *phase;
public:
	MediumInteraction() : phase(nullptr) {}
	MediumInteraction(const Point3f &p, const Vector3f &wo, Float time,
		const Medium *medium, const PhaseFunction *phase)
		: Interaction(p, wo, time, MediumInterface(medium)), phase(phase) {}
	//判断是否是有效的MediumInteraction
	bool IsValid() const { return phase != nullptr; }
	
};


class UVInteraction {
	public:
		Point3f  p;
	    Normal3f n;
	public:
	UVInteraction(){}
	UVInteraction(const Point3f &pp,const Normal3f &nn){
		p=pp;
		n=nn;
	}
};


#endif /* SRC_CORE_INTERACTION_H_ */

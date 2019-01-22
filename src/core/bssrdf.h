#pragma once
#include "raiden.h"
#include "geometry.h"
//双向次表面反射分布函数
class BSSRDF{
private:
	const SurfaceInteraction &_po;//出射点和方向
	Float _eta;//折射系数
public:
	BSSRDF(const SurfaceInteraction &po,Float eta):_po(po),_eta(eta){}
	//S(po,wo,pi,wi)
	//出射的微元radiance和入射的微元flux之比
	virtual Spectrum S(const Point3f& pi,const Vector3f& wi) const=0;
};

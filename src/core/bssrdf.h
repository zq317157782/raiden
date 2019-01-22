#pragma once
#include "raiden.h"
#include "geometry.h"

//从PBRT复制的两个工具函数
//使用多项式近似来近似菲涅尔的第一M和第二M
//参数使用的是折射系数的倒数(和PBRT的实现有关)
Float FresnelMoment1(Float invEta);
Float FresnelMoment2(Float invEta);

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

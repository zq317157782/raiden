/*
 * point.cpp
 *
 *  Created on: 2016年12月8日
 *      Author: zhuqian
 */
#include "point.h"
#include "interaction.h"
Spectrum PointLight::Sample_Li(Interaction& itr,Vector3f* wi,Float* pdf) const{
	*wi=Normalize(Vector3f(_position-itr.p));//计算光线方向
	*pdf=1;
	return _I/DistanceSquared(_position,itr.p);//随着距离的增加，空间角变小，所以能量也变小了(或者是受光面积相对光源变小,irradiance随之变小)
}



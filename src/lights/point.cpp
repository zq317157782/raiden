/*
 * point.cpp
 *
 *  Created on: 2016年12月8日
 *      Author: zhuqian
 */
#include "point.h"
#include "interaction.h"
#include "paramset.h"
#include "sampling.h"
Spectrum PointLight::Sample_Li(const Interaction& itr,const Point2f &u,Vector3f* wi,Float* pdf, VisibilityTester* vis) const{
	*wi=Normalize(Vector3f(_position-itr.p));//计算光线方向
	*pdf=1;
	*vis = VisibilityTester(itr, Interaction(_position, itr.time));
	return _I/DistanceSquared(_position,itr.p);//随着距离的增加，空间角变小，所以能量也变小了(或者是受光面积相对光源变小,irradiance随之变小)
}

Float PointLight::Pdf_Li(const Interaction &ref, const Vector3f &wi) const{
	return 0;
}

Spectrum PointLight::Power() const{
	//点光源的话，dA不考虑，所以flux等于整个球体空间角4pi乘以光源的强度
	return _I*4*Pi;
}

Spectrum PointLight::Sample_Le(const Point2f &u1/*position样本*/,
			const Point2f &u2/*direction样本*/, Float time, Ray *ray,
			Normal3f *normalLight, Float *pdfPos, Float *pdfDir) const{
	*ray=Ray(_position,UniformSampleSphere(u1),Infinity,time);
	*normalLight=(Normal3f)ray->d;//得到法线,因为在光源局部坐标下，所以_position就是原点
	*pdfPos=1.0f;//采样点光源的位置永远在同一个位置，所以pdf为1
	*pdfDir=UniformSpherePdf();//均匀采样球体的pdf
	return _I;
}

void PointLight::Pdf_Le(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
					Float *pdfDir) const{
	//永远都不可能通过采样采样到点光源的position
	*pdfPos=0;
	*pdfDir=UniformSpherePdf();
}

std::shared_ptr<PointLight> CreatePointLight(const Transform &light2world,const ParamSet &paramSet) {
    Spectrum I = paramSet.FindOneSpectrum("I", Spectrum(1.0));
    Spectrum sc = paramSet.FindOneSpectrum("scale", Spectrum(1.0));
    Point3f P = paramSet.FindOnePoint3f("from", Point3f(0, 0, 0));
    Transform l2w = Translate(Vector3f(P.x, P.y, P.z)) * light2world;
	Debug("[CreatePointLight]");
    return std::make_shared<PointLight>(l2w,I * sc);
}



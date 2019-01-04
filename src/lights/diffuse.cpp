/*
 * diffuse.cpp
 *
 *  Created on: 2017年1月17日
 *      Author: zhuqian
 */
#include "diffuse.h"
#include "paramset.h"
#include "interaction.h"
#include "sampling.h"
Spectrum DiffuseAreaLight::Sample_Li(const Interaction& ref, const Point2f &u,
		Vector3f* wi, Float* pdf, VisibilityTester* vis) const {
	//采样光源上的一个空间点
	Interaction shapeP = _shape->Sample(ref, u, pdf);
	//还需要判断两个点之间的距离知否为0
	if (*pdf == 0 || (DistanceSquared(shapeP.p, ref.p) == 0)) {
		*pdf = 0;
		return 0;
	}
	*wi = Normalize(shapeP.p - ref.p);
	*vis = VisibilityTester(ref, shapeP);
	return L(shapeP, -*wi);
}

Float DiffuseAreaLight::Pdf_Li(const Interaction &ref,
		const Vector3f &wi) const {
	return _shape->Pdf(ref, wi);
}

Spectrum DiffuseAreaLight::Power() const {
	//计算flux的公式:radiance*area*st
	return _Le * _area * Pi;
}

Spectrum DiffuseAreaLight::Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
			Ray *ray, Normal3f *nLight, Float *pdfPos, Float *pdfDir) const{
	//先从shape上采样交点
	Interaction ref=_shape->Sample(u1,pdfPos);
	ref.mediumInterface=mediumInterface;
	//采样方向,light空间
	Vector3f wi=CosineSampleHemisphere(u2);
	*pdfDir=CosineHemispherePdf(wi.z);
	//获得法线
	*nLight=ref.n;
	//转换到相应的世界坐标
	Vector3f v1,v2,n(ref.n);
	CoordinateSystem(n,&v1,&v2);
	//转换
	Vector3f w=v1*wi.x+v2*wi.y+n*wi.z;
	//产生射线
	*ray=ref.SpawnRay(w);
	return L(ref,w);
}
void DiffuseAreaLight::Pdf_Le(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
	Float *pdfDir) const {
	auto ref = Interaction(ray.o, nLight,Vector3f(),Vector3f(nLight),ray.time,mediumInterface);
	*pdfPos=_shape->Pdf(ref);//计算posPdf
	*pdfDir = CosineHemispherePdf(Dot(ray.d,nLight));
}

std::shared_ptr<AreaLight> CreateDiffuseAreaLight(const Transform &light2world,
		const ParamSet &paramSet, const std::shared_ptr<Shape> &shape) {
	Spectrum L = paramSet.FindOneSpectrum("L", Spectrum(1.0));
	Spectrum sc = paramSet.FindOneSpectrum("scale", Spectrum(1.0));
	int nSamples = paramSet.FindOneInt("nsamples", 1);
	return std::make_shared<DiffuseAreaLight>(light2world,nSamples, L * sc,shape,nullptr);
}


/*
 * diffuse.cpp
 *
 *  Created on: 2017年1月17日
 *      Author: zhuqian
 */
#include "diffuse.h"
#include "paramset.h"
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

std::shared_ptr<AreaLight> CreateDiffuseAreaLight(const Transform &light2world,
		const ParamSet &paramSet, const std::shared_ptr<Shape> &shape) {
	Spectrum L = paramSet.FindOneSpectrum("L", Spectrum(1.0));
	Spectrum sc = paramSet.FindOneSpectrum("scale", Spectrum(1.0));
	int nSamples = paramSet.FindOneInt("nsamples", 1);
	Debug("[CreateDiffuseAreaLight]");
	return std::make_shared<DiffuseAreaLight>(light2world,nSamples, L * sc,shape,nullptr);
}


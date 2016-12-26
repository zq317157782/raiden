/*
 * reflection.cpp
 *
 *  Created on: 2016年12月6日
 *      Author: zhuqian
 */
#include "reflection.h"
#include "sampling.h"
#include "spectrum.h"

bool Refract(const Vector3f& wi, const Normal3f& n,
		Float oeta/*这里是两个折射率之比(i/t)*/, Vector3f* wt) {
	//1.先求cosThetaT
	//2.然后使用推导的折射方向公式
	Float cosThetaI = Dot(wi, n);
	Float sinThetaI2 = std::max(0.0f, 1.0f - cosThetaI * cosThetaI);
	//这里运用snell law
	Float sinThetaT2 = oeta * sinThetaI2;
	if (sinThetaT2 >= 1) {
		return false;	//完全反射情况
	}
	Float cosThetaT = std::sqrt(1.0f - sinThetaT2);
	//代入公式
	*wt = oeta * (-wi) + (oeta * Dot(wi, n) - cosThetaT) * Vector3f(n);
	return true;
}

Float FrDielectric(Float cosThetaI, Float etaI, Float etaT) {
	cosThetaI = Clamp(cosThetaI, -1, 1);
	//cosThetaI小于0说明，入射光线在内部，需要交换两个ETA
	if (cosThetaI < 0.0f) {
		std::swap(etaI, etaT);
		cosThetaI = std::abs(cosThetaI);
	}
	//计算折射光线和法线之间的夹角
	//运用Snell's law
	Float sinThetaI = std::sqrt(std::max(0.0f, 1.0f - cosThetaI * cosThetaI));
	Float sinThetaT = (etaI / etaT) * sinThetaI;
	//判断是否发生全反射
	if (sinThetaT >= 1.0f) {
		return 1.0f;
	}
	//运用Fresnel equation计算反射系数
	Float cosThetaT = std::sqrt(std::max(0.0f, 1.0f - sinThetaT * sinThetaT));
	Float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT))
			/ ((etaT * cosThetaI) + (etaI * cosThetaT));
	Float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT))
			/ ((etaI * cosThetaI) + (etaT * cosThetaT));
	return (Rparl * Rparl + Rperp * Rperp) / 2;
}

Spectrum BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
		Float *pdf, BxDFType *sampledType) const {
	//1.采样入射射线
	//2.调整wi到wo相同半球
	//3.返回brdf
	*wi = CosineSampleHemisphere(sample);
	if (!SameHemisphere(wo, *wi)) {
		wi->z *= -1;
	}
	*pdf = Pdf(wo, *wi);
	return f(wo, *wi);
}

Float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const {
	if (SameHemisphere(wo, wi)) {
		return AbsCosTheta(wi) * InvPi;
	} else {
		return 0.0f;
	}
}

Spectrum BxDF::rho(const Vector3f &wo, int nSamples,
		const Point2f *samples) const {
	//使用蒙特卡洛法计算r-h反射系数
	Spectrum ret(0);
	for (int i = 0; i < nSamples; ++i) {
		Vector3f wi;
		Float pdf;
		//采样pdf
		Spectrum f = Sample_f(wo, &wi, samples[i], &pdf);
		if (pdf > 0.0f) {
			ret += (f * AbsCosTheta(wi)) / pdf;
		}
	}
	return ret / nSamples;
}

Spectrum BxDF::rho(int nSamples, const Point2f *samples1,
		const Point2f *samples2) const {
	Spectrum ret(0);
	for (int i = 0; i < nSamples; ++i) {
		Vector3f wi;
		Float wiPdf;
		//均匀采样出射方向
		Vector3f wo = UniformSampleHemisphere(samples1[i]);
		Float woPdf = UniformHemispherePdf();
		//采样pdf
		Spectrum f = Sample_f(wo, &wi, samples2[i], &wiPdf);
		ret += (f * AbsCosTheta(wo) * AbsCosTheta(wi)) / (woPdf * wiPdf);
	}
	return ret / (nSamples * Pi);
}

/*
 * reflection.cpp
 *
 *  Created on: 2016年12月6日
 *      Author: zhuqian
 */
#include "reflection.h"
#include "sampling.h"
#include "spectrum.h"
#include "microfacet.h"

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

//没想到这个公式是参考塞巴斯的博客的
// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
Spectrum FrConductor(Float cosThetaI, const Spectrum &etai,
                     const Spectrum &etat, const Spectrum &k) {
    cosThetaI = Clamp(cosThetaI, -1.0f, 1.0f);
    Spectrum eta = etat / etai;
    Spectrum etak = k / etai;

    Float cosThetaI2 = cosThetaI * cosThetaI;
    Float sinThetaI2 = 1.0f - cosThetaI2;
    Spectrum eta2 = eta * eta;
    Spectrum etak2 = etak * etak;

    Spectrum t0 = eta2 - etak2 - sinThetaI2;
    Spectrum a2plusb2 = Sqrt(t0 * t0 + 4 * eta2 * etak2);
    Spectrum t1 = a2plusb2 + cosThetaI2;
    Spectrum a = Sqrt(0.5f * (a2plusb2 + t0));
    Spectrum t2 = (Float)2 * cosThetaI * a;
    Spectrum Rs = (t1 - t2) / (t1 + t2);

    Spectrum t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
    Spectrum t4 = t2 * sinThetaI2;
    Spectrum Rp = Rs * (t3 - t4) / (t3 + t4);

    return 0.5f * (Rp + Rs);
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

Spectrum LambertianTransmission::Sample_f(const Vector3f &wo, Vector3f *wi,
			const Point2f &sample, Float *pdf,
			BxDFType *sampledType ) const {
		//1.采样入射射线
		//2.调整wi到wo不同半球
		//3.返回brdf
		*wi = CosineSampleHemisphere(sample);
		if(wo.z>0.0f){
			wi->z*=-1.0f;
		}
		*pdf = Pdf(wo, *wi);
		return f(wo, *wi);
	}



	Spectrum MicrofacetReflection::f(const Vector3f &wo, const Vector3f &wi) const  {
		
		//计算分母的系数
		Float cosThetaWo=AbsCosTheta(wo);
		Float cosThetaWi=AbsCosTheta(wi);
		//判断退化情况
		if(cosThetaWo==0||cosThetaWi==0){
			return 0.0;
		}
		//计算半角向量
		Vector3f wh=wo+wi;
		if(wh.x==0&&wh.y==0&&wh.z==0){
			return 0.0;
		}
		wh=Normalize(wh);
		//计算菲涅尔反射系数
		Spectrum F=_fresnel->Evaluate(Dot(wi,wh)); //Dot(wo,wh)也可以

		return _R*(F*_distribution->D(wh)*_distribution->G(wo,wi))/(4*cosThetaWo*cosThetaWi);
	}


	Spectrum MicrofacetTransmission::f(const Vector3f &wo, const Vector3f &wi) const {
		//首先判断wo和wi是否在同一个半球内
		if(SameHemisphere(wo,wi)){
			return 0.0;
		}

		Float cosThetaWo=CosTheta(wo);
		Float cosThetaWi=CosTheta(wi);
		
		if(cosThetaWo==0||cosThetaWi==0){
			return 0.0;
		}

		//判断wo的折射率是A还是B
		Float eta=cosThetaWo>0?(_etaB/_etaA):(_etaA/_etaB);

		//计算半角向量
		Vector3f wh=Normalize(wo+eta*wi);
		if(wh.z<0){
			wh=-wh;//半角永远要在微平面的上半球
		}

		//计算Fresnel
		Spectrum F=_fresnel->Evaluate(Dot(wo,wh));//这里不能使用Dot(wo,wh)
		
		Float term=Dot(wo,wh)+eta*Dot(wi,wh);
		term=term*term;

		Float factor=(_mode==TransportMode::Radiance)?(1/eta):1.0;

		return eta*eta*_distribution->D(wh)*_distribution->G(wo,wi)*(Spectrum(1.0)-F)*AbsDot(wi,wh)*AbsDot(wo,wh)*factor*factor/(term*cosThetaWo*cosThetaWi);
	}
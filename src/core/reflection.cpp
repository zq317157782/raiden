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
#include <array>

bool Refract(const Vector3f &wi, const Normal3f &n,
			 Float oeta /*这里是两个折射率之比(i/t)*/, Vector3f *wt)
{
	//1.先求cosThetaT
	//2.然后使用推导的折射方向公式
	Float cosThetaI = Dot(wi, n);
	Float sinThetaI2 = std::max((Float)0.0f, (Float)1.0f - cosThetaI * cosThetaI);
	//这里运用snell law
	Float sinThetaT2 = oeta * sinThetaI2;
	if (sinThetaT2 >= 1)
	{
		return false; //完全反射情况
	}
	Float cosThetaT = std::sqrt(1.0f - sinThetaT2);
	//代入公式
	*wt = oeta * (-wi) + (oeta * Dot(wi, n) - cosThetaT) * Vector3f(n);
	return true;
}

Float FrDielectric(Float cosThetaI, Float etaI, Float etaT)
{
	cosThetaI = Clamp(cosThetaI, -1, 1);
	//cosThetaI小于0说明，入射光线在内部，需要交换两个ETA
	if (cosThetaI < 0.0f)
	{
		std::swap(etaI, etaT);
		cosThetaI = std::abs(cosThetaI);
	}
	//计算折射光线和法线之间的夹角
	//运用Snell's law
	Float sinThetaI = std::sqrt(std::max((Float)0.0f, (Float)1.0f - cosThetaI * cosThetaI));
	Float sinThetaT = (etaI / etaT) * sinThetaI;
	//判断是否发生全反射
	if (sinThetaT >= 1.0f)
	{
		return 1.0f;
	}
	//运用Fresnel equation计算反射系数
	Float cosThetaT = std::sqrt(std::max((Float)0.0f, (Float)1.0f - sinThetaT * sinThetaT));
	Float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) / ((etaT * cosThetaI) + (etaI * cosThetaT));
	Float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) / ((etaI * cosThetaI) + (etaT * cosThetaT));
	return (Rparl * Rparl + Rperp * Rperp) / 2;
}

Spectrum SchlickFresnel(const Spectrum &R, Float cosTheta)
{
	auto pow5 = [](Float a) { return (a * a) * (a * a) * a; };
	return R + (Spectrum(1) - R) * pow5(1 - cosTheta);
}

//没想到这个公式是参考塞巴斯的博客的
// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
Spectrum FrConductor(Float cosThetaI, const Spectrum &etai,
					 const Spectrum &etat, const Spectrum &k)
{
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
	Spectrum a = Sqrt(0.5 * (a2plusb2 + t0));
	Spectrum t2 = (Float)2 * cosThetaI * a;
	Spectrum Rs = (t1 - t2) / (t1 + t2);

	Spectrum t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
	Spectrum t4 = t2 * sinThetaI2;
	Spectrum Rp = Rs * (t3 - t4) / (t3 + t4);

	return 0.5 * (Rp + Rs);
}

Spectrum BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
						Float *pdf, BxDFType *sampledType) const
{
	//1.采样入射射线
	//2.调整wi到wo相同半球
	//3.返回brdf
	*wi = CosineSampleHemisphere(sample);
	if (!SameHemisphere(wo, *wi))
	{
		wi->z *= -1;
	}
	*pdf = Pdf(wo, *wi);
	return f(wo, *wi);
}

Float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
	if (SameHemisphere(wo, wi))
	{
		return AbsCosTheta(wi) * InvPi;
	}
	else
	{
		return 0.0f;
	}
}

Spectrum BxDF::rho(const Vector3f &wo, int nSamples,
				   const Point2f *samples) const
{
	//使用蒙特卡洛法计算r-h反射系数
	Spectrum ret(0);
	for (int i = 0; i < nSamples; ++i)
	{
		Vector3f wi;
		Float pdf;
		//采样pdf
		Spectrum f = Sample_f(wo, &wi, samples[i], &pdf);
		if (pdf > 0.0f)
		{
			ret += (f * AbsCosTheta(wi)) / pdf;
		}
	}
	return ret / nSamples;
}

Spectrum BxDF::rho(int nSamples, const Point2f *samples1,
				   const Point2f *samples2) const
{
	Spectrum ret(0);
	for (int i = 0; i < nSamples; ++i)
	{
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
										  BxDFType *sampledType) const
{
	//1.采样入射射线
	//2.调整wi到wo不同半球
	//3.返回brdf
	*wi = CosineSampleHemisphere(sample);
	if (wo.z > 0.0f)
	{
		wi->z *= -1.0f;
	}
	*pdf = Pdf(wo, *wi);
	return f(wo, *wi);
}

Spectrum MicrofacetReflection::f(const Vector3f &wo, const Vector3f &wi) const
{

	//计算分母的系数
	Float cosThetaWo = AbsCosTheta(wo);
	Float cosThetaWi = AbsCosTheta(wi);
	//判断退化情况
	if (cosThetaWo == 0 || cosThetaWi == 0)
	{
		return 0.0;
	}
	//计算半角向量
	Vector3f wh = wo + wi;
	if (wh.x == 0 && wh.y == 0 && wh.z == 0)
	{
		return 0.0;
	}
	wh = Normalize(wh);
	//计算菲涅尔反射系数
	Spectrum F = _fresnel->Evaluate(Dot(wi, wh)); //Dot(wo,wh)也可以

	return _R * (F * _distribution->D(wh) * _distribution->G(wo, wi)) / (4 * cosThetaWo * cosThetaWi);
}

Float MicrofacetReflection::Pdf(const Vector3f &wo, const Vector3f &wi) const
{

	if (!SameHemisphere(wo, wi))
	{
		return 0.0;
	}
	//首先获取半角向量
	Vector3f wh = wo + wi;
	wh = Normalize(wh);

	//获取measurement为半角向量的pdf
	Float pdf_wh = _distribution->Pdf(wo, wh);

	//从wh measurement转换到wi measurement
	return pdf_wh / (4 * Dot(wo, wh));
}

Spectrum MicrofacetReflection::Sample_f(const Vector3f &wo, Vector3f *wi,
										const Point2f &sample, Float *pdf,
										BxDFType *sampledType) const
{
	//采样半角向量
	//半角向量和wo是在同一个半球中的
	Vector3f wh = _distribution->Sample_wh(wo, sample);
	//根据根据半角向量获得反射向量
	*wi = Reflect(wo, Normal3f(wh));
	//这里wh和wo之间的夹角可能大于90度，以此wi可能和wo不在一个半球上，以此需要判断
	if (!SameHemisphere(wo, *wi))
	{
		*pdf = 0;
		return 0.0;
	}
	//获得pdf
	*pdf = _distribution->Pdf(wo, wh) / (4 * Dot(wo, wh));

	return f(wo, *wi);
}

Spectrum MicrofacetTransmission::f(const Vector3f &wo, const Vector3f &wi) const
{
	//首先判断wo和wi是否在同一个半球内
	if (SameHemisphere(wo, wi))
	{
		return 0.0;
	}

	Float cosThetaWo = CosTheta(wo);
	Float cosThetaWi = CosTheta(wi);

	if (cosThetaWo == 0 || cosThetaWi == 0)
	{
		return 0.0;
	}

	//判断wo的折射率是A还是B
	Float eta = cosThetaWo > 0 ? (_etaB / _etaA) : (_etaA / _etaB);

	//计算半角向量
	Vector3f wh = Normalize(wo + eta * wi);
	if (wh.z < 0)
	{
		wh = -wh; //半角永远要在微平面的上半球
	}

	//计算Fresnel
	Spectrum F = _fresnel->Evaluate(Dot(wo, wh)); //这里不能使用Dot(wo,wh)

	Float term = Dot(wo, wh) + eta * Dot(wi, wh);
	Float term2 = term * term;

	Float factor = (_mode == TransportMode::Radiance) ? (1 / eta) : 1.0;
	Spectrum brdf = _T * (Spectrum(1.0) - F) * std::abs(eta * eta * _distribution->D(wh) * _distribution->G(wo, wi) * AbsDot(wi, wh) * AbsDot(wo, wh) * factor * factor / (term2 * cosThetaWo * cosThetaWi));
	return brdf;
}

Spectrum MicrofacetTransmission::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
										  Float *pdf, BxDFType *sampledType) const
{

	//判断wo是否和表面平行
	//平行没能量 YOYOYO
	if (wo.z == 0)
	{
		*pdf = 0;
		return 0;
	}

	//采样半角向量
	Vector3f wh = _distribution->Sample_wh(wo, sample);

	//计算eta 这里的eta=iorO/iorI
	Float eta = CosTheta(wh) > 0 ? (_etaA / _etaB) : (_etaB / _etaA);
	//采样折射后的
	if (!Refract(wo, Normal3f(wh), eta, wi))
	{
		*pdf = 0;
		return 0;
	}

	//计算相应的pdf和brdf系数
	*pdf = Pdf(wo, *wi);
	return f(wo, *wi);
}

Float MicrofacetTransmission::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
	////折射情况下,wo和wi不能在同一个半球内
	if (SameHemisphere(wo, wi))
	{
		return 0.0f;
	}

	//获取折射情况下的半角向量
	//etaA是上半球ior etaB是下班球ior
	//eta==iorI/iorO
	Float eta = (wo.z > 0) ? (_etaB / _etaA) : (_etaA / _etaB);
	Vector3f wh = Normalize(wo + eta * wi);

	//获得半角向量空间的pdf
	Float pdf_wh = _distribution->Pdf(wo, wh);

	//计算dwh/dwi 用来从wh空间转换到wi空间
	//这里从论文的公式转换到PBRT中那套公式需要变换下i,o的定位以及wh所在的半球范围
	//ior^2*abs(i_dot_h)/(iorO*o_dot_h+iorI*i_dot_h)^2
	Float term = Dot(wo, wh) + eta * Dot(wi, wh);
	Float dwh_dwi = std::abs((eta * eta * Dot(wi, wh)) / (term * term));

	//返回已经变换到wi空间的pdf
	return pdf_wh * dwh_dwi;
}

Spectrum FresnelBlend::f(const Vector3f &wo, const Vector3f &wi) const
{
	if (!SameHemisphere(wo, wi))
	{
		return 0;
	}
	//求半角向量
	Vector3f wh = wo + wi;
	if (wh.x == 0 && wh.y == 0 && wh.z == 0)
	{
		return 0;
	}
	wh = Normalize(wh);

	Float cosThetaWi = AbsCosTheta(wi);
	Float cosThetaWo = AbsCosTheta(wo);
	Spectrum diffuse(0);
	//两个cos有一个为0，漫反射就没有贡献了
	if (cosThetaWi != 0 && cosThetaWo != 0)
	{
		//首先计算漫反射成分
		auto pow5 = [](Float a) { return (a * a) * (a * a) * a; };
		Float termWo = 1 - pow5(1 - cosThetaWo * 0.5);
		Float termWi = 1 - pow5(1 - cosThetaWi * 0.5);
		diffuse = (28.0 / 23.0) * InvPi * termWo * termWi * _Rd * (Spectrum(1) - _Rs);
	}

	//计算Specular成分
	Spectrum specular(0);
	Float dotHI = Dot(wi, wh);
	specular = _distribution->D(wh) / (4 * dotHI * std::max(cosThetaWi, cosThetaWo)) * SchlickFresnel(_Rs, dotHI);

	return diffuse + specular;
}

//从pbrt复制的modified Bessal Function
inline Float I0(Float x)
{
	Float val = 0;
	Float x2i = 1;
	int64_t ifact = 1;
	int i4 = 1;
	// I0(x) \approx Sum_i x^(2i) / (4^i (i!)^2)
	for (int i = 0; i < 10; ++i)
	{
		if (i > 1)
		{
			ifact *= i;
		}

		val += x2i / (i4 * Sqr(ifact));
		x2i *= x * x;
		i4 *= 4;
	}
	return val;
}
//从pbrt复制的modified Bessal Function
inline Float LogI0(Float x)
{
	if (x > 12)
	{
		return x + 0.5 * (-std::log(2 * Pi) + std::log(1 / x) + 1 / (8 * x));
	}

	else
	{
		return std::log(I0(x));
	}
}

//头发渲染中的Mp成分
//cosThetaO sinThetaO : wo和normal平面夹角的正弦和余弦
//cosThetaI sinThetaI : wi和normal平面夹角的正弦和余弦
//v : roughness variance
//Mp = 1/(2*v*sinH(1/v))*exp(e,-(sinThetaI*sinThetaO)/v)*IO(cosThetaI*cosThetaO/v)
//IO为modified Bessal函数
//[d’Eon 2011]"An energyconserving hair reflectance model."
static Float Mp(Float cosThetaO, Float sinThetaO, Float cosThetaI, Float sinThetaI, Float v)
{
	Float a = sinThetaI * sinThetaO / v;
	Float b = cosThetaI * cosThetaO / v;
	Float mp = 0;
	//小于0.1的时候，用一个更加数值稳定的版本
	if (v <= 0.1f)
	{
		mp = std::exp(LogI0(b) - a - 1 / v + 0.6931f + std::log(1 / (2 * v)));
	}
	else
	{
		mp = std::exp(-a) * I0(b) / (2 * v * std::sinh(1 / v));
	}
	return mp;
}

//用来计算不同的p mode下,假设完美镜面反射和折射，在normal平面上的phi角度需要调整的角度
//net change in azimuthal direction
static inline Float Phi(int p, Float gammaO, Float gammaT)
{
	Assert(p >= 0);
	return 2 * p * gammaT - 2 * gammaO + p * Pi;
}

inline Float TrimmedLogistic(Float x, Float s, Float a, Float b)
{
	return LogisticPdf(x, s) / (LogisticCdf(b, s) - LogisticCdf(a, s));
}

static Float Np(Float phi,int p,Float s,Float gammaO,Float gammaT){
	Float dPhi=phi-Phi(p,gammaO,gammaT);
	//映射到TrimmedLogistic有效的范围内
	if(dPhi>Pi){
		dPhi-=2*Pi;
	}else if(dPhi<-Pi){
		dPhi+=2*Pi;
	}
	return TrimmedLogistic(dPhi,s,-Pi,Pi);
}

static  std::array<Spectrum,pMax+1> Ap(Float cosThetaO,Float eta,Float h,const Spectrum& T){
	//这个三角函数我咋不记得呢
	std::array<Spectrum,pMax+1> ap;

	Float cosGammaO=SafeSqrt(1-Sqr(h));//计算gamma O
	Float cosTheta=cosThetaO*cosGammaO;
	Float f=FrDielectric(cosTheta,1.0f,eta);

	ap[0]=f; 		 //f
	ap[1]=Sqr(1-f)*T;//(1-f)*T*(1-f)
	//(1-f)*T*f*T*...*T*f*T*(1-f)
	for(int p=2;p<pMax;++p){
		ap[p]=ap[p-1]*f*T;
	}

	//运用几何级数来计算剩余的order
	//a=ap[pMax-1]*f*T;
	//r=f*T;
	ap[pMax]=ap[pMax-1]*f*T/(Spectrum(1.0f)-f*T);
	return ap;
}

HairBSDF::HairBSDF(Float h, Float eta, const Spectrum &sigmaA, Float betaM, Float betaN, Float alpha) : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY | BSDF_TRANSMISSION)),
																										_h(h), _eta(eta), _sigmaA(sigmaA), _betaM(betaM), _betaN(betaN), _alpha(alpha)
{
	Assert(_h>=-1&&_h<=1)
	Assert(_betaM>=0&&_betaM<=1);
	Assert(_betaN>=0&&_betaN<=1);
	//计算roughness variance
	//具体细节，参考PBRT的futher reading
	_v[0] = Sqr(0.726f * _betaM + 0.812f * Sqr(_betaM) + 3.7f * Pow<20>(_betaM));
	_v[1] = (Float)0.25f * _v[0];
	_v[2] = 4 * _v[0];
	for (int p = 3; p <= pMax; ++p)
	{
		_v[p] = _v[2];
	}

	//预计算logistic的缩放参数,使得感知上是线性的
	//TODO 出处在哪？
	_s = SqrtPiOver8 * (0.265f * _betaN + 1.194f * Sqr(_betaN) +
						5.372f * Pow<22>(_betaN));

	_gammaO=SafeASin(_h);
}

//Bravais函数，用来计算映射到normal平面后的ior
inline Float Bravais(Float eta,Float sinTheta,Float cosTheta){
	return std::sqrt(eta*eta-Sqr(sinTheta))/cosTheta;
}


Spectrum HairBSDF::f(const Vector3f &wo, const Vector3f &wi) const
{
	//normal平面垂直于x轴，所以theta的对边的长度是x,斜边的长度是1(因为是标准化向量)，所以sinTheta=x/1=x
	Float sinThetaO = wo.x;
	Float cosThetaO = SafeSqrt(1 - Sqr(sinThetaO));
	Float phiO = std::atan2(wo.z, wo.y); //简单的三角函数几何推导
	//同样计算wi相应的参数
	Float sinThetaI = wi.x;
	Float cosThetaI = SafeSqrt(1 - Sqr(sinThetaI));
	Float phiI = std::atan2(wi.z, wi.y);

	//计算纵向平面的折射角度
	//Snell's Law
	Float sinThetaT=sinThetaO/_eta;
	Float cosThetaT=SafeSqrt(1 - Sqr(sinThetaT));

	//首先计算映射到normal平面的ior
	Float etaP=Bravais(_eta,sinThetaO,cosThetaO);
	//然后根据Snell's Law计算新的角度
	Float sinGammaT=_h/etaP;
	Float cosGammaT=SafeSqrt(1-Sqr(sinGammaT));
	Float gammaT=SafeASin(sinGammaT);


	Float phi=phiI-phiO;
	
	//计算transmittance
	auto T=Exp(-(2*cosGammaT/cosThetaT)*_sigmaA);

	auto ap=Ap(cosThetaO,_eta,_h,T);

	Spectrum sum(0);
	//计算每个p的贡献
	for (int i = 0; i < pMax; ++i)
	{
		sum += Mp(cosThetaO, sinThetaO, cosThetaI, sinThetaI, _v[i])*ap[i]*Np(phi,i,_s,_gammaO,gammaT);
	}
	Float absCosThetaI = AbsCosTheta(wi);
	if (absCosThetaI > 0)
	{
		sum = sum / absCosThetaI;
	}

	//添加大于pMax后的路径的贡献
	//这里normal平面的分布使用均匀分布来近似
	sum += Mp(cosThetaO, sinThetaO, cosThetaI, sinThetaI, _v[pMax])*ap[pMax]/(2.0f*Pi);

	Assert(!std::isinf(sum.y()));
	Assert(!std::isnan(sum.y()));
	return sum;
}

Float HairBSDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
	//normal平面垂直于x轴，所以theta的对边的长度是x,斜边的长度是1(因为是标准化向量)，所以sinTheta=x/1=x
	Float sinThetaO = wo.x;
	Float cosThetaO = SafeSqrt(1 - Sqr(sinThetaO));
	Float phiO = std::atan2(wo.z, wo.y); //简单的三角函数几何推导
	//同样计算wi相应的参数
	Float sinThetaI = wi.x;
	Float cosThetaI = SafeSqrt(1 - Sqr(sinThetaI));
	Float phiI = std::atan2(wi.z, wi.y);

	//计算纵向平面的折射角度
	//Snell's Law
	Float sinThetaT=sinThetaO/_eta;
	Float cosThetaT=SafeSqrt(1 - Sqr(sinThetaT));

	//首先计算映射到normal平面的ior
	Float etaP=Bravais(_eta,sinThetaO,cosThetaO);
	//然后根据Snell's Law计算新的角度
	Float sinGammaT=_h/etaP;
	Float cosGammaT=SafeSqrt(1-Sqr(sinGammaT));
	Float gammaT=SafeASin(sinGammaT);


	Float phi=phiI-phiO;

	Float sum(0);
	//计算每个p的贡献
	for (int i = 0; i < pMax; ++i)
	{
		sum += Mp(cosThetaO, sinThetaO, cosThetaI, sinThetaI, _v[i])*Np(phi,i,_s,_gammaO,gammaT);
	}
	Assert(!std::isinf(sum.y()));
	Assert(!std::isnan(sum.y()));
	return sum;
}
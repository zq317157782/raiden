/*
 * reflection.h
 *
 *  Created on: 2016年12月6日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_REFLECTION_H_
#define SRC_CORE_REFLECTION_H_
#include "raiden.h"
#include "geometry.h"
#include "spectrum.h"
//BxDF坐标系下
//法线(0,0,1)与W(x,y,z)点乘等于W.z
inline Float CosTheta(const Vector3f &w) {
	return w.z;
}

inline Float CosTheta2(const Vector3f &w) {
	return w.z * w.z;
}

inline Float AbsCosTheta(const Vector3f &w) {
	return std::abs(w.z);
}

//三角函数公式:sin^2+cos^2=1
inline Float SinTheta2(const Vector3f& w) {
	return std::max(0.0f, 1.0f - CosTheta2(w)); //这里进行max操作防止Float误差导致值小于0
}

inline Float SinTheta(const Vector3f& w) {
	return std::sqrt(SinTheta2(w));
}

inline Float TanTheta(const Vector3f& w) {
	return SinTheta(w) / CosTheta(w);
}

inline Float TanTheta2(const Vector3f& w) {
	return SinTheta2(w) / CosTheta2(w);
}

inline Float CosPhi(const Vector3f& w) {
	Float sint = SinTheta(w);
	if (sint == 0.0f) {
		return 1.0f;
	}
	//sint等于是w在x-y平面上投影的长度，斜边
	//w.x/sint 等于对边比斜边,等于角度的cos值
	//一如既往的加入float误差的保护
	return Clamp(w.x / sint, -1.f, 1.f);
}

inline Float CosPhi2(const Vector3f& w) {
	Float cosphi = CosPhi(w);
	return cosphi * cosphi;
}

inline Float SinPhi(const Vector3f& w) {
	Float sint = SinTheta(w);
	if (sint == 0.0f) {
		return 0.0f;
	}
	//sint等于是w在x-y平面上投影的长度，斜边
	//w.y/sint 等于邻边比斜边,等于角度的sin值
	//一如既往的加入float误差的保护
	return Clamp(w.y / sint, -1.f, 1.f);
}

inline Float SinPhi2(const Vector3f& w) {
	Float sinphi = SinPhi(w);
	return sinphi * sinphi;
}

//计算反射方向
//默认wo和n在同一半球
inline Vector3f Reflect(const Vector3f& wo, const Normal3f& n) {
	return Vector3f(2 * Dot(wo, n) * n) - wo;
}
//计算折射方向
//默认wt和n在同一半球
//eta是材质的折射率
bool Refract(const Vector3f& wi, const Normal3f& n,
		Float oeta/*这里是两个折射率之比(i/t)*/, Vector3f* wt);

//计算绝缘体的菲涅尔反射系数，绝缘体的反射系数可以看成是实数
Float FrDielectric(Float cosThetaI, Float etaI, Float etaT);
//计算导电体的菲涅尔反射系数，金属的折射率是复数，包含吸收率
Spectrum FrConductor(Float cosThetaI, const Spectrum &etaI,
		const Spectrum &etaT, const Spectrum &k);
//计算菲涅尔系数相关的操作
class Fresnel {
public:
	virtual ~Fresnel() {
	}
	//返回反射系数，反射系数是频谱相关的，所以是Spectrum类型
	virtual Spectrum Evaluate(Float cosI) const=0;
};

//关于导电体相关的FRESNEL操作
class FresnelConductor: public Fresnel {
private:
	Spectrum _etaI;	//外部材质折射率
	Spectrum _etaT;	//内部材质折射率
	Spectrum _K;   //吸收率，金属需要考虑的一个因素
public:
	FresnelConductor(const Spectrum& etaI, const Spectrum& etaT,
			const Spectrum& k) :
			_etaI(etaI), _etaT(etaT), _K(k) {
	}
	virtual Spectrum Evaluate(Float cosI) const override {
		return FrConductor(std::abs(cosI), _etaI, _etaT, _K);
	}
};

//关于绝缘体相关的FRESNEL操作
class FresnelDielectric: public Fresnel {
private:
	Float _etaI;
	Float _etaT;
public:
	FresnelDielectric(Float etaI, Float etaT) :
			_etaI(etaI), _etaT(etaT) {
	}
	virtual Spectrum Evaluate(Float cosI) const override {
		return FrDielectric(cosI, _etaI, _etaT);
	}
};

class FresnelNop: public Fresnel {
public:
	virtual Spectrum Evaluate(Float cosI) const override {
		return Spectrum(1.0f);
	}
};
//这里把BxDF抽象成3个类型 Specular/Diffuse/Glossy
//两个行为 Reflection/Transmission
enum BxDFType {
	BSDF_REFLECTION = 1 << 0,  //反射
	BSDF_TRANSMISSION = 1 << 1,  //折射
	BSDF_DIFFUSE = 1 << 2,  //漫反射
	BSDF_GLOSSY = 1 << 3,
	BSDF_SPECULAR = 1 << 4,  //镜面反射
	BSDF_ALL_TYPES = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR,
	BSDF_ALL_REFLECTION = BSDF_REFLECTION | BSDF_ALL_TYPES,
	BSDF_ALL_TRANSMISSION = BSDF_TRANSMISSION | BSDF_ALL_TYPES,
	BSDF_ALL = BSDF_ALL_REFLECTION | BSDF_ALL_TRANSMISSION
};

//判断两个向量是否在同一半球
//BxDF坐标系下
inline bool SameHemisphere(const Vector3f &w, const Vector3f &wp) {
	return w.z * wp.z > 0.f;
}

class BxDF {
public:
	const BxDFType type;
public:
	virtual ~BxDF() {
	}
	BxDF(BxDFType type) :
			type(type) {
	}
	bool MatchesFlags(BxDFType t) const {
		return (type & t) == type;
	}
	virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const = 0; //根据入射光线和出射光线，返回brdf
	//根据样本点和出射光线，采样入射光线以及PDF，出射和入射可以互惠
	//默认满足cosine分布
	virtual Spectrum Sample_f(const Vector3f &wo, Vector3f *wi,
			const Point2f &sample, Float *pdf,
			BxDFType *sampledType = nullptr) const;
	//返回direction-hemisphere反射率
	virtual Spectrum rho(const Vector3f &wo, int nSamples,
			const Point2f *samples) const;
	//返回hemisphere-hemisphere反射率
	virtual Spectrum rho(int nSamples, const Point2f *samples1,
			const Point2f *samples2) const;
	//返回pdf
	//默认返回满足cosine分布PDF
	virtual Float Pdf(const Vector3f &wo, const Vector3f &wi) const;
};

//菲涅尔镜面反射
class SpecularReflection: public BxDF {
private:
	Spectrum _R;	//用来缩放BRDF的一个系数
	Fresnel* _fresnel;	//用来计算菲涅尔反射系数的参数
public:
	SpecularReflection(const Spectrum& R,Fresnel* fresnel):BxDF(BxDFType(BSDF_SPECULAR|BSDF_REFLECTION)),_R(R),_fresnel(fresnel){
	}
	//镜面反射不可能被普通的采样方式采样到
	virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override{
		return 0.0f;
	}
	//镜面反射不可能被普通的采样方式采样到
	virtual Float Pdf(const Vector3f &wo, const Vector3f &wi) const override{
		return 0.0f;
	}

	virtual Spectrum Sample_f(const Vector3f &wo, Vector3f *wi,
				const Point2f &sample, Float *pdf,
				BxDFType *sampledType = nullptr) const override{
		//反射坐标系下
		*wi=Vector3f(-wo.x,-wo.y,wo.z);
		*pdf=1.0f;
		return _fresnel->Evaluate(CosTheta(*wi))*_R/AbsCosTheta(*wi);//除以AbsCosTheta是为了标准化brdf
	}
};

#endif /* SRC_CORE_REFLECTION_H_ */

﻿/*
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
#include "material.h"
#include "interaction.h"
#include "rng.h"
#include "mmath.h"
//BxDF坐标系下
//法线(0,0,1)与W(x,y,z)点乘等于W.z
inline Float CosTheta(const Vector3f &w)
{
	return w.z;
}

inline Float CosTheta2(const Vector3f &w)
{
	return w.z * w.z;
}

inline Float AbsCosTheta(const Vector3f &w)
{
	return std::abs(w.z);
}

//三角函数公式:sin^2+cos^2=1
inline Float SinTheta2(const Vector3f &w)
{
	return std::max((Float)0.0f, (Float)1.0f - CosTheta2(w)); //这里进行max操作防止Float误差导致值小于0
}

inline Float SinTheta(const Vector3f &w)
{
	return std::sqrt(SinTheta2(w));
}

inline Float TanTheta(const Vector3f &w)
{
	return SinTheta(w) / CosTheta(w);
}

inline Float TanTheta2(const Vector3f &w)
{
	return SinTheta2(w) / CosTheta2(w);
}

inline Float CosPhi(const Vector3f &w)
{
	Float sint = SinTheta(w);
	if (sint == 0.0f)
	{
		return 1.0f;
	}
	//sint等于是w在x-y平面上投影的长度，斜边
	//w.x/sint 等于对边比斜边,等于角度的cos值
	//一如既往的加入float误差的保护
	return Clamp(w.x / sint, -1.f, 1.f);
}

inline Float CosPhi2(const Vector3f &w)
{
	Float cosphi = CosPhi(w);
	return cosphi * cosphi;
}

inline Float SinPhi(const Vector3f &w)
{
	Float sint = SinTheta(w);
	if (sint == 0.0f)
	{
		return 0.0f;
	}
	//sint等于是w在x-y平面上投影的长度，斜边
	//w.y/sint 等于邻边比斜边,等于角度的sin值
	//一如既往的加入float误差的保护
	return Clamp(w.y / sint, -1.f, 1.f);
}

inline Float SinPhi2(const Vector3f &w)
{
	Float sinphi = SinPhi(w);
	return sinphi * sinphi;
}

//计算反射方向
//默认wo和n在同一半球
inline Vector3f Reflect(const Vector3f &wo, const Normal3f &n)
{
	return Vector3f(2 * Dot(wo, n) * n) - wo;
}
//计算折射方向
//默认wi和n在同一半球
//eta是材质的折射率
bool Refract(const Vector3f &wi, const Normal3f &n,
			 Float oeta /*这里是两个折射率之比(i/t)*/, Vector3f *wt);

//计算绝缘体的菲涅尔反射系数，绝缘体的反射系数可以看成是实数
Float FrDielectric(Float cosThetaI, Float etaI, Float etaT);
//计算导电体的菲涅尔反射系数，金属的折射率是复数，包含吸收率
Spectrum FrConductor(Float cosThetaI, const Spectrum &etaI,
					 const Spectrum &etaT, const Spectrum &k);

//Schlick Fresnel近似
Spectrum SchlickFresnel(const Spectrum &R, Float cosTheta);

//计算菲涅尔系数相关的操作
class Fresnel
{
  public:
	virtual ~Fresnel()
	{
	}
	//返回反射系数，反射系数是频谱相关的，所以是Spectrum类型
	virtual Spectrum Evaluate(Float cosI) const = 0;
};

class FresnelSchlick : public Fresnel
{
  private:
	Spectrum _F0;

  public:
	FresnelSchlick(const Spectrum &F0) : _F0(F0)
	{
	}

	virtual Spectrum Evaluate(Float cosI) const override
	{
		return SchlickFresnel(_F0, cosI);
	}
};

//关于导电体相关的FRESNEL操作
class FresnelConductor : public Fresnel
{
  private:
	Spectrum _etaI; //外部材质折射率
	Spectrum _etaT; //内部材质折射率
	Spectrum _K;	//吸收率，金属需要考虑的一个因素
  public:
	FresnelConductor(const Spectrum &etaI, const Spectrum &etaT,
					 const Spectrum &k) : _etaI(etaI), _etaT(etaT), _K(k)
	{
	}
	virtual Spectrum Evaluate(Float cosI) const override
	{
		return FrConductor(std::abs(cosI), _etaI, _etaT, _K);
	}
};

//关于绝缘体相关的FRESNEL操作
class FresnelDielectric : public Fresnel
{
  private:
	Float _etaI;
	Float _etaT;

  public:
	FresnelDielectric(Float etaI, Float etaT) : _etaI(etaI), _etaT(etaT)
	{
	}
	virtual Spectrum Evaluate(Float cosI) const override
	{
		return FrDielectric(cosI, _etaI, _etaT);
	}
};

class FresnelNop : public Fresnel
{
  public:
	virtual Spectrum Evaluate(Float cosI) const override
	{
		return Spectrum(1.0f);
	}
};
//这里把BxDF抽象成3个类型 Specular/Diffuse/Glossy
//两个行为 Reflection/Transmission
enum BxDFType
{
	BSDF_REFLECTION = 1 << 0,   //反射
	BSDF_TRANSMISSION = 1 << 1, //折射
	BSDF_DIFFUSE = 1 << 2,		//漫反射
	BSDF_GLOSSY = 1 << 3,
	BSDF_SPECULAR = 1 << 4, //镜面反射
	BSDF_ALL_TYPES = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR,
	BSDF_ALL_REFLECTION = BSDF_REFLECTION | BSDF_ALL_TYPES,
	BSDF_ALL_TRANSMISSION = BSDF_TRANSMISSION | BSDF_ALL_TYPES,
	BSDF_ALL = BSDF_ALL_REFLECTION | BSDF_ALL_TRANSMISSION
};

//判断两个向量是否在同一半球
//BxDF坐标系下
inline bool SameHemisphere(const Vector3f &w, const Vector3f &wp)
{
	return w.z * wp.z > 0.f;
}

class BxDF
{
  public:
	const BxDFType type;

  public:
	virtual ~BxDF()
	{
	}
	BxDF(BxDFType type) : type(type)
	{
	}
	bool MatchesFlags(BxDFType t) const
	{
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
class SpecularReflection : public BxDF
{
  private:
	const Spectrum _R;		 //用来缩放BRDF的一个系数
	const Fresnel *_fresnel; //用来计算菲涅尔反射系数的参数
  public:
	SpecularReflection(const Spectrum &R, Fresnel *fresnel) : BxDF(BxDFType(BSDF_SPECULAR | BSDF_REFLECTION)), _R(R), _fresnel(
																														  fresnel)
	{
	}
	//镜面反射不可能被普通的采样方式采样到
	virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override
	{
		return 0.0f;
	}
	//镜面反射不可能被普通的采样方式采样到
	virtual Float Pdf(const Vector3f &wo, const Vector3f &wi) const override
	{
		return 0.0f;
	}

	virtual Spectrum Sample_f(const Vector3f &wo, Vector3f *wi,
							  const Point2f &sample, Float *pdf,
							  BxDFType *sampledType = nullptr) const override
	{
		//反射坐标系下
		*wi = Vector3f(-wo.x, -wo.y, wo.z);
		*pdf = 1.0f;
		return _fresnel->Evaluate(CosTheta(*wi)) * _R / AbsCosTheta(*wi); //除以AbsCosTheta是为了标准化brdf
	}
};

//菲涅尔散射，不考虑导电体
class SpecularTransmission : public BxDF
{
  private:
	const Spectrum _T;
	const FresnelDielectric _fresnel;
	const Float _etaA, _etaB;
	const TransportMode _mode;

  public:
	SpecularTransmission(const Spectrum &T, Float etaA /*wo所在的eta*/,
						 Float etaB /*wi所在的eta*/, TransportMode mode) : BxDF(BxDFType(BSDF_SPECULAR | BSDF_TRANSMISSION)), _T(T), _fresnel(etaA, etaB), _etaA(etaA), _etaB(etaB), _mode(mode)
	{
	}
	//镜面散射不可能被普通的采样方式采样到
	virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override
	{
		return 0.0f;
	}
	//镜面散射不可能被普通的采样方式采样到
	virtual Float Pdf(const Vector3f &wo, const Vector3f &wi) const override
	{
		return 0.0f;
	}
	virtual Spectrum Sample_f(const Vector3f &wo, Vector3f *wi,
							  const Point2f &sample, Float *pdf,
							  BxDFType *sampledType = nullptr) const override
	{
		bool isEntering = (CosTheta(wo) > 0.0f);
		Float etaI = isEntering ? _etaA : _etaB;
		Float etaT = isEntering ? _etaB : _etaA;
		//反射坐标系下,通过wo计算wi
		if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi))
		{
			//全反射的情况
			return 0.0f;
		}
		*pdf = 1.0f;
		//计算wi所在的空间为正面的情况下,wi方向的反射系数，就可以得到折射到wo方向的系数了
		Spectrum ret = (Spectrum(1.0f) - _fresnel.Evaluate(CosTheta(*wi))) * _T;
		//如果wi方向的能量为来自光源
		if (_mode == TransportMode::Radiance)
		{
			ret *= (etaI * etaI) / (etaT * etaT); //这个系数和折射后的radiacne压缩比有关
		}
		return ret / AbsCosTheta(*wi); //除以AbsCosTheta是为了标准化brdf
	}
};

class FresnelSpecular : public BxDF
{
  private:
	const Spectrum _R;
	const Spectrum _T;
	const Float _etaA, _etaB;
	const FresnelDielectric _fresnel;
	const TransportMode _mode;

  public:
	FresnelSpecular(const Spectrum &R, const Spectrum &T,
					Float etaA /*wo所在的eta*/, Float etaB /*wi所在的eta*/, TransportMode mode) : BxDF(BxDFType(BSDF_SPECULAR | BSDF_REFLECTION | BSDF_TRANSMISSION)), _R(R), _T(T), _etaA(etaA), _etaB(etaB), _fresnel(etaA, etaB), _mode(mode)
	{
	}
	//镜面不可能被普通的采样方式采样到
	virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override
	{
		return 0.0f;
	}
	//镜面不可能被普通的采样方式采样到
	virtual Float Pdf(const Vector3f &wo, const Vector3f &wi) const override
	{
		return 0.0f;
	}

	virtual Spectrum Sample_f(const Vector3f &wo, Vector3f *wi,
							  const Point2f &sample, Float *pdf,
							  BxDFType *sampledType = nullptr) const override
	{
		//计算wo方向的菲涅尔反射系数
		Float fr = FrDielectric(CosTheta(wo), _etaA, _etaB);
		//Spectrum fr = _fresnel.Evaluate(CosTheta(wo));
		//根据样本值，来判断计算的是反射，还是折射
		if (sample.x < fr)
		{
			//反射
			*wi = Vector3f(-wo.x, -wo.y, wo.z);
			*pdf = fr;
			if (sampledType)
			{
				*sampledType = BxDFType(BSDF_SPECULAR | BSDF_REFLECTION);
			}
			return fr * _R / AbsCosTheta(*wi); //除以AbsCosTheta是为了标准化brdf
		}
		else
		{
			//折射
			bool isEntering = (CosTheta(wo) > 0.0f);
			Float etaI = isEntering ? _etaA : _etaB;
			Float etaT = isEntering ? _etaB : _etaA;
			//反射坐标系下,通过wo计算wi
			if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), etaI / etaT,
						 wi))
			{
				//全反射的情况
				return 0.0f;
			}

			//计算wi所在的空间为正面的情况下,wi方向的反射系数，就可以得到折射到wo方向的系数了
			Spectrum ret = (1.0f - fr) * _T;
			//如果wi方向的能量为来自光源
			if (_mode == TransportMode::Radiance)
			{
				ret *= (etaI * etaI) / (etaT * etaT); //这个系数和折射后的radiacne压缩比有关
			}
			if (sampledType)
			{
				*sampledType = BxDFType(BSDF_SPECULAR | BSDF_TRANSMISSION);
			}
			*pdf = 1.0f - fr;
			return ret / AbsCosTheta(*wi); //除以AbsCosTheta是为了标准化brdf
		}
	}
};

//Lambertian完美漫反射
class LambertianReflection : public BxDF
{
  private:
	const Spectrum _R;

  public:
	LambertianReflection(const Spectrum &R) : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), _R(R)
	{
	}

	//Lambertian向各个方向反射的能量相同
	virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override
	{
		return _R * InvPi;
	}

	//返回direction-hemisphere反射率
	virtual Spectrum rho(const Vector3f &wo, int nSamples,
						 const Point2f *samples) const override
	{
		return _R;
	}
	//返回hemisphere-hemisphere反射率
	virtual Spectrum rho(int nSamples, const Point2f *samples1,
						 const Point2f *samples2) const override
	{
		return _R;
	}
};

class LambertianTransmission : public BxDF
{
  private:
	const Spectrum _T;

  public:
	LambertianTransmission(const Spectrum &T) : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_DIFFUSE)), _T(T)
	{
	}

	//Lambertian向各个方向反射的能量相同
	virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override
	{
		return _T * InvPi;
	}

	//返回direction-hemisphere反射率
	virtual Spectrum rho(const Vector3f &wo, int nSamples,
						 const Point2f *samples) const override
	{
		return _T;
	}
	//返回hemisphere-hemisphere反射率
	virtual Spectrum rho(int nSamples, const Point2f *samples1,
						 const Point2f *samples2) const override
	{
		return _T;
	}

	virtual Float Pdf(const Vector3f &wo, const Vector3f &wi) const override
	{
		//不同半球
		if (!SameHemisphere(wo, wi))
		{
			return AbsCosTheta(wi) * InvPi;
		}
		else
		{
			return 0.0f;
		}
	}

	virtual Spectrum Sample_f(const Vector3f &wo, Vector3f *wi,
							  const Point2f &sample, Float *pdf,
							  BxDFType *sampledType = nullptr) const override;
};

//OrenNayar漫反射模型
//考虑了在视角和光线方向相近的情况下的自反射
//输入的sigma角度
class OrenNayar : public BxDF
{
  private:
	const Spectrum _R;
	Float _A, _B;

  public:
	OrenNayar(const Spectrum &R, Float sigma) : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), _R(R)
	{
		//首先把角度转换到弧度
		sigma = Radians(sigma);
		//然后计算A和B
		Float sigma2 = sigma * sigma;
		_A = 1 - (sigma2 / (2 * (sigma2 + 0.33)));
		_B = (0.45 * sigma2) / (sigma2 + 0.09);
	}

	//OrenNayar的BRDF系数
	virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override
	{
		Float sinThetaWo = SinTheta(wo);
		Float sinThetaWi = SinTheta(wi);

		//根据sinTheta的值来判断是否在半球的上半球
		//PBRT这里使用了1e-4作为最小值，我有点理解不能为啥用这个数，因为可以用更小的数
		Float maxCos = 0;
		if (sinThetaWo > 1e-4 && sinThetaWi > 1e-4)
		{
			//使用三角函数变换式 cos(a-b)=cos(a)*cos(b)+sin(a)*sin(b)
			Float sinPhiWi = SinPhi(wi), cosPhiWi = CosPhi(wi);
			Float sinPhiWo = SinPhi(wo), cosPhiWo = CosPhi(wo);
			maxCos = std::max((Float)(0.0), (cosPhiWi * cosPhiWo + sinPhiWi * sinPhiWo));
		}

		Float sinAlpha = 0;
		Float tanBeta = 0;
		//使用cos值来比较大小
		//cos值大的角度小
		if (AbsCosTheta(wi) > AbsCosTheta(wo))
		{
			sinAlpha = sinThetaWo;
			tanBeta = sinThetaWi / AbsCosTheta(wi);
		}
		else
		{
			sinAlpha = sinThetaWi;
			tanBeta = sinThetaWo / AbsCosTheta(wo);
		}

		//整合所有成分
		return _R * InvPi * (_A + _B * maxCos * sinAlpha * tanBeta);
	}
};

//微平面反射模型
class MicrofacetReflection : public BxDF
{
  private:
	const Spectrum _R;
	const MicrofacetDistribution *_distribution;
	const Fresnel *_fresnel;

  public:
	MicrofacetReflection(const Spectrum &R, MicrofacetDistribution *distribution, Fresnel *fresnel) : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)), _R(R), _distribution(distribution), _fresnel(fresnel)
	{
	}

	virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override;

	virtual Float Pdf(const Vector3f &wo, const Vector3f &wi) const override;

	virtual Spectrum Sample_f(const Vector3f &wo, Vector3f *wi,
							  const Point2f &sample, Float *pdf,
							  BxDFType *sampledType = nullptr) const override;
};

//微平面透视模型
class MicrofacetTransmission : public BxDF
{
  private:
	const Spectrum _T;
	const MicrofacetDistribution *_distribution;
	const FresnelDielectric *_fresnel; //只考虑绝缘体，电解质不考虑透射
	const Float _etaA, _etaB;		   //折射率 A为上层材质折射率 B为下层材质折射率
	const TransportMode _mode;

  public:
	MicrofacetTransmission(const Spectrum &T, const MicrofacetDistribution *distribution, const FresnelDielectric *fresnel, Float etaA, Float etaB, const TransportMode mode) : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_GLOSSY)), _T(T), _distribution(distribution), _fresnel(fresnel), _etaA(etaA), _etaB(etaB), _mode(mode)
	{
	}
	virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override;
	//透射需要修改采样方式，需要wo和wi不在同一个半球内
	virtual Spectrum Sample_f(const Vector3f &wo, Vector3f *wi,
							  const Point2f &sample, Float *pdf,
							  BxDFType *sampledType = nullptr) const override;
	//PDF 需要wo和wi不在同一个半球内
	virtual Float Pdf(const Vector3f &wo, const Vector3f &wi) const override;
};

//Ashikhmin-Shirley model
//考虑了diffuse和fresnel成分，并且保证了能量守恒
class FresnelBlend : public BxDF
{
  private:
	const Spectrum _Rd;							 //albedo
	const Spectrum _Rs;							 //F0
	const MicrofacetDistribution *_distribution; //Specular分布
  public:
	FresnelBlend(const Spectrum &Rd, const Spectrum &Rs, const MicrofacetDistribution *distribution) : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)), _Rd(Rd), _Rs(Rs), _distribution(distribution)
	{
	}
	virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override;
};

//用来缩放一个BxDF贡献，比如通过weight来控制diffuse和specular的相应成分
class ScaledBxDF : public BxDF
{
  private:
	//这里使用裸指针,但是不用担心内存泄漏，因为BxDF都需要通过MemoryAren来分配
	BxDF *_bxdf;
	Spectrum _scale;

  public:
	ScaledBxDF(BxDF *bxdf, const Spectrum &scale) : BxDF(BxDFType(bxdf->type)), _bxdf(bxdf), _scale(scale)
	{
	}

	virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override
	{
		return _scale * _bxdf->f(wo, wi);
	}
	virtual Spectrum Sample_f(const Vector3f &wo, Vector3f *wi,
							  const Point2f &sample, Float *pdf,
							  BxDFType *sampledType = nullptr) const override
	{
		return _scale * _bxdf->Sample_f(wo, wi, sample, pdf, sampledType);
	}

	virtual Spectrum rho(const Vector3f &wo, int nSamples,
						 const Point2f *samples) const override
	{
		return _scale * _bxdf->rho(wo, nSamples, samples);
	}
	virtual Spectrum rho(int nSamples, const Point2f *samples1,
						 const Point2f *samples2) const override
	{
		return _scale * _bxdf->rho(nSamples, samples1, samples2);
	}
	virtual Float Pdf(const Vector3f &wo, const Vector3f &wi) const override
	{
		return _bxdf->Pdf(wo, wi);
	}
};

//双向散射分布函数
class BSDF
{
  private:
	static constexpr int MaxBxDFs = 8; //默认包含8个BxDF组件
	BxDF *_bxdfs[MaxBxDFs];
	int _nBxDF = 0;		//当前BxDF个数
	const Normal3f _ns; //shading normal;
	const Normal3f _ng; //geometry normal
	const Vector3f _ss; //切线
	const Vector3f _ts; //次切线
	~BSDF()
	{
	} //把析构函数设置成私有防止被不慎析构掉
  public:
	const Float eta; //折射率之比
	BSDF(const SurfaceInteraction &si, Float eta = 1.0f) : eta(eta), _ns(si.shading.n), _ng(si.n), _ss(Normalize(si.dpdu)), _ts(
																																Cross(_ns, _ss))
	{
	}
	//增加一个BxDF组件
	void Add(BxDF *bxdf)
	{
		Assert(_nBxDF < MaxBxDFs);
		_bxdfs[_nBxDF++] = bxdf;
	}
	//返回BSDF包含特定的BxDF个数
	int NumComponents(BxDFType flags = BSDF_ALL) const
	{
		int num = 0;
		for (int i = 0; i < _nBxDF; ++i)
		{
			if (_bxdfs[i]->MatchesFlags(flags))
			{
				++num;
			}
		}
		return num;
	}
	//世界到BSDF的局部坐标
	//因为是正交矩阵，所以逆转置就是自己，所以不需要对法线做特殊处理
	Vector3f WorldToLocal(const Vector3f &v) const
	{
		return Vector3f(Dot(v, _ss), Dot(v, _ts), Dot(v, _ns));
	}
	//BSDF的局部坐标到世界坐标
	//正交矩阵，转置既是逆
	Vector3f LocalToWorld(const Vector3f &v) const
	{
		return Vector3f(_ss.x * v.x + _ts.x * v.y + _ns.x * v.z,
						_ss.y * v.x + _ts.y * v.y + _ns.y * v.z,
						_ss.z * v.x + _ts.z * v.y + _ns.z * v.z);
	}

	//采样brdf系数
	Spectrum f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags = BSDF_ALL) const
	{
		//获得局部坐标下的射线
		Vector3f wo = WorldToLocal(woW);
		Vector3f wi = WorldToLocal(wiW);
		//防止后续操作出现nan
		if (wo.z == 0)
		{
			return 0.0f;
		}
		bool reflect = Dot(wiW, _ng) * Dot(woW, _ng) > 0.0f; //判断是反射还是折射
		Spectrum result(0.0f);
		for (int i = 0; i < _nBxDF; ++i)
		{
			if (_bxdfs[i]->MatchesFlags(flags))
			{
				if ((reflect && (_bxdfs[i]->type & BSDF_REFLECTION)) || (!reflect && (_bxdfs[i]->type & BSDF_TRANSMISSION)))
				{
					result += _bxdfs[i]->f(wo, wi);
				}
			}
		}
		return result;
	}
	//H-H
	Spectrum rho(int nSamples, const Point2f *samples1, const Point2f *samples2,
				 BxDFType flags = BSDF_ALL) const
	{
		Spectrum result(0.0f);
		for (int i = 0; i < _nBxDF; ++i)
		{
			if (_bxdfs[i]->MatchesFlags(flags))
			{
				result += _bxdfs[i]->rho(nSamples, samples1, samples2);
			}
		}
		return result;
	}

	//D-H wo是local?
	Spectrum rho(const Vector3f &wo, int nSamples, const Point2f *samples,
				 BxDFType flags = BSDF_ALL) const
	{
		Spectrum result(0.0f);
		for (int i = 0; i < _nBxDF; ++i)
		{
			if (_bxdfs[i]->MatchesFlags(flags))
			{
				result += _bxdfs[i]->rho(wo, nSamples, samples);
			}
		}
		return result;
	}

	Spectrum Sample_f(const Vector3f &woWorld /*世界坐标系下面的出射射线*/, Vector3f *wiWorld /*世界坐标系下面的入射射线*/, const Point2f &u /*样本*/, Float *pdf, BxDFType type /*希望匹配的BXDF类型*/, BxDFType *sampledType = nullptr /*实际匹配的BXDF类型*/) const
	{
		//1.先采样一个BxDF，确定BxDF的类型，以及采样入射方向
		//2.根据入射和出射方向，采样pdf
		//3.计算BSDF系数
		int matchingCompNum = NumComponents(type);

		//没有匹配的BxDF就直接返回0
		if (matchingCompNum == 0)
		{
			*pdf = 0;
			if (sampledType)
			{
				*sampledType = BxDFType(0); //未知类型
			}

			return 0;
		}
		//根据0~1之间的样本值，采样出1个组件
		int comp = std::min((int)std::floor(matchingCompNum * u[0]), matchingCompNum - 1);
		//开始采样1个bxdf
		BxDF *bxdf = nullptr;
		int count = comp;
		for (int i = 0; i < _nBxDF; ++i)
		{
			if (_bxdfs[i]->MatchesFlags(type) && (count-- == 0) /*这里先比较，再--,不能加括号*/)
			{
				bxdf = _bxdfs[i];
				break;
			}
		}
		Assert(bxdf != nullptr);
		//重新映射样本
		Point2f uNew = Point2f(std::min(matchingCompNum * u[0] - comp, OneMinusEpsilon), u[1]);
		//反射坐标下下的wo,wi
		Vector3f wo = WorldToLocal(woWorld);
		//光线压根没和表面相交
		if (wo.z == 0)
		{
			return 0;
		}
		Vector3f wi;
		//初始化部分数据
		*pdf = 0;
		if (sampledType)
		{
			*sampledType = bxdf->type;
		}
		//计算bxdf系数以及采样出射方向
		Spectrum f = bxdf->Sample_f(wo, &wi, uNew, pdf, sampledType);

		if (*pdf == 0)
		{
			if (sampledType)
			{
				*sampledType = BxDFType(0); //未知类型
			}
			return 0;
		}

		//计算世界坐标系下的出射方向
		*wiWorld = LocalToWorld(wi);

		if (!(bxdf->type & BSDF_SPECULAR) && matchingCompNum > 1)
		{
			//计算每个符合的bxdf的pdf之和，然后计算平均pdf
			for (int i = 0; i < _nBxDF; ++i)
			{
				if (_bxdfs[i]->MatchesFlags(type) && _bxdfs[i] != bxdf)
				{
					*pdf += _bxdfs[i]->Pdf(wo, wi);
				}
			}
		}
		if (matchingCompNum > 1)
		{
			*pdf = (*pdf) / matchingCompNum;
		}
		if (!(bxdf->type & BSDF_SPECULAR) && matchingCompNum > 1)
		{
			//计算bxdf系数和
			//bool reflect = (wi.z*wo.z) > 0;
			//PBRT中其实是这样写的
			bool reflect = Dot(*wiWorld, _ng) * Dot(woWorld, _ng) > 0;
			f = Spectrum(0); //重置
			for (int i = 0; i < _nBxDF; ++i)
			{
				if (_bxdfs[i]->MatchesFlags(type) && ((reflect && _bxdfs[i]->type & BSDF_REFLECTION) || (!reflect && _bxdfs[i]->type & BSDF_TRANSMISSION)))
				{
					f += _bxdfs[i]->f(wo, wi);
				}
			}
		}
		return f;
	}

	//计算PDF
	Float Pdf(const Vector3f &woWorld, const Vector3f &wiWorld, BxDFType type = BSDF_ALL) const
	{
		if (_nBxDF == 0)
		{
			return 0.0f;
		}
		//计算局部wo,wi
		Vector3f wo = WorldToLocal(woWorld);
		Vector3f wi = WorldToLocal(wiWorld);
		//和表面没有相交点
		if (wo.z == 0)
		{
			return 0.0f;
		}
		Float pdf = 0;
		int matchingCompNum = 0;
		for (int i = 0; i < _nBxDF; ++i)
		{
			if (_bxdfs[i]->MatchesFlags(type))
			{
				++matchingCompNum;
				pdf += _bxdfs[i]->Pdf(wo, wi);
			}
		}
		if (matchingCompNum > 0)
		{
			return pdf / matchingCompNum;
		}
		return 0.0f;
	}
};

static const Float SqrtPiOver8 = 0.626657069f;
static const int pMax=3;//独立计算的p的最大值，超过这个p的值全部用级数一起计算

//头发的BSDF的实现
//f(wo,wi)=Sum f_p(wo,wi)
//f_p(wo,wi)=M_p*A_p*Np/Abs(cosTheta_i)
class HairBSDF : public BxDF
{
  private:
	Float _h;//v:[0,1]=>h:[-1,1]
	Float _eta;//头发的ior
	Spectrum _sigmaA;//头发的吸收系数(absorb cross section)
	Float _betaM,_betaN;//相应的纵向？横向？成分的粗糙度
	Float _alpha;//头发表面的scale的角度(通常的均值为2)
	//以下是预计算的数据
	Float _v[pMax+1];//每个p对应的roughness variance
	Float _sin2kAlpha[3];
	Float _cos2kAlpha[3];

	Float _s;
	Float _gammaO;//等于asin(h)

	//计算Ap的分布
	std::array<Float,pMax+1> ComputeApPdf(Float cosThetaO) const;

  public:
	HairBSDF(Float h,Float eta,const Spectrum& sigmaA,Float betaM,Float betaN,Float alpha);
	virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override;
	virtual Spectrum Sample_f(const Vector3f &wo, Vector3f *wi,
							  const Point2f &sample, Float *pdf,
							  BxDFType *sampledType = nullptr) const override;
	// virtual Spectrum rho(const Vector3f &wo, int nSamples,
	// 					 const Point2f *samples) const override
	// {
	// 	Assert(false);
	// 	return 0;
	// }
	// virtual Spectrum rho(int nSamples, const Point2f *samples1,
	// 					 const Point2f *samples2) const override
	// {
	// 	Assert(false);
	// 	return 0;
	// }
	virtual Float Pdf(const Vector3f &wo, const Vector3f &wi) const override;
};

#endif /* SRC_CORE_REFLECTION_H_ */

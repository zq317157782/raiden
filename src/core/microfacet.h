/*
 * microfacet.h
 *
 *  Created on: 2016年12月27日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_MICROFACET_H_
#define SRC_CORE_MICROFACET_H_
#include "raiden.h"
#include "geometry.h"
class MicrofacetDistribution {
protected:
	MicrofacetDistribution()  {
	}
	//Lambda是 不可见微平面面积 与 可见微平面面积 的比值
	//或者是 可见微平面面积 与 总微平面面积 的比值
	virtual Float Lambda(const Vector3f &w) const = 0;

public:
	virtual ~MicrofacetDistribution(){};

	//NDF
	//传入半角向量，返回相应方向的微平面总面积,或者说是法线分布
	virtual Float D(const Vector3f &wh) const = 0;
	
	
	
	//Masking-Function
	//返回有多少比例的法线为wh的微平面可以被w方向看见，微平面的可见性假设和微平面的wh无关
	Float G1(const Vector3f &w) const {
		//    if (Dot(w, wh) * CosTheta(w) < 0.0) return 0.0;
		return 1.0 / (1.0 + Lambda(w));
	}
	
	//Masking-Shadowing Function
	//返回有多少比例的微平面可以同时被wo和wi方向同时看到
	//这里使用的是考虑wo和wi的之间关联性的方法
	Float G(const Vector3f &wo, const Vector3f &wi) const {
		return 1.0 / (1.0 + Lambda(wo) + Lambda(wi));
	}
	
	//根据样本点 采样半角向量
	virtual Vector3f Sample_wh(const Vector3f &wo, const Point2f &u) const = 0;
	Float Pdf(const Vector3f &wo, const Vector3f &wh) const;
	//virtual std::string ToString() const = 0;
};


//从粗糙度到alpha参数的转换的工具函数
Float GGXRoughnessToAlpha(Float roughness);

//Isotropy Beckmann
class IsotropyBeckmannDistribution:public MicrofacetDistribution{
	private:
		Float _alpha;
	public:
		IsotropyBeckmannDistribution(Float alpha):_alpha(alpha){}
		Float D(const Vector3f &wh) const override;

		Float Lambda(const Vector3f &w) const override;
		//采样GGX的半角向量
		Vector3f Sample_wh(const Vector3f &wo, const Point2f &u) const override;
};

//GGX的各项异性版本
class AnisotropyGGXDistribution:public MicrofacetDistribution{
private:
	Float _alphaX,_alphaY;
	Float Lambda(const Vector3f &w) const override;
public:
	AnisotropyGGXDistribution(Float ax,Float ay):_alphaX(ax),_alphaY(ay){}
	Float D(const Vector3f &wh) const override;
	
	
	//采样GGX的半角向量
	Vector3f Sample_wh(const Vector3f &wo, const Point2f &u) const override{
		LError << "AnisotropyGGXDistribution::Sample_wh is not implemented!!!";
		exit(-1);
		return Vector3f(0,0,0);
	}

	// Float Pdf(const Vector3f &wo, const Vector3f &wh) const override {
	// 	LError << "AnisotropyGGXDistribution::Pdf is not implemented!!!";
	// 	exit(-1);
	// 	return 0;
	// }
};


//GGX各向同性版本
class IsotropyGGXDistribution:public MicrofacetDistribution{
private:
	Float _alpha;
	Float Lambda(const Vector3f &w) const override;
public:
	IsotropyGGXDistribution(Float a) :_alpha(a) {}
	Float D(const Vector3f &wh) const override;
	//采样GGX的半角向量
	Vector3f Sample_wh(const Vector3f &wo, const Point2f &u) const override;
	//Float Pdf(const Vector3f &wo, const Vector3f &wh) const override;
};


#endif /* SRC_CORE_MICROFACET_H_ */

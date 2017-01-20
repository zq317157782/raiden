/*
* medium.h
*
*  Created on: 2017年1月19日
*      Author: zhuqian
*/


#ifndef SRC_CORE_MEDIUM_H_
#define SRC_CORE_MEDIUM_H_
#include "raiden.h"
#include "geometry.h"

//体渲染使用的类似于BSDF的函数
class PhaseFunction {
public:
	virtual Float P(const Vector3f& wo,const Vector3f& wi)const = 0;
	virtual ~PhaseFunction() {}
	//采样PhaseFunction
	virtual Float Sample_p(const Vector3f &wo, Vector3f *wi,
		const Point2f &u) const = 0;

	virtual std::string ToString() const = 0;

};



inline std::ostream &operator<<(std::ostream &os, const PhaseFunction &p) {
	os << p.ToString();
	return os;
}

//HenyeyGreenstein
inline Float PhaseHG(Float cosTheta, Float g) {
	Float denom = 1 + g * g + 2 * g * cosTheta;
	return Inv4Pi * (1 - g * g) / (denom * std::sqrt(denom));
}


//HenyeyGreenstein PhaseFunction
class HenyeyGreenstein :public PhaseFunction {
private:
	Float _g;//控制参数,范围-1~1,正数和负数代表了反射回去还是向前反射
public:
	HenyeyGreenstein(Float g) :_g(g) {}

	Float P(const Vector3f& wo, const Vector3f& wi) const override {
		return PhaseHG(Dot(wo, wi), _g);
	}

	Float Sample_p(const Vector3f &wo, Vector3f *wi,
		const Point2f &u) const override {
		//采样costheta
		Float cosTheta;
		if (std::abs(_g) < 1e-3) {
			//_g为0的时候用均匀采样
			cosTheta = 1 - 2 * u[0];
		}
		else {
			//取CDF的逆函数
			Float sqrTerm = (1 - _g * _g) / (1 - _g + 2 * _g * u[0]);
			cosTheta = (1 + _g * _g - sqrTerm * sqrTerm) / (2 * _g);
		}
		//采样入射方向
		Float sinTheta = std::sqrt(std::max((Float)0, 1 - cosTheta * cosTheta));
		Float phi = 2 * Pi * u[1];
		Vector3f v1, v2;
		CoordinateSystem(wo, &v1, &v2);
		//-wo取0是因为PBRT的wi方向和学术界论文中常用的方向正好相反
		*wi = SphericalDirection(sinTheta, cosTheta, phi, v1, v2, -wo);
		//-cosTheta理由同上
		return PhaseHG(-cosTheta, _g);
	}
};

//材质介质
class Medium {
public:
	//返回射线在这个Medium中传播到ray.tMax后得到的 beam transmittance.
	//通俗点讲就是有多少沿着射线的能量还被保留着，或者说传播着
	virtual Spectrum Tr(const Ray &ray, Sampler &sampler) const = 0;
};

//两个介质的相交面
struct MediumInterface {
	//默认是真空介质
	MediumInterface() : inside(nullptr), outside(nullptr) {}
	//两边相同介质
	MediumInterface(const Medium *medium) : inside(medium), outside(medium) {}

	MediumInterface(const Medium *inside, const Medium *outside)
		: inside(inside), outside(outside) {}
	//判断是否有介质转移
	bool IsMediumTransition() const { return inside != outside; }
	const Medium *inside, *outside;
};

#endif /* SRC_CORE_MEDIUM_H_ */

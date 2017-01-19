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

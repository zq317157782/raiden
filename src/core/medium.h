﻿/*
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



#endif /* SRC_CORE_MEDIUM_H_ */

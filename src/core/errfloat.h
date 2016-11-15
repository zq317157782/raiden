/*
 * errfloat.h
 *
 *  Created on: 2016年11月15日
 *      Author: zhuqian
 *
 * 这个文件维护一个ErrFloat类，是PBRTV3中的EFloat类的功能，这里把名字改成ErrFloat让类名更加明确意义
 * 然后PBRTV3的书籍上的实现与其代码中的实现也有一定的差异
 */

#ifndef SRC_CORE_ERRFLOAT_H_
#define SRC_CORE_ERRFLOAT_H_

#include "raiden.h"

class ErrFloat{
private:
	float _value;//浮点数round后的值
	//float _error;//浮点数的绝对误差(real值减去round值)
	float _high;//round值加上绝对error后的值
	float _low;//round值减去绝对error后的值
#ifdef DEBUG_BUILD
	double _highPrecisionValue;//调试模式下需要计算的高精度的值
#endif
public:
	ErrFloat(){}
	ErrFloat(float,float err=0.0f);
	ErrFloat operator+(ErrFloat) const;

	//强制转换成float
	explicit operator float() const{
		return _value;
	}

	explicit operator double() const{
		return _value;
	}

	float UpperBound() const{return _high;}
	float LowerBound() const{return _low;}
	//Check函数是内敛函数，然后内部的实现只在DEBUG_BUILD模式有用
	//所以编译后如果不是DB模式，不会产生额外的代码
	inline void Check() const{
#ifdef DEBUG_BUILD
		//todo 对数据的检查
#endif
	}
};




#endif /* SRC_CORE_ERRFLOAT_H_ */

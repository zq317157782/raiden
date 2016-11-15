/*
 * errfloat.cpp
 *
 *  Created on: 2016年11月15日
 *      Author: zhuqian
 */

#include "errfloat.h"

ErrFloat::ErrFloat(float v, float err) :
		_value(v) {
	Assert(!std::isnan(v));
	Assert(!std::isnan(err));
	if (err == 0.0f) {
		_low=_high=v;
	}else{
		//这里调用NextFloatUp和NextFloatDown是为了做保守的边界
		_low=NextFloatDown(v-err);
		_high=NextFloatUp(v+err);
	}
#ifdef DEBUG_BUILD
	_highPrecisionValue=_value;
	Check();
#endif
}

ErrFloat ErrFloat::operator+(ErrFloat f) const {
	ErrFloat result;
	result._value=_value+f._value;
#ifdef DEBUG_BUILD
	result._highPrecisionValue=_highPrecisionValue+f._highPrecisionValue;
#endif
	//这里调用NextFloatUp和NextFloatDown是为了做保守的边界
	result._low=NextFloatDown(LowerBound()+f.LowerBound());
	result._high=NextFloatUp(UpperBound()+f.UpperBound());
	Check();
	return result;
}


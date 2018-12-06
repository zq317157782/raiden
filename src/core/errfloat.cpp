/*
 * errfloat.cpp
 *
 *  Created on: 2016年11月15日
 *      Author: zhuqian
 */

#include "errfloat.h"

EFloat::EFloat(float v, float err) :
		_value(v) {
	Assert(!std::isnan(v));
	Assert(!std::isnan(err));
	if (err == 0.0f) {
		_low = _high = v;
	} else {
		//这里调用NextFloatUp和NextFloatDown是为了做保守的边界
		_low = NextFloatDown(v - err);
		_high = NextFloatUp(v + err);
	}
#ifdef DEBUG
	_highPrecisionValue = _value;
	Check();
#endif
}

EFloat EFloat::operator+(EFloat f) const {
	EFloat result;
	result._value = _value + f._value;
#ifdef DEBUG
	result._highPrecisionValue = _highPrecisionValue + f._highPrecisionValue;
#endif
	//这里调用NextFloatUp和NextFloatDown是为了做保守的边界
	result._low = NextFloatDown(LowerBound() + f.LowerBound());
	result._high = NextFloatUp(UpperBound() + f.UpperBound());
	result.Check();
	return result;
}

EFloat EFloat::operator-(EFloat f) const {
	EFloat result;
	result._value = _value - f._value;
#ifdef DEBUG
	result._highPrecisionValue = _highPrecisionValue - f._highPrecisionValue;
#endif
	//这里调用NextFloatUp和NextFloatDown是为了做保守的边界
	result._low = NextFloatDown(LowerBound() - f.UpperBound());
	result._high = NextFloatUp(UpperBound() - f.LowerBound());
	result.Check();
	return result;
}

EFloat EFloat::operator*(EFloat f) const {
	EFloat result;
	result._value = _value * f._value;
#ifdef DEBUG
	result._highPrecisionValue = _highPrecisionValue * f._highPrecisionValue;
#endif
	Float all[4] = { LowerBound() * f.LowerBound(), LowerBound()
			* f.UpperBound(), UpperBound() * f.LowerBound(), UpperBound()
			* f.UpperBound() };
	result._low = NextFloatDown(
			std::min(std::min(all[0], all[1]), std::min(all[2], all[3])));
	result._high = NextFloatUp(
			std::max(std::max(all[0], all[1]), std::max(all[2], all[3])));
	result.Check();
	return result;
}

EFloat EFloat::operator/(EFloat f) const {
	EFloat result;
	result._value = _value / f._value;
#ifdef DEBUG
	result._highPrecisionValue = _highPrecisionValue / f._highPrecisionValue;
#endif

	if (f._low < 0.0f && f._high > 0.0f) {
		//除以0的情况
		result._low = -Infinity;
		result._high = Infinity;
	} else {
		Float all[4] = { LowerBound() / f.LowerBound(), LowerBound()
				/ f.UpperBound(), UpperBound() / f.LowerBound(), UpperBound()
				/ f.UpperBound() };
		result._low = NextFloatDown(
				std::min(std::min(all[0], all[1]), std::min(all[2], all[3])));
		result._high = NextFloatUp(
				std::max(std::max(all[0], all[1]), std::max(all[2], all[3])));
	}
	result.Check();
	return result;
}

EFloat EFloat::operator-() const{
	EFloat result;
	result._value=-_value;
#ifdef DEBUG
	result._highPrecisionValue = -_highPrecisionValue;
#endif
	result._low=-_high;
	result._high=-_low;
	result.Check();
	return result;
}

bool EFloat::operator==(EFloat f) const{
	return _value==f._value;
}

EFloat::EFloat(const EFloat&f){
	_value=f._value;
#ifdef DEBUG
	_highPrecisionValue =f._highPrecisionValue;
#endif
	_low=f._low;
	_high=f._high;
	Check();
}
EFloat& EFloat::operator=(const EFloat&f){
	_value=f._value;
	#ifdef DEBUG
		_highPrecisionValue =f._highPrecisionValue;
	#endif
		_low=f._low;
		_high=f._high;
		Check();
		return *this;
}

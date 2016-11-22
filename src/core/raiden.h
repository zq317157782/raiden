/*
 * raiden.h
 *
 *  Created on: 2016年11月9日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_RAIDEN_H_
#define SRC_CORE_RAIDEN_H_
//raiden.h包含所有全局设置
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <assert.h>

//定义一个Float宏 可能指向float可能指向double
typedef float Float;
//无限大数
static constexpr Float Infinity=std::numeric_limits<Float>::infinity();

//根据编译选项设置断言宏
#ifdef DEBUG_BUILD
#define Assert(x) assert(x)
#else
#define Assert(x) ((void)0)
#endif

//打印错误的宏定义，我把它定义在这，PBRT的实现我没有深究，对我来说，这个目前已经做够了
#define Error(x) std::cerr<<x<<std::endl;

static constexpr Float Pi = 3.14159265358979323846;

template<typename T> class Vector3;
template<typename T> class Vector2;
template<typename T> class Point3;
template<typename T> class Point2;
template<typename T> class Normal3;
template<typename T> class Bound3;
class Ray;
class RayDifferential;
struct Matrix4x4;
class Transform;
class ErrFloat;//计算累积误差的浮点数实现
class Medium;
struct MediumInterface;

//float类型相应的IEEE标准的BIT格式
//最高位是符号位，然后8位是指数,接下来23位是值，
//指数为0的时候没有默认最高位的1
//指数为255的时候，值为0则为无穷大，值不为0则为NaN
inline uint32_t FloatToBits(float f) {
	Assert(!std::isnan(f));
	uint32_t bits = 0;
	std::memcpy(&bits, &f, sizeof(float));
	return bits;
}
//从BIT形式转换会float类型
inline float BitsToFloat(uint32_t bits) {
	float f = 0;
	std::memcpy(&f, &bits, sizeof(uint32_t));
	return f;
}

//double版本的 FloatToBits
inline uint64_t FloatToBits(double d) {
	Assert(!std::isnan(d));
	uint64_t bits = 0;
	std::memcpy(&bits, &d, sizeof(double));
	return d;
}

inline double BitsToFloat(uint64_t bits) {
	double d = 0;
	std::memcpy(&d, &bits, sizeof(uint64_t));
	return d;
}

//获取下一个大于本float变量的float变量
//1.先判断是否是无限值，是的话直接返回
//2.如果是负0的话，先转换成正0，因为下面的比较需要0是一个正0
//3.转换成BIT形式，并且比较BIT是否大于0，大于++，小于--
//4.再度转换回float，并且返回
inline float NextFloatUp(float f) {
	Assert(!std::isnan(f));
	if (std::isinf(f) && f > 0.0f)
		return f;
	if (f == -0.0f)
		f = 0.0f;
	uint32_t bits = FloatToBits(f);
	if (bits >= 0)
		++bits;
	else
		--bits;
	return BitsToFloat(bits);
}

//获取下一个小于本float变量的float变量
inline float NextFloatDown(float f) {
	Assert(!std::isnan(f));
	if (std::isinf(f) && f < 0.0f)
		return f;
	if (f == -0.0f)
		f = 0.0f;
	uint32_t bits = FloatToBits(f);
	if (bits >= 0)
		--bits;
	else
		++bits;
	return BitsToFloat(bits);
}

inline double NextFloatUp(double d, int delta = 1) {
	Assert(!std::isnan(d));
	if (std::isinf(d) && d > 0.0)
		return d;
	if (d == -0.0)
		d = 0.0;
	uint64_t bits = FloatToBits(d);
	if (bits >= 0)
		bits += delta;
	else
		bits -= delta;
	return BitsToFloat(bits);
}

inline double NextFloatDown(double d, int delta = 1) {
	Assert(!std::isnan(d));
	if (std::isinf(d) && d < 0.0)
		return d;
	if (d == -0.0)
		d = 0.0;
	uint64_t bits = FloatToBits(d);
	if (bits >= 0)
		bits -= delta;
	else
		bits += delta;
	return BitsToFloat(bits);
}

//这个MachineEpsion是数值分析下的MachineEpsion；为2的-24次方；
//C++标准库提供的Epsion是大于1的ULP，为2的-23次方
//所以需要再除以2
static constexpr Float MachineEpsion = 0.5f
		* std::numeric_limits<Float>::epsilon();

//这个gamma不是用来做Gamma校正的gamma，这个gamma是浮点数运算中，每一次运算后的最大ERR边界
inline constexpr Float gamma(int n){return (n*MachineEpsion)/(1-n*MachineEpsion);}



//裁剪函数
inline Float Clamp(Float val, Float low, Float high) {
    if (val < low) return low;
    else if (val > high) return high;
    else return val;
}

//角度转换弧度
inline Float Radians(Float deg) { return (Pi / 180) * deg; }

#endif /* SRC_CORE_RAIDEN_H_ */

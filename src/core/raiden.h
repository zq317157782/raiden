
/*
 * raiden.h
 *
 *  Created on: 2016年11月9日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_RAIDEN_H_
#define SRC_CORE_RAIDEN_H_

/*
"You've instantiated std::aligned_storage<Len, Align> with an extended alignment (in other "
		"words, Align > alignof(max_align_t)). Before VS 2017 15.8, the member type would "
		"non-conformingly have an alignment of only alignof(max_align_t). VS 2017 15.8 was fixed to "
		"handle this correctly, but the fix inherently changes layout and breaks binary compatibility "
		"(*only* for uses of aligned_storage with extended alignments). "
		"Please define either "
		"(1) _ENABLE_EXTENDED_ALIGNED_STORAGE to acknowledge that you understand this message and "
		"that you actually want a type with an extended alignment, or "
		"(2) _DISABLE_EXTENDED_ALIGNED_STORAGE to silence this message and get the old non-conformant "
		"behavior."*/
#if defined(_MSC_VER)
    #define _ENABLE_EXTENDED_ALIGNED_STORAGE
#endif

//raiden.h包含所有全局设置
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>
#include <assert.h>
#include <thread>
#include "platform.h"
#include "glog/logging.h"

#include "config.h"

//定义一个Float宏 可能指向float可能指向double
#ifdef RAIDEN_DOUBLE_PRECISION
	typedef double Float;
#elif  defined(RAIDEN_FLOAT_PRECISION)
	typedef float Float;
#else
	#error undefined float precision
#endif
//无限大数
static constexpr Float Infinity = std::numeric_limits<Float>::infinity();
static constexpr Float MaxFloat = std::numeric_limits<Float>::max();
 
#ifndef NDEBUG
	#define  RAIDEN_DEBUG
#endif // !NDEBUG
 
//三角面相关
//#define TRIANGLE_MESH_PRECOMPUTE_DATA_IF_CAN //预计算数据 比如dpdu,dpdv


//断言宏
#define Assert(x) assert(x)

//一如既往的全局Alloc函数,分配栈空间
#define ALLOCA(TYPE, COUNT) (TYPE *) alloca((COUNT) * sizeof(TYPE))

//打印错误的宏定义，我把它定义在这，PBRT的实现我没有深究，对我来说，这个目前已经做够了
// #define Error(x) LOG(ERROR)<<x;
#define LError LOG(ERROR)
#define LWarning LOG(WARNING)
#define LInfo LOG(INFO)

//命名空间
#define RAIDEN_NAMESPACE_BEGIN namespace raiden{
#define RAIDEN_NAMESPACE_END   };	


static constexpr Float Pi = (Float)3.14159265358979323846;
static constexpr Float InvPi = (Float)0.31830988618379067154;
static constexpr Float Inv2Pi = (Float)0.15915494309189533577;
static constexpr Float Inv4Pi = (Float)0.07957747154594766788;
static constexpr Float PiOver2 = (Float)1.57079632679489661923;
static constexpr Float PiOver4 = (Float)0.78539816339744830961;
static constexpr Float Sqrt2 = (Float)1.41421356237309504880;
static constexpr Float ShadowEpsilon = (Float)0.0001f;

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
class Quaternion;
class EFloat;
//计算累积误差的浮点数实现
class Interaction;
class SurfaceInteraction;
class MediumInteraction;
class Shape;
class Primitive;
class GeomPrimitive;
class Material;
class AreaLight;
template<int num> class CoefficientSpectrum;
class SampledSpectrum;
class RGBSpectrum;
class Film;
class FilmTile;
class Camera;
class Filter;
class Scene;
class Integrator;
//积分器
class Sampler;
class Light;
class MemoryArena;
class RNG;
class ParamSet;
class TextureParams;
template<typename T> class Texture;
class BSDF;
class BSSRDF;
class Material;
class VisibilityTester;
class Medium;
struct MediumInterface;
class PhaseFunction;
struct Distribution1D;
class MicrofacetDistribution;
template<typename T> class MIPMap;
//为啥这里默认要2呢？
template <typename T, int logBlockSize = 2>
class BlockedArray;


struct Options {
	int numThread = 0; //线程个数
	std::string imageFile; //储存的图片名字
};
extern Options RaidenOptions;

typedef RGBSpectrum Spectrum;
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
	return bits;
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
	if (f >= 0.0f)
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
	if (f == 0.0f)
		f = -0.0f;
	uint32_t bits = FloatToBits(f);
	if (f > 0.0f)
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
	if (d >= 0.0)
		bits += delta;
	else
		bits -= delta;
	return BitsToFloat(bits);
}

inline double NextFloatDown(double d, int delta = 1) {
	Assert(!std::isnan(d));
	if (std::isinf(d) && d < 0.0)
		return d;
	if (d == 0.0)
		d = -0.0;
	uint64_t bits = FloatToBits(d);
	if (d > 0)
		bits -= delta;
	else
		bits += delta;
	return BitsToFloat(bits);
}

//这个MachineEpsion是数值分析下的MachineEpsion；为2的-24次方；
//C++标准库提供的Epsion是大于1的ULP，为2的-23次方
//所以需要再除以2
static constexpr Float MachineEpsion = std::numeric_limits<Float>::epsilon()/(Float)2;

//这个gamma不是用来做Gamma校正的gamma，这个gamma是浮点数运算中，每一次运算后的最大ERR边界
inline constexpr Float gamma(int n) {
	return (n * MachineEpsion) / (1 - n * MachineEpsion);
}




template <typename T>
inline T Mod(T a, T b) {
	T ret = a - (a / b)*b;
	if (ret < 0) {
		 return ret + b;
	}
	return ret;
}

template <>
inline Float Mod(Float a, Float b) {
	return std::fmod(a, b);
}

//裁剪函数
// inline Float Clamp(Float val, Float low, Float high) {
// 	if (val < low)
// 		return low;
// 	else if (val > high)
// 		return high;
// 	else
// 		return val;
// }

template <typename T,typename U,typename V>
inline T Clamp(T val, U low, V high) {
	if (val < low)
		return low;
	else if (val > high)
		return high;
	else
		return val;
}

//线性插值
inline Float Lerp(Float val, Float min, Float max) {
	return min + (max - min) * val;
}

//线性插值
template<typename T>
inline T Lerp(Float val, T min, T max) {
	return min + (max - min) * val;
}

//角度转换弧度
inline Float Radians(Float deg) {
	return (Pi / 180) * deg;
}

//寻找区间 返回offset
//使用二分法来寻找区间,区间为offset~offset+1
//保证值有效的情况下，val[offset]满足预测函数，val[offset]不满足预测函数
//如果值不在提供的范围中，就会返回最前面或者最后的两个区间中的一个
template<typename PredicateFunc>
int FindInterval(int size, const PredicateFunc &pred) {
	int first = 0, len = size;
	while (len > 0) {
		int half = len >> 1, middle = first + half;
		if (pred(middle)) {
			first = middle + 1;
			len -= half + 1;
		} else
			len = half;
	}
	return Clamp(first - 1, 0, size - 2);
}

//gamma校验
//sRGB空间下crt显示器的gamma校验的近似曲线是个分段函数
inline Float GammaCorrect(Float value) {
    if (value <= 0.0031308f) return 12.92f * value;
    return 1.055f * std::pow(value, (Float)(1.f / 2.4f)) - 0.055f;
}
inline Float InverseGammaCorrect(Float value) {
    if (value <= 0.04045f) return value * 1.f / 12.92f;
    return std::pow((value + 0.055f) * 1.f / 1.055f, (Float)2.4f);
}


//判断是否是2的幂
template <typename T>
inline constexpr bool IsPowerOf2(T v) {
    return v && !(v & (v - 1));
}
//32位int到2的幂的转换函数
inline int32_t RoundUpPow2(int32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v + 1;
}
//64位int到2的幂的转换函数
inline int64_t RoundUpPow2(int64_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    return v + 1;
}

//求Float型的Log2
inline Float Log2(Float x) {
	//使用了换底公式
	const Float invLog2 = 1.442695040888963387004650940071;
	return std::log(x) * invLog2;
}


//求int的log2
//__builtin_clz是glibc内置函数，作用是返回左起第一个为1的位之前0的个数
inline int Log2Int(uint32_t v) {
#ifdef _WIN32
	unsigned long lz;
	if (_BitScanReverse(&lz, v >> 32))
		lz += 32;
	else {
		_BitScanReverse(&lz, v & 0xffffffff);
	}
	return lz;
#else
	return 31 - __builtin_clz(v);
#endif
}

//计算Sqr
inline Float Sqr(Float v){
	return v*v;
}


template<int n> static Float Pow(Float v){
	static_assert(n>=0,"Power can't be negative");
	Float v2=Pow<n/2>(v);
	return v2*v2*Pow<n&1>(v);
}

 template<> Float Pow<0>(Float v){
 	return 1;
 }

template<> Float Pow<1>(Float v){
	return v;
}


inline Float SafeASin(Float x){
	return std::asin(Clamp(x,-1,1));
}

inline Float SafeSqrt(Float x){
	return std::sqrt(std::max(Float(0),x));
}

#endif /* SRC_CORE_RAIDEN_H_ */

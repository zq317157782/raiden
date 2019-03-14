
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
#include "config.h"
#include "logging.h"

//定义一个Float宏 可能指向float可能指向double
#ifdef RAIDEN_DOUBLE_PRECISION
typedef double Float;
#elif defined(RAIDEN_FLOAT_PRECISION)
typedef float Float;
#else
#error undefined float precision
#endif

#ifndef NDEBUG
#define RAIDEN_DEBUG
#endif // !NDEBUG

//三角面相关
//#define TRIANGLE_MESH_PRECOMPUTE_DATA_IF_CAN //预计算数据 比如dpdu,dpdv

//断言宏
#define Assert(x) assert(x)

//一如既往的全局Alloc函数,分配栈空间
#define ALLOCA(TYPE, COUNT) (TYPE *)alloca((COUNT) * sizeof(TYPE))

//命名空间
#define RAIDEN_NAMESPACE_BEGIN \
	namespace raiden           \
	{
#define RAIDEN_NAMESPACE_END \
	}                        \
	;



template <typename T>
class Vector3;
template <typename T>
class Vector2;
template <typename T>
class Point3;
template <typename T>
class Point2;
template <typename T>
class Normal3;
template <typename T>
class Bound3;

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
template <int num>
class CoefficientSpectrum;
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
template <typename T>
class Texture;
class BSDF;
class BSSRDF;
class Material;
class VisibilityTester;
class Medium;
struct MediumInterface;
class PhaseFunction;
struct Distribution1D;
class MicrofacetDistribution;
template <typename T>
class MIPMap;
//为啥这里默认要2呢？
template <typename T, int logBlockSize = 2>
class BlockedArray;

struct Options
{
	int numThread = 0;	 //线程个数
	std::string imageFile; //储存的图片名字
};
extern Options RaidenOptions;

typedef RGBSpectrum Spectrum;


#endif /* SRC_CORE_RAIDEN_H_ */

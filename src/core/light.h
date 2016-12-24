/*
 * light.h
 *
 *  Created on: 2016年12月8日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_LIGHT_H_
#define SRC_CORE_LIGHT_H_
#include "raiden.h"
#include "geometry.h"
#include "transform.h"
#include "spectrum.h"
//第一次使用c++11中的enum class
//但是这里为啥要用这个新特性，完全无法理解
//这里定义了4个光源特征
enum class LightFlags
	:int {
		DeltaPosition = 1, DeltaDirection = 2, Area = 4, Infinite = 8
};

//一个用来判断光源是否是狄克尔光源的类型
inline bool IsDeltaLight(int flag) {
	return (flag & (int) LightFlags::DeltaDirection)
			|| (flag & (int) LightFlags::DeltaPosition);
}
class Light {
protected:
	const Transform _lightToWorld;
	const Transform _worldToLight;
public:
	const int numSamples; //采样光源的时候需要采样的样本个数
	const int flags; //光源的特征
public:
	Light(int flags, const Transform& l2w, int numSamples = 1) :
			_lightToWorld(l2w), _worldToLight(Inverse(l2w)), numSamples(
					numSamples), flags(flags) {

	}


	virtual Spectrum Le(const RayDifferential& ray) const {return Spectrum(0);};
	//返回入射光线方向以及相应的radiance
	virtual Spectrum Sample_Li(Interaction& interaction,Vector3f* wi,Float* pdf) const=0;
	//返回光源的flux
	virtual Spectrum Power() const = 0;
	virtual ~Light(){}
};

#endif /* SRC_CORE_LIGHT_H_ */

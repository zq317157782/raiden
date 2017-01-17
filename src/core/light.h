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
#include "interaction.h"
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

	//在scene被创建后，会被调用的函数
	//预处理
	virtual void Preprocess(Scene& scene) {}

	virtual Spectrum Le(const RayDifferential& ray) const {return Spectrum(0);};
	//返回入射光线方向以及相应的radiance
	virtual Spectrum Sample_Li(const Interaction& interaction,const Point2f &u,Vector3f* wi,Float* pdf, VisibilityTester* vis) const=0;
	//返回采样入射光线的pdf
	virtual Float Pdf_Li(const Interaction &ref, const Vector3f &wi) const = 0;
	//返回光源的flux
	virtual Spectrum Power() const = 0;
	//从光源角度采样radiance
	virtual Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
	                               Ray *ray, Normal3f *nLight, Float *pdfPos,
	                               Float *pdfDir) const = 0;
	//返回从光源采样光线的pdf相关数据
	virtual void Pdf_Le(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
				Float *pdfDir) const = 0;
	virtual ~Light(){}
};


//判断两个交点是否可见的结构
class VisibilityTester {
private:
	Interaction _p0;
	Interaction _p1;
public:
	VisibilityTester() {}
	VisibilityTester(const Interaction& p0, const Interaction& p1):_p0(p0),_p1(p1){

	}

	const Interaction& P0() { return _p0; }
	const Interaction& P1() { return _p1; }
	//判断两个点之间是否被遮挡
	bool Unoccluded(const Scene& scene) const;
};


class AreaLight :public Light {
public:
	AreaLight(const Transform& l2w, int numSamples):Light((int)LightFlags::Area,l2w,numSamples){}
	//计算从光源上的某个点往某个方向的radiance
	virtual Spectrum L(const Interaction& ref, const Vector3f& w) const=0;
};
#endif /* SRC_CORE_LIGHT_H_ */

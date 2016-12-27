/*
 * microfacet.h
 *
 *  Created on: 2016年12月27日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_MICROFACET_H_
#define SRC_CORE_MICROFACET_H_
#include "raiden.h"
class MicrofacetDistribution {
protected:
	MicrofacetDistribution(bool sampleVisibleArea) :
			_sampleVisibleArea(sampleVisibleArea) {
	}
	const bool _sampleVisibleArea;
public:
	virtual ~MicrofacetDistribution(){};
	//传入半角向量，返回相应方向的微平面总面积，或者说是法线分布
	virtual Float D(const Vector3f &wh) const = 0;
	//Lambda是 不可见微平面面积 与 可见微平面面积 的比值
	virtual Float Lambda(const Vector3f &w) const = 0;
	//返回有多少比例的微平面可以被W方向看见，微平面的可见性假设和微平面的WH无关
	Float G1(const Vector3f &w) const {
		//    if (Dot(w, wh) * CosTheta(w) < 0.0) return 0.0;
		return 1.0 / (1.0 + Lambda(w));
	}
	//返回有多少比例的微平面可以同时被wo和wi方向同时看到
	Float G(const Vector3f &wo, const Vector3f &wi) const {
		return 1.0 / (1.0 + Lambda(wo) + Lambda(wi));
	}
	//virtual Vector3f Sample_wh(const Vector3f &wo, const Point2f &u) const = 0;
	//Float Pdf(const Vector3f &wo, const Vector3f &wh) const;
	//virtual std::string ToString() const = 0;
};


#endif /* SRC_CORE_MICROFACET_H_ */

/*
 * distant.h
 *
 *  Created on: 2017年1月5日
 *      Author: zhuqian
 */

#ifndef SRC_LIGHTS_DISTANT_H_
#define SRC_LIGHTS_DISTANT_H_
#include "raiden.h"
#include "light.h"

class DistantLight :public Light {
private:
	Spectrum _L; //光源辐射度，radiance，单位Watte/(sr*m^2)
	Vector3f _direction;//光线的方向
	Point3f _worldCenter;
	Float  _worldRadius;
public:
	DistantLight(const Transform& l2w, const Spectrum& L,const Vector3f& dir) :
		Light((int)LightFlags::DeltaDirection, l2w),_L(L), _direction(dir){}
	virtual void Preprocess(Scene& scene) override;
	virtual Spectrum Sample_Li(const Interaction& interaction, const Point2f &u, Vector3f* wi,
		Float* pdf, VisibilityTester* vis) const override;
	virtual Float Pdf_Li(const Interaction &ref, const Vector3f &wi) const override { return 0.0f;};
	virtual Spectrum Power() const override;
	virtual Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
		Ray *ray, Normal3f *nLight, Float *pdfPos, Float *pdfDir) const
		override;
	virtual void Pdf_Le(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
		Float *pdfDir) const override;
};


std::shared_ptr<DistantLight> CreateDistantLight(const Transform &light2world,
	const ParamSet &paramSet);

#endif /* SRC_LIGHTS_DISTANT_H_ */

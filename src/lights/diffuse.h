/*
 * diffuse.h
 *
 *  Created on: 2017年1月17日
 *      Author: zhuqian
 */

#ifndef SRC_LIGHTS_DIFFUSE_H_
#define SRC_LIGHTS_DIFFUSE_H_
#include "raiden.h"
#include "light.h"

class DiffuseAreaLight: public AreaLight {
private:
	Spectrum _Le; //光源发射的radiance值
	std::shared_ptr<Shape> _shape; //光源的几何形状
	Float _area; //面积
public:
	DiffuseAreaLight(const Transform& l2w, int numSamples, const Spectrum& Le,
			const std::shared_ptr<Shape>& shape,const MediumInterface& mi) :
			AreaLight(l2w, mi,numSamples), _Le(Le), _shape(shape), _area(
					shape->Area()) {
	}
	Spectrum L(const Interaction& ref, const Vector3f& w) const override {
		if (Dot(Vector3f(ref.n), w) >= 0) {
			return _Le;
		} else {
			return 0;
		}
	}

	virtual Spectrum Sample_Li(const Interaction& interaction, const Point2f &u,
			Vector3f* wi, Float* pdf, VisibilityTester* vis) const override;
	virtual Float Pdf_Li(const Interaction &ref, const Vector3f &wi) const
			override;
	virtual Spectrum Power() const override;

	virtual Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
			Ray *ray, Normal3f *nLight, Float *pdfPos, Float *pdfDir) const
					override {
		Assert(false);
		return 0;
	}
	virtual void Pdf_Le(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
			Float *pdfDir) const override {
		Assert(false);
	}
};

std::shared_ptr<AreaLight> CreateDiffuseAreaLight(const Transform &light2world,
		const ParamSet &paramSet, const std::shared_ptr<Shape> &shape);

#endif /* SRC_LIGHTS_DIFFUSE_H_ */

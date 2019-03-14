/*
* spot.h
*
*  Created on: 2017年1月24日
*      Author: zhuqian
*/

#ifndef SRC_LIGHTS_SPOT_H_
#define SRC_LIGHTS_SPOT_H_
#include "raiden.h"
#include "light.h"
#include "mmath.h"
//聚光灯光源
class SpotLight : public Light {
private:
	Point3f _position; //位置，预计算，免得每次都要计算
	Spectrum _I; //光源强度，I，单位Watte/sr
	Float _cosTotalWidth;
	Float _cosFallOff;

	Float FallOff(const Vector3f& w) const;
public:
	SpotLight(const Transform& l2w,const MediumInterface& mediumInterface,const Spectrum& I,Float totalWidth,Float falloffStart) :
		Light((int)LightFlags::DeltaPosition, l2w, mediumInterface) {
		_position = _lightToWorld(Point3f(0, 0, 0)); //计算光源在世界坐标下的位置
		_I = I;
		_cosTotalWidth = std::cos(Radians(totalWidth));
		_cosFallOff = std::cos(Radians(falloffStart));
	}
	virtual Spectrum Sample_Li(const Interaction& interaction, const Point2f &u, Vector3f* wi,
		Float* pdf, VisibilityTester* vis) const override;
	virtual Float Pdf_Li(const Interaction &ref, const Vector3f &wi) const override;
	virtual Spectrum Power() const override;
	virtual Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
		Ray *ray, Normal3f *nLight, Float *pdfPos, Float *pdfDir) const
		override;
	virtual void Pdf_Le(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
		Float *pdfDir) const override;
};

std::shared_ptr<SpotLight> CreateSpotLight(const Transform &light2world, const Medium *medium,
	const ParamSet &paramSet);

#endif /* SRC_LIGHTS_SPOT_H_ */

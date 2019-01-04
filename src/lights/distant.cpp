/*
 * distant.cpp
 *
 *  Created on: 2017年1月5日
 *      Author: zhuqian
 */


#include "distant.h"
#include "scene.h"
#include "geometry.h"
#include "sampling.h"
#include "paramset.h"

void DistantLight::Preprocess(Scene& scene) {
	//获得场景的包围盒
	scene.WorldBound().BoundingSphere(&_worldCenter, &_worldRadius);
}

Spectrum DistantLight::Sample_Li(const Interaction& interaction, const Point2f &u, Vector3f* wi,
	Float* pdf, VisibilityTester* vis) const {
	*pdf = 1;
	*wi = _direction;
	Point3f p = interaction.p + _direction*(_worldRadius * 2);
	*vis = VisibilityTester(interaction,Interaction(p, interaction.time,mediumInterface));
	return _L;
}


Spectrum DistantLight::Power() const {
	//L*A  A==Pi*R^2;
	//近似成scene的包围球投影的disk的面积
	return _L*(Pi*_worldRadius*_worldRadius);
}


Spectrum DistantLight::Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
	Ray *ray, Normal3f *nLight, Float *pdfPos, Float *pdfDir) const {
	Vector3f v1, v2;
	CoordinateSystem(_direction,&v1,&v2);
	//采样disk
	Point2f u=ConcentricSampleDisk(u1);
	//获得一个在scene包围求投影的disk上的一个点
	Point3f point = _worldCenter + (u.x*v1 + u.y*v2)*_worldRadius;
	*ray=Ray(point+_direction*_worldRadius,-_direction,Infinity,time);
	*nLight = (Normal3f)(ray->d);
	*pdfPos = 1.0 / (Pi*_worldRadius*_worldRadius);
	*pdfDir = 1.0f;
	return _L;
}

void DistantLight::Pdf_Le(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
	Float *pdfDir) const {
	*pdfPos = 1.0 / (Pi*_worldRadius*_worldRadius);
	*pdfDir = 0.0f;//不可能通过其他采样方式采样到光线的方向
}

std::shared_ptr<DistantLight> CreateDistantLight(const Transform &light2world, const ParamSet &paramSet) {
	Spectrum L = paramSet.FindOneSpectrum("L", Spectrum(1.0));
	Spectrum sc = paramSet.FindOneSpectrum("scale", Spectrum(1.0));
	Point3f P = paramSet.FindOnePoint3f("from", Point3f(0, 0, 0));
	Point3f P2 = paramSet.FindOnePoint3f("to", Point3f(0, 0, 1));
	return std::make_shared<DistantLight>(light2world, L * sc,P-P2);
}

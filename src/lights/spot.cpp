#include "spot.h"
#include "paramset.h"

Float SpotLight::FallOff(const Vector3f& w) const {
	//求夹角
	Float cosTheta = Dot(_worldToLight(w), Vector3f(0, 0, 1));
	if (cosTheta > _cosFallOff) {
		return 1;
	}
	if (cosTheta < _cosTotalWidth) {
		return 0;
	}

	return (cosTheta - _cosTotalWidth) / (_cosFallOff - _cosTotalWidth);
}

Spectrum SpotLight::Sample_Li(const Interaction& interaction, const Point2f &u, Vector3f* wi,
	Float* pdf, VisibilityTester* vis) const{
	*wi = Normalize(_position-interaction.p);//指向光源的方向
	*pdf = 1;//delta分布
	*vis = VisibilityTester(interaction,Interaction(_position, interaction.time,mediumInterface));
	Float fallOff = FallOff(-*wi);//光源射出的方向
	return _I*fallOff/ DistanceSquared(_position,interaction.p);;
}

Float SpotLight::Pdf_Li(const Interaction &ref, const Vector3f &wi) const {
	return 0;
}

Spectrum SpotLight::Power() const {
	return 2 * Pi*(1 - (_cosFallOff + _cosTotalWidth)*0.5);
}

Spectrum SpotLight::Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
	Ray *ray, Normal3f *nLight, Float *pdfPos, Float *pdfDir) const {
	LWarning<<"SpotLight::Sample_Le invalid";
	Assert(false);
	return 0;
}

void SpotLight::Pdf_Le(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
	Float *pdfDir) const {
	LWarning<<"SpotLight::Pdf_Le invalid";
	Assert(false);
}


std::shared_ptr<SpotLight> CreateSpotLight(const Transform &light2world, const Medium *medium,
	const ParamSet &paramSet) {
	Spectrum I = paramSet.FindOneSpectrum("I", Spectrum(1.0));
	Spectrum sc = paramSet.FindOneSpectrum("scale", Spectrum(1.0));
	Point3f from = paramSet.FindOnePoint3f("from", Point3f(0, 0, 0));
	Point3f to = paramSet.FindOnePoint3f("to", Point3f(0, 0, 1));
	Float coneangle = paramSet.FindOneFloat("coneangle", 30.0);
	Float conedelta = paramSet.FindOneFloat("conedeltaangle", 5.0);

	Vector3f dir = Normalize(to - from);

	Vector3f du, dv;
	CoordinateSystem(dir, &du, &dv);

	Transform dirToZ =
		Transform(Matrix4x4(du.x, du.y, du.z, 0.0, dv.x, dv.y, dv.z, 0.0, dir.x,
			dir.y, dir.z, 0.0, 0, 0, 0, 1.0));
	Transform l2w =
		light2world * Translate(Vector3f(from.x, from.y, from.z)) * Inverse(dirToZ);
	Debug("[CreateSpotLight]");
	return std::make_shared<SpotLight>(l2w, MediumInterface(medium), I * sc, coneangle, coneangle-conedelta);
}
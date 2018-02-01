#pragma once
#include "raiden.h"
#include "imageio.h"
#include "spectrum.h"
#include "mipmap.h"
#include "light.h"
#include "scene.h"
//无限远面积光
//环境光
class InfiniteAreaLight :public Light {
private:
	Transform _worldToLight;
	Point3f _worldCenter;
	Float _worldRadius;

	std::unique_ptr<MIPMap<RGBSpectrum>> _Lmap;

public:
	InfiniteAreaLight(const Transform &LightToWorld,
		const Spectrum &L, int nSamples,
		const std::string &texmap):Light((int)LightFlags::Infinite, LightToWorld, MediumInterface(), nSamples), _worldToLight(Inverse(LightToWorld)){
		Point2i res;
		std::unique_ptr<RGBSpectrum[]> rawImage = nullptr;
		if (texmap != "") {
			rawImage = ReadImage(texmap.c_str(), &res);
			
			for (int i = 0; i < res.x*res.y; ++i) {
				rawImage[i] = rawImage[i] * L;
			}
		}
		


		if (!rawImage) {
			//处理没有读取到纹理的情况
		}

		//创建MipMap
		_Lmap.reset(new MIPMap<RGBSpectrum>(res, rawImage.get()));
	}

	virtual void Preprocess(Scene& scene) override {
		scene.WorldBound().BoundingSphere(&_worldCenter, &_worldRadius);
	}

	virtual Spectrum Le(const RayDifferential& ray)  const override  { return Spectrum(0); };
	//返回入射光线方向以及相应的radiance
	virtual Spectrum Sample_Li(const Interaction& interaction, const Point2f &u, Vector3f* wi, Float* pdf, VisibilityTester* vis) const override { return 0; }
	//返回采样入射光线的pdf
	virtual Float Pdf_Li(const Interaction &ref, const Vector3f &wi) const override { return 0; };
	//返回光源的flux
	virtual Spectrum Power() const override { return 0; };
	//从光源角度采样radiance
	virtual Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
		Ray *ray, Normal3f *nLight, Float *pdfPos,
		Float *pdfDir) const override {return 0;};
	//返回从光源采样光线的pdf相关数据
	virtual void Pdf_Le(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
		Float *pdfDir) const override {
	};
};

std::shared_ptr<InfiniteAreaLight> CreateInfiniteAreaLight(const Transform &light2world,
	const ParamSet &paramSet);
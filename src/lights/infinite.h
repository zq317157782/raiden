#pragma once
#include "raiden.h"
#include "imageio.h"
#include "spectrum.h"
#include "mipmap.h"
#include "light.h"
#include "scene.h"
#include "sampling.h"
//无限远面积光
//环境光
class InfiniteAreaLight :public Light {
private:
	Transform _worldToLight;
	Point3f _worldCenter;
	Float _worldRadius;

	std::unique_ptr<MIPMap<RGBSpectrum>> _Lmap;
	std::unique_ptr<Distribution2D> _distribution;
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
		//计算标量
		std::unique_ptr<Float[]> img(new Float[res.x*res.y]);
		
		Float filterWidth = 1.0/std::max(res.x, res.y);
		for (int j = 0; j < res.y; ++j) {
			//计算防止扭曲的sintheta
			Float sinTheta=std::sin((Float(j + 0.5)/res.y)*Pi);
			Float v = (Float)j / res.y;
			for (int i = 0; i < res.x; ++i) {
				Float u = (Float)i / res.y;
				//把相应的RGB值转换成标量
				img[j*res.x + i] = _Lmap->Lookup(Point2f(u,v),filterWidth).y();
				//乘以防止2D到球面映射的扭曲的系数
				img[j*res.x + i] *= sinTheta;
			}
		}

		//再把标量转换成2D PDF
		_distribution.reset(new Distribution2D(img.get(),res.x,res.y));
	}

	virtual void Preprocess(Scene& scene) override {
		//计算世界包围球
		scene.WorldBound().BoundingSphere(&_worldCenter, &_worldRadius);
	}

	virtual Spectrum Le(const RayDifferential& ray)  const override  {
		//转换世界坐标系下的方向到光源坐标系下
		Vector3f dir=Normalize(_worldToLight(ray.d));
		//把方向(立体角空间)转换到球面坐标
		Float phi = SphericalPhi(dir);
		Float theta = SphericalPhi(dir);
		//从球面坐标向[0~1]空间转换
		Point2f uv;
		uv[1] = theta*InvPi;
		uv[0] = phi * Inv2Pi;
		//采样radiance
		return Spectrum(_Lmap->Lookup(uv));
	};
	//返回入射光线方向以及相应的radiance
	virtual Spectrum Sample_Li(const Interaction& interaction, const Point2f &u, Vector3f* wi, Float* pdf, VisibilityTester* vis) const override {
		//先采样2D分布获取具体的uv和pdf
		Float pdf2D;
		Point2f uv= _distribution->SampleContinuous(u,&pdf2D);
		if (pdf2D == 0) {
			return Spectrum(0);
		}
		//从UV转换到球面坐标
		Float phi = uv[0] * Pi * 2;
		Float theta = uv[1] * Pi;

		Float cosTheta = std::cos(theta);
		Float sinTheta = std::sin(theta);

		//从球面坐标到立体角空间
		Vector3f w=SphericalDirection(sinTheta,cosTheta,phi);
		*wi = _lightToWorld(w);

		if (sinTheta == 0) {
			*pdf = 0;
			return Spectrum(0.0);
		}

		//计算立体角空间下的pdf
		*pdf = pdf2D / (2 * Pi*Pi*sinTheta);
		

		//这就没啥好解释了
		*vis = VisibilityTester(interaction,Interaction(interaction.p+*wi*_worldRadius*2,interaction.time,mediumInterface));

		return Spectrum(_Lmap->Lookup(uv));
	}
	//返回采样入射光线的pdf
	virtual Float Pdf_Li(const Interaction &ref, const Vector3f &wi) const override {
		Vector3f w = Normalize(_worldToLight(wi));
		Float phi = SphericalPhi(w);
		Float theta = SphericalTheta(w);
		Float sinTheta = std::sin(theta);
		if (sinTheta == 0) {
			return 0.0;
		}
		Point2f uv(phi*Inv2Pi, theta*InvPi);
		Float pdf=_distribution->Pdf(uv);
		//转换到立体角空间
		return pdf / (2 * Pi*Pi*sinTheta);
	}
	//返回光源的flux
	virtual Spectrum Power() const override { 
		return Pi*_worldRadius*_worldRadius*_Lmap->Lookup(Point2f(0.5,0.5),0.5);
	};
	//从光源角度采样radiance
	virtual Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
		Ray *ray, Normal3f *nLight, Float *pdfPos,
		Float *pdfDir) const override {
	//先采样方向
		Float pdf2D;
		Point2f uv=_distribution->SampleContinuous(u1, &pdf2D);
		if (pdf2D == 0) {
			return Spectrum(0);
		}
		//转换到球面坐标
		Float theta = uv[1] * Pi;
		Float phi = uv[0] * 2 * Pi;
		
		Float sinTheta = std::sin(theta);
		Float cosTheta = std::cos(theta);
		Float sinPhi = std::sin(phi);
		Float cosPhi = std::cos(phi);

		//TODO 这里可以优化成不调用函数的形式
		Vector3f w=-_lightToWorld(SphericalDirection(sinTheta, cosTheta, phi));
		*nLight = (Normal3f)w;

		//采样圆盘
		Vector3f v1, v2;
		CoordinateSystem(-w, &v1, &v2);
		Point2f sample= ConcentricSampleDisk(u2);
		Point3f posW=_worldCenter + _worldRadius*(sample.x*v1 + sample.y*v2);
		*ray = Ray(posW+_worldRadius*(-w),w,Infinity,time);

		//立体角空间
		*pdfDir = sinTheta == 0 ? 0 : pdf2D / (2 * Pi*Pi*sinTheta);
		*pdfPos = 1.0 / (Pi*_worldRadius*_worldRadius);
		
		return Spectrum(_Lmap->Lookup(uv));
	};
	//返回从光源采样光线的pdf相关数据
	virtual void Pdf_Le(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
		Float *pdfDir) const override {
		Vector3f d = Normalize(-_worldToLight(ray.d));
		Float theta = SphericalTheta(d);
		Float phi = SphericalPhi(d);

		Point2f uv(phi*Inv2Pi, theta*InvPi);

		Float pdf2D=_distribution->Pdf(uv);

		Float sinTheta = std::sin(theta);

		*pdfDir = sinTheta == 0 ? 0 : pdf2D / (2 * Pi*Pi*sinTheta);
		*pdfPos = 1.0 / (Pi*_worldRadius*_worldRadius);
	};
};

std::shared_ptr<InfiniteAreaLight> CreateInfiniteAreaLight(const Transform &light2world,
	const ParamSet &paramSet);
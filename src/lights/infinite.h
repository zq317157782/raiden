#pragma once
#include "raiden.h"
#include "imageio.h"
#include "spectrum.h"
#include "mipmap.h"
#include "light.h"
#include "scene.h"
#include "sampling.h"
#include "mmath.h"
//����Զ�����
//������
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
			//����û�ж�ȡ�����������
		}

		//����MipMap
		_Lmap.reset(new MIPMap<RGBSpectrum>(res, rawImage.get()));
		//�������
		std::unique_ptr<Float[]> img(new Float[res.x*res.y]);
		
		Float filterWidth = 1.0/std::max(res.x, res.y);
		for (int j = 0; j < res.y; ++j) {
			//�����ֹŤ����sintheta
			Float sinTheta=std::sin((Float(j + 0.5)/res.y)*Pi);
			Float v = (Float)j / res.y;
			for (int i = 0; i < res.x; ++i) {
				Float u = (Float)i / res.x;
				//����Ӧ��RGBֵת���ɱ���
				img[j*res.x + i] = _Lmap->Lookup(Point2f(u,v),filterWidth).y();
				//���Է�ֹ2D������ӳ���Ť����ϵ��
				img[j*res.x + i] *= sinTheta;
			}
		}

		//�ٰѱ���ת����2D PDF
		_distribution.reset(new Distribution2D(img.get(),res.x,res.y));
	}

	virtual void Preprocess(Scene& scene) override {
		//���������Χ��
		scene.WorldBound().BoundingSphere(&_worldCenter, &_worldRadius);
	}

	virtual Spectrum Le(const RayDifferential& ray)  const override  {
		//ת����������ϵ�µķ��򵽹�Դ����ϵ��
		Vector3f dir=Normalize(_worldToLight(ray.d));
		//�ѷ���(����ǿռ�)ת������������
		Float phi = SphericalPhi(dir);
		Float theta = SphericalTheta(dir);
		//������������[0~1]�ռ�ת��
		Point2f uv;
		uv[1] = theta*InvPi;
		uv[0] = phi * Inv2Pi;
		
		//����radiance
		return Spectrum(_Lmap->Lookup(uv));
	};
	//����������߷����Լ���Ӧ��radiance
	virtual Spectrum Sample_Li(const Interaction& interaction, const Point2f &u, Vector3f* wi, Float* pdf, VisibilityTester* vis) const override {
		//�Ȳ���2D�ֲ���ȡ�����uv��pdf
		Float pdf2D;
		Point2f uv= _distribution->SampleContinuous(u,&pdf2D);
		if (pdf2D == 0) {
			return Spectrum(0);
		}
		//��UVת������������
		Float phi = uv[0] * Pi * 2;
		Float theta = uv[1] * Pi;

		Float cosTheta = std::cos(theta);
		Float sinTheta = std::sin(theta);

		//���������굽����ǿռ�
		Vector3f w=SphericalDirection(sinTheta,cosTheta,phi);
		*wi = _lightToWorld(w);

		if (sinTheta == 0) {
			*pdf = 0;
			return Spectrum(0.0);
		}

		//��������ǿռ��µ�pdf
		*pdf = pdf2D / (2 * Pi*Pi*sinTheta);
		

		//���ûɶ�ý�����
		*vis = VisibilityTester(interaction,Interaction(interaction.p+*wi*_worldRadius*2,interaction.time,mediumInterface));

		return Spectrum(_Lmap->Lookup(uv));
	}
	//���ز���������ߵ�pdf
	virtual Float Pdf_Li(const Interaction &ref, const Vector3f &wi) const override {
		Vector3f w =Normalize( _worldToLight(wi));
		Float phi = SphericalPhi(w);
		Float theta = SphericalTheta(w);
		Float sinTheta = std::sin(theta);
		if (sinTheta == 0) {
			return 0.0;
		}
		Point2f uv(phi*Inv2Pi, theta*InvPi);
		Float pdf=_distribution->Pdf(uv);
		//ת��������ǿռ�
		return pdf / (2 * Pi*Pi*sinTheta);
	}
	//���ع�Դ��flux
	virtual Spectrum Power() const override { 
		return Pi*_worldRadius*_worldRadius*_Lmap->Lookup(Point2f(0.5,0.5),0.5);
	};
	//�ӹ�Դ�ǶȲ���radiance
	virtual Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
		Ray *ray, Normal3f *nLight, Float *pdfPos,
		Float *pdfDir) const override {
	//�Ȳ�������
		Float pdf2D;
		Point2f uv=_distribution->SampleContinuous(u1, &pdf2D);
		if (pdf2D == 0) {
			return Spectrum(0);
		}
		//ת������������
		Float theta = uv[1] * Pi;
		Float phi = uv[0] * 2 * Pi;
		
		Float sinTheta = std::sin(theta);
		Float cosTheta = std::cos(theta);
		Float sinPhi = std::sin(phi);
		Float cosPhi = std::cos(phi);

		//TODO ��������Ż��ɲ����ú�������ʽ
		Vector3f w=-_lightToWorld(SphericalDirection(sinTheta, cosTheta, phi));
		*nLight = (Normal3f)w;

		//����Բ��
		Vector3f v1, v2;
		CoordinateSystem(-w, &v1, &v2);
		Point2f sample= ConcentricSampleDisk(u2);
		Point3f posW=_worldCenter + _worldRadius*(sample.x*v1 + sample.y*v2);
		*ray = Ray(posW+_worldRadius*(-w),w,Infinity,time);

		//����ǿռ�
		*pdfDir = sinTheta == 0 ? 0 : pdf2D / (2 * Pi*Pi*sinTheta);
		*pdfPos = 1.0 / (Pi*_worldRadius*_worldRadius);
		
		return Spectrum(_Lmap->Lookup(uv));
	};
	//���شӹ�Դ�������ߵ�pdf�������
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
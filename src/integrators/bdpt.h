/*
 * bdpt.h
 *
 *  Created on: 2017年5月7日
 *      Author: zhuqian
 */

#ifndef SRC_INTEGRATORS_BDPT_H_
#define SRC_INTEGRATORS_BDPT_H_

#include "raiden.h"
#include "interaction.h"
#include "integrator.h"
#include "camera.h"
#include "film.h"
#include "reflection.h"
#include "light.h"
#include "primitive.h"
//代表lens上的或者光源上的一个点
struct EndpointInteraction: public Interaction {
public:
	//使用联合体来指向光源和相机，同一时间只有一个有效
	//永远都不会改变相应的光源/相机属性，所以设置成const
	union {
		const Light *light; //指向光源的指针
		const Camera *camera; //指向相机的指针
	};
public:
	//默认构造函数
	EndpointInteraction() :
			Interaction(), light(nullptr) {
	}
	//初始化代表相机的点
	EndpointInteraction(const Camera* cam, Ray& ray) :
			Interaction(ray.o, ray.time, ray.medium), camera(cam) {
	}
	EndpointInteraction(const Interaction& interaction, const Camera* cam) :
			Interaction(interaction), camera(cam) {
	}
	//初始化代表光源的点
	EndpointInteraction(const Light* _light, Ray& ray, Normal3f& normal) :
			Interaction(ray.o, ray.time, ray.medium), light(_light) {
		this->n = normal; //这里是离开光源的射线所在的点所在的表面的法线
	}
	//这里对应的光源表面的法线信息已经在interaction中了，所以不需要显示提供法线
	EndpointInteraction(const Interaction& interaction, const Light* _light) :
			Interaction(interaction), light(_light) {
	}
};

//详见Vech的论文[Robust Monte Carlo Methods for Light Transport],解释得非常到位
//只有的表面交点的情况下，才会出现这种情况
Float CorrectShadingNormal(const SurfaceInteraction& ref, const Vector3f& wo,
		const Vector3f& wi, TransportMode mode);

//双向路径追踪
class BDPTIntegrator: public Integrator {
private:
	std::shared_ptr<const Camera> _camera;
public:
	BDPTIntegrator(const std::shared_ptr<const Camera>& camera) :
			_camera(camera) {
	}
	virtual void Render(const Scene&) override {
		_camera->film->WriteImage();
	}
};

//四种顶点类型
enum class VertexType {
	Camera, Light, Surface, Medium
};
//代表路径中的Vertex
struct Vertex {
	VertexType type;
	Spectrum beta; //代表的是累积的贡献
	//代表实际的交点
	union {
		EndpointInteraction ei;
		SurfaceInteraction si;
		MediumInteraction mi;
	};
	bool delta = false;	//标识当前顶点是否包含Dirac分布
	Float pdfFwd = 0;	//采样这个点的概率(立体角)
	Float pdfRev = 0;	//逆向采样这个点的概率(立体角)

	Vertex() :
			ei()/*si;ei;*/{
	}
	//使用SurfaceInteraction来初始化顶点的话，可以确定顶点类型，所以只需要两个参数就可以
	Vertex(const SurfaceInteraction& surfaceInteraction, const Spectrum& beta) :
			type(VertexType::Surface), si(surfaceInteraction), beta(beta) {
	}

	//初始化代表EndPoint的顶点，需要提供是Camera还是Light
	Vertex(VertexType vertexType,
			const EndpointInteraction& endpointInteraction,
			const Spectrum& beta) :
			type(vertexType), ei(endpointInteraction), beta(beta) {
	}

	//初始化Medium顶点
	Vertex(const MediumInteraction& mediumInteraction, const Spectrum& beta) :
			type(VertexType::Medium), mi(mediumInteraction), beta(beta) {
	}

	//这里保留了PBRT的解释
	// Need to define these two to make compilers happy with the non-POD
	// objects in the anonymous union above.
	Vertex(const Vertex &v) {
		memcpy(this, &v, sizeof(Vertex));
	}
	Vertex &operator=(const Vertex &v) {
		memcpy(this, &v, sizeof(Vertex));
		return *this;
	}

	//获取Interaction的引用
	const Interaction& GetInteraction() const {
		//PBRT这里为了可读性，进行了类型的判断，我这里直接去除了
		return ei;
	}

	//返回空间点
	const Point3f& p() const {
		return GetInteraction().p;
	}

	//返回几何法线
	const Normal3f& ng() const {
		return GetInteraction().n;
	}

	//返回着色法线,只有Surface有着色法线，其余的直接用几何法线
	const Normal3f& ns() const {
		if (type == VertexType::Surface) {
			return si.shading.n;
		}
		return GetInteraction().n;
	}

	//判断是否在Surface上，用法线来判断，如果没有法线就说明不在Surface上
	bool IsOnSurface() const {
		if (GetInteraction().n == Normal3f(0, 0, 0)) {
			return false;
		}
		return true;
	}

	//计算相应的brdf系数
	//因为wo已经在interaction中了，所以这里只要提供next节点，以及transport mdoe
	Spectrum f(const Vertex& next, TransportMode mode) const {
		//计算wi
		Vector3f wi = next.p() - p();
		if (wi.LengthSquared() == 0) {
			return Spectrum(0);
		}
		wi = Normalize(wi);
		switch (type) {
		case VertexType::Surface: {
			//这里要修正shading normal带来的不对称性
			return si.bsdf->f(si.wo, wi)
					* CorrectShadingNormal(si, si.wo, wi, mode);
		}
			break;
		case VertexType::Medium: {
			//中间介质不需要关心shading normal带来的不对称性
			return mi.phase->P(mi.wo, wi);
		}
			break;

		default: {
			Warning("Unimplement for this type!");
			return Spectrum(0);
		}
		}
	}

	//判断是否能够连接，其实就是判断是否有Dirac分布
	bool IsConnectable() const {
		if (type == VertexType::Camera) {
			return true;
		} else if (type == VertexType::Light) {
			return ei.light->flags & (int) LightFlags::DeltaDirection == 0;
		} else if (type == VertexType::Medium) {
			return true;
		} else if (type == VertexType::Surface) {
			return si.bsdf->NumComponents(
					BxDFType(
							BxDFType::BSDF_REFLECTION
									| BxDFType::BSDF_TRANSMISSION
									| BxDFType::BSDF_DIFFUSE
									| BxDFType::BSDF_GLOSSY)) > 0;
		}
		return false;
	}

	//判断是否是光源
	//1.type为光源
	//2.type为Surface 并且是区域光
	bool IsLight() const {
		if (type == VertexType::Light
				|| (type == VertexType::Surface && si.primitive->GetAreaLight())) {
			return true;
		}
		return false;
	}

	//判断是否是delta光源
	bool IsDeltaLight() const {
	if(type==VertexType::Light&&ei.light&&::IsDeltaLight(ei.light->flags)){
		return true;
	}
	return false;
}

	//TODO InfiniteLight相关

	//获取这个点的自发光能量
	Spectrum Le(const Scene& scene,const Vertex& v) const{
		//先判断是否是光源
		if(!IsLight()){
			return Spectrum(0);
		}
		Vector3f w=v.p()-p();
		if(w.LengthSquared()==0){
			return Spectrum(0);
		}
		w=Normalize(w);
		//TODO 这里也有InfiniteLight相关
		const AreaLight* light=si.primitive->GetAreaLight();
		return light->L(si,w);
	}
};

BDPTIntegrator *CreateBDPTIntegrator(const ParamSet &params,
		std::shared_ptr<Sampler> sampler, std::shared_ptr<const Camera> camera);

#endif /* SRC_INTEGRATORS_BDPT_H_ */

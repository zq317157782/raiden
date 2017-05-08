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
	Vertex(VertexType vertexType,const EndpointInteraction& endpointInteraction,const Spectrum& beta):
		type(vertexType),ei(endpointInteraction),beta(beta){}
};

BDPTIntegrator *CreateBDPTIntegrator(const ParamSet &params,
		std::shared_ptr<Sampler> sampler, std::shared_ptr<const Camera> camera);

#endif /* SRC_INTEGRATORS_BDPT_H_ */

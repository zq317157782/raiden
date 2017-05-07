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

//TODO 还没有完全理解，需要进一步的理解修正shading normal带来的非对称操作的步骤
//只有的表面交点的情况下，才会出现这种情况
Float CorrectShadingNormal(const SurfaceInteraction& ref,const Vector3f& wo,const Vector3f& wi,TransportMode mode);

#endif /* SRC_INTEGRATORS_BDPT_H_ */

/*
 * bdpt.cpp
 *
 *  Created on: 2017年5月7日
 *      Author: zhuqian
 */
#include "bdpt.h"
#include "geometry.h"
#include "sampler.h"
Float CorrectShadingNormal(const SurfaceInteraction& ref, const Vector3f& wo,
		const Vector3f& wi, TransportMode mode) {
	if (mode == TransportMode::Importance) {
		//向前传播
		return (AbsDot(ref.shading.n, wo) * AbsDot(ref.n, wi))
				/ (AbsDot(ref.n, wo) * AbsDot(ref.shading.n, wi));
	} else {
		//向后传播
		return 1.0;
	}
}

//RandomWalk函数，用于生成一条长度小于等于maxDepth的路径
int RandomWalk(const Scene&scene, RayDifferential ray, Sampler& sampler,
		MemoryArena& arena, Spectrum beta, Float pdf, int maxDepth,
		TransportMode mode, Vertex* path) {
//如果最大深度为0，则不需要生成路径
	if (maxDepth == 0) {
		return 0;
	}
	//射线反射的次数
	int bounces = 0;

	//相应的pdf
	//应该是area measurement
	Float pdfFwd = pdf, pdfRev;

	while (true) {
		MediumInteraction mi;	//和中间介质的相交点
		SurfaceInteraction si;	//和表面的相交点
		bool isHit = scene.Intersect(ray, &si);	//寻找相交点
		//如果存在medium 需要采样medium
		if (ray.medium) {
			beta = beta * ray.medium->Sample(ray, sampler, arena, &mi);
		}
		//判断当前的能量是否为0
		if (beta.IsBlack()) {
			break;
		}
		//获取当前顶点，以及前一个顶点
		Vertex& vertex = path[bounces];
		Vertex& preV = path[bounces - 1];		//传进来的path是从第二个开始顶点开始的

		if (mi.IsValid()) {
			vertex = Vertex::CreateMedium(mi, beta, pdfFwd, preV);
			++bounces;
			if (bounces >= maxDepth) {
				break;
			}
			Vector3f wi;
			//采样新的方向，这里的pdf是立体角的pdf
			pdfFwd = pdfRev = mi.phase->Sample_p(-ray.d, &wi,
					sampler.Get2DSample());
			//产生新的射线
			ray=mi.SpawnRay(wi);
		} else {

		}
	}
	//返回路径的长度
	return bounces;
}

BDPTIntegrator *CreateBDPTIntegrator(const ParamSet &params,
		std::shared_ptr<Sampler> sampler,
		std::shared_ptr<const Camera> camera) {
	return new BDPTIntegrator(camera);
}

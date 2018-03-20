/*
 * bdpt.cpp
 *
 *  Created on: 2017年5月7日
 *      Author: zhuqian
 */
#include "bdpt.h"
#include "geometry.h"
#include "sampler.h"
#include "paramset.h"
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
			pdfFwd = pdfRev = mi.phase->Sample_p(mi.wo, &wi,
					sampler.Get2DSample());
			//产生新的射线
			ray=mi.SpawnRay(wi);
		} else {
			if (!isHit) {
				//TODO InfiniteLight
				return bounces;
			}
			//处理medium边界
			si.ComputeScatteringFunctions(ray,arena,true,mode);
			if (si.bsdf == nullptr) {
				ray=si.SpawnRay(ray.d);
				continue;
			}
			//生成顶点
			vertex = Vertex::CreateSurface(si,beta,pdfFwd,preV);
			++bounces;
			if (bounces >= maxDepth) {
				break;
			}
			//采样新的节点
			Vector3f wi;
			BxDFType flag;
			auto f=si.bsdf->Sample_f(si.wo,&wi, sampler.Get2DSample(),&pdfFwd,BSDF_ALL,&flag);
			if (f.IsBlack() || pdfFwd == 0) {
				break;
			}
			//更新beta
			beta = beta*f*AbsDot(si.shading.n, wi) / pdfFwd;
			//计算revPdf
			pdfRev = si.bsdf->Pdf(wi, si.wo, BSDF_ALL);
			//判断当前顶点是否包含Dirac分布
			if (flag&BSDF_SPECULAR) {
				vertex.delta = true;
				pdfFwd = pdfRev = 0;
			}
			//修正shading normal带来的不对称关系
			beta = beta*CorrectShadingNormal(si,si.wo,wi,mode);
		}
		//转换pdf到area，并且赋值给前一个顶点
		preV.pdfRev = vertex.ConvertDensity(pdfRev, preV);
	}
	//返回路径的长度
	return bounces;
}

//生成相机SubPath
int GenerateCameraSubPath(const Scene& scene,Sampler& sampler,MemoryArena& arena,const Camera& camera,int maxDepth,const Point2f& pFilm,Vertex* path) {
	if (maxDepth == 0) {
		return 0;
	}

	CameraSample sample;
	sample.pFilm = pFilm;
	sample.pLen = sampler.Get2DSample();
	sample.time = sampler.Get1DSample();
	//生成相机射线
	RayDifferential ray;
	Spectrum beta=camera.GenerateRayDifferential(sample, &ray);
	ray.ScaleRayDifferential(1.0 / std::sqrt(sampler.samplesPerPixel));

	Float dirPdf,posPdf;
	camera.Pdf_We(ray, &posPdf, &dirPdf);

	return RandomWalk(scene,ray,sampler,arena,beta,dirPdf, maxDepth-1,TransportMode::Radiance,path+1)+1;
}

BDPTIntegrator *CreateBDPTIntegrator(const ParamSet &params,
		std::shared_ptr<Sampler> sampler,
		std::shared_ptr<const Camera> camera) {
	int np;
	const int *pb = params.FindInt("pixelbounds", &np);
	Bound2i pixelBounds = camera->film->GetSampleBounds();
	if (pb) {
		if (np != 4)
			printf("need four\"pixelbounds\"param.actual:%d.",
				np);
		else {
			pixelBounds = Intersect(pixelBounds, Bound2i{ { pb[0], pb[2] },{ pb[1], pb[3] } });
			if (pixelBounds.Area() == 0) {
				Error("\"pixelbounds\"is a tuihua bound.");
			}
		}
	}

	int depth = params.FindOneInt("depth", 5);
	return new BDPTIntegrator(camera, sampler,depth,pixelBounds);
}

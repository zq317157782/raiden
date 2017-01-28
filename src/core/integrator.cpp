/*
 * integrator.cpp
 *
 *  Created on: 2016年12月7日
 *      Author: zhuqian
 */
#include "integrator.h"
#include "camera.h"
#include "film.h"
#include "scene.h"
#include "sampler.h"
#include "interaction.h"
#include "light.h"
#include "parallel.h"
#include "memory.h"
#include "reflection.h"
#include "sampling.h"
#include "progressreporter.h"

//这个函数主要是为了解决RayDifferential的生成
Spectrum SamplerIntegrator::SpecularReflect(const RayDifferential& ray, const SurfaceInteraction& isect, const Scene&scene, Sampler& sampler, MemoryArena& arena, int depth) const {
	BxDFType type = BxDFType(BSDF_REFLECTION | BSDF_SPECULAR);//镜面反射
	Normal3f ns = isect.shading.n;
	BSDF* bsdf = isect.bsdf;
	Vector3f wo = isect.wo;
	Vector3f wi;
	Float pdf;
	//采样反射射线
	Spectrum f=bsdf->Sample_f(wo,&wi,sampler.Get2DSample(),&pdf,type);
	Float costheta = AbsDot(wi, ns);
	//条件满足才说明反射有贡献
	if (pdf > 0 && !f.IsBlack() && costheta != 0) {
		RayDifferential rayReflect = isect.SpawnRay(wi);//生成反射射线
		if (ray.hasDifferential) {
			rayReflect.hasDifferential = true;
			//计算offset射线的原点
			rayReflect.ox = isect.p + isect.dpdx;
			rayReflect.oy = isect.p + isect.dpdy;

			//计算offset射线的方向
			//1.先计算dndx/dndy 和 dwodx/dwody
			//2.用反射公式计算 dwidx/dwidy
			Normal3f dndx = isect.dndu*isect.dudx + isect.dndv*isect.dvdx;
			Normal3f dndy = isect.dndu*isect.dudy + isect.dndv*isect.dvdy;

			Vector3f dwodx = -ray.dx - wo;
			Vector3f dwody = -ray.dy - wo;

			Float dwondx = Dot(dwodx, ns) + Dot(wo, dndx);
			Float dwondy = Dot(dwody, ns) + Dot(wo, dndy);
			//反射公式的偏导公式
			rayReflect.dx = wi - dwodx + Vector3f(2.0f*(Dot(wo, ns)*dndx + dwondx*ns));
			rayReflect.dy = wi - dwody + Vector3f(2.0f*(Dot(wo, ns)*dndy + dwondy*ns));
		}
		//计算蒙特卡洛估计式的一个item
		return f*Li(rayReflect, scene, sampler, arena, depth+1)*costheta/pdf;
	}
	else {
		return Spectrum(0);
	}
}

Spectrum SamplerIntegrator::SpecularTransmit(const RayDifferential& ray, const SurfaceInteraction& isect, const Scene&scene, Sampler& sampler, MemoryArena& arena, int depth) const {
	BxDFType type = BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR);//镜面折射
	Normal3f ns = isect.shading.n;
	BSDF* bsdf = isect.bsdf;
	Vector3f wo = isect.wo;
	Vector3f wi;
	Float pdf;
	//采样反射射线
	Spectrum f = bsdf->Sample_f(wo, &wi, sampler.Get2DSample(), &pdf, type);
	Float costheta = AbsDot(wi, ns);
	if (pdf > 0 && !f.IsBlack() && costheta != 0) {
		RayDifferential rayRefract = isect.SpawnRay(wi);//生成折射射线
		if (ray.hasDifferential) {
			rayRefract.hasDifferential = true;
			//计算offset射线的原点
			rayRefract.ox = isect.p + isect.dpdx;
			rayRefract.oy = isect.p + isect.dpdy;
			Float eta = bsdf->eta;
			//说明入射射线在表面下面，需要反转折射系数比例
			if (Dot(wo, ns) < 0) {
				eta = 1.0 / eta;
			}

			//和镜面反射一样，求折射公式的一阶导数
			Normal3f dndx = isect.dndu*isect.dudx + isect.dndv*isect.dvdx;
			Normal3f dndy = isect.dndu*isect.dudy + isect.dndv*isect.dvdy;

			Vector3f dwodx = -ray.dx - wo;
			Vector3f dwody = -ray.dy - wo;

			Float dwondx = Dot(dwodx, ns) + Dot(wo, dndx);
			Float dwondy = Dot(dwody, ns) + Dot(wo, dndy);


			Vector3f w = -wo;
			Float mu = eta * Dot(w, ns) - Dot(wi, ns);
			Float dmudx =
				(eta - (eta * eta * Dot(w, ns)) / Dot(wi, ns)) * dwondx;
			Float dmudy =
				(eta - (eta * eta * Dot(w, ns)) / Dot(wi, ns)) * dwondy;

			rayRefract.dx= wi + eta * dwodx - Vector3f(mu * dndx + dmudx * ns);
			rayRefract.dy= wi + eta * dwody - Vector3f(mu * dndy + dmudy * ns);
		}
		//计算蒙特卡洛估计式的一个item
		return f*Li(rayRefract, scene, sampler, arena, depth + 1)*costheta / pdf;
	}
	else {
		return Spectrum(0.0f);
	}
}

void SamplerIntegrator::RenderScene(const Scene& scene) {
	//首先计算需要的tile数
	Bound2i filmBound = _camera->film->GetSampleBounds();
	Debug("[SamplerIntegrator::RenderScene][filmBound:"<<filmBound<<"]");
	Vector2i filmExtent = filmBound.Diagonal();
	const int tileSize = 16; //默认是16*16为1个tile
	int numTileX = (filmExtent.x + tileSize - 1) / tileSize;
	int numTileY = (filmExtent.y + tileSize - 1) / tileSize;
	Point2i numTile(numTileX, numTileY);
	ProgressReporter reporter(numTile.x*numTile.y,"Rendering");
	//开始并行处理每个tile
	ParallelFor2D([&](Point2i tile) {
//<<并行循环体开始>>
			MemoryArena arena;
			int seed=tile.y*numTile.x+tile.x;//计算种子数据
			std::unique_ptr<Sampler> localSampler=_sampler->Clone(seed);
			//计算这个tile覆盖的像素范围
			int x0=filmBound.minPoint.x+tile.x*tileSize;
			int y0=filmBound.minPoint.y+tile.y*tileSize;
			int x1=std::min(x0+tileSize,filmBound.maxPoint.x);
			int y1=std::min(y0+tileSize,filmBound.maxPoint.y);
			Bound2i tileBound(Point2i(x0,y0),Point2i(x1,y1));
			//获取tile
			std::unique_ptr<FilmTile> filmTile=_camera->film->GetFilmTile(tileBound);
			//遍历tile
			for(Point2i pixel:tileBound) {
				localSampler->StartPixel(pixel); //开始一个像素
				//检查像素是否在积分器负责的范围内
				//这里把检查放在StartPixel后面v3有个解释：
				// Do this check after the StartPixel() call; this keeps
				// the usage of RNG values from (most) Samplers that use
				// RNGs consistent, which improves reproducability /
				// debugging.
				if(!InsideExclusive(pixel,_pixelBound)) {
					continue;
				}
				do {
					//获取相机样本
					CameraSample cs = localSampler->GetCameraSample(pixel);
					//生成射线
					RayDifferential ray;
					Float rWeight=_camera->GenerateRayDifferential(cs, &ray);
					//根据每个像素中包含的样本数，缩放射线微分值
					ray.ScaleRayDifferential(1.0f/std::sqrt((Float)localSampler->samplesPerPixel));
					Spectrum L(0.0f);//总共的radiance之和
					if(rWeight>0.0f){
						L=Li(ray,scene,*localSampler,arena);
					}
					filmTile->AddSample(pixel, L, rWeight);
					arena.Reset();
				}while (localSampler->StartNextSample());
			}
			//合并tile
			_camera->film->MergeFilmTile(std::move(filmTile));
			reporter.Update();
//<<并行循环体结束>>
		}, numTile);
		reporter.Done();
		_camera->film->WriteImage();
}


Spectrum UniformSampleAllLights(const Interaction&it, const Scene& scene, MemoryArena &arena, Sampler &sampler,
	const std::vector<int> &nLightSamples,
	bool handleMedia) {
	Spectrum L(0);
	for (int i = 0; i < scene.lights.size(); ++i) {
		const std::shared_ptr<Light>& light = scene.lights[i];
		int numSamples = nLightSamples[i];//获得当前光源的样本个数
		//获取样本数组
		Point2f* lightArray = sampler.Get2DArray(numSamples);
		Point2f* scatteringArray=sampler.Get2DArray(numSamples);
		//没有样本数组的情况
		if (!lightArray || !scatteringArray) {
			Point2f lightSample = sampler.Get2DSample();
			Point2f scatteringSample = sampler.Get2DSample();
			L += EstimateDirect(it, scatteringSample,*light,lightSample,scene,sampler,arena,handleMedia);
		}
		else {
			Spectrum Ld(0.0f);
			for (int j = 0; j < numSamples; ++j) {
				Point2f lightSample = lightArray[j];
				Point2f scatteringSample = scatteringArray[j];
				Ld += EstimateDirect(it, scatteringSample, *light, lightSample, scene, sampler, arena, handleMedia);
			}
			L = Ld / numSamples;
		}
	}
	return L;
}


Spectrum UniformSampleOneLight(const Interaction&it, const Scene& scene, MemoryArena &arena, Sampler &sampler,
	bool handleMedia) {
	int numLights = scene.lights.size();
	if (numLights == 0) {
		return Spectrum(0.0);
	}
	//选择一个光源
	int lightIndex = std::min((int)sampler.Get1DSample()*numLights, numLights - 1);
	Float lightPdf = 1.0 / numLights;

	std::shared_ptr<Light> light = scene.lights[lightIndex];

	Point2f lightSample = sampler.Get2DSample();
	Point2f  scatteringSample = sampler.Get2DSample();

	return EstimateDirect(it, scatteringSample, *light, lightSample, scene, sampler, arena, handleMedia);
}

Spectrum EstimateDirect(const Interaction &it, const Point2f &uScattering,
	const Light &light, const Point2f &uLight,
	const Scene &scene, Sampler &sampler,
	MemoryArena &arena, bool handleMedia, bool specular) {
	//1.先采样光源,delta光源不需要参与MIS
	//2.然后采样BSDF，如果光源是delta,不需要采样BSDF，因为肯定采样不到光源

	//首先判断需不需要考虑采样 specular lobe
	BxDFType bsdfFlags = specular ? (BSDF_ALL) : BxDFType(BSDF_ALL&~BSDF_SPECULAR);
	Spectrum Ld(0);
	Vector3f wi;
	Float lightPdf = 0;
	Float scatteringPdf = 0;
	VisibilityTester vis;
	//采样光源MIS
	Spectrum Li=light.Sample_Li(it,uLight,&wi,&lightPdf,&vis);
	if (lightPdf > 0&& !Li.IsBlack()) {
		Spectrum f;
		if (it.IsSurfaceInteraction()) {
			const SurfaceInteraction& si = (const SurfaceInteraction&)(it);
			f=si.bsdf->f(si.wo, wi, bsdfFlags)*AbsDot(wi,si.shading.n);//隐式包含了cos成分
			scatteringPdf = si.bsdf->Pdf(si.wo,wi, bsdfFlags);
		}
		else {
			const MediumInteraction& mi = (const MediumInteraction&)(it);
			Float p = mi.phase->P(mi.wo, wi);
			Assert(!std::isnan(p));
			f = Spectrum(p);
			scatteringPdf = p;
		}
		if (!f.IsBlack()) {
			if (handleMedia) {
				//考虑介质对光源能量的衰减
				Li=Li*vis.Tr(scene, sampler);
			}
			else {
				//判断光源和射线是否倍遮挡
				if (!vis.Unoccluded(scene)) {
					Li = 0.0f;//设置光源能量为0
				}
			}

			if (!Li.IsBlack()) {
				Float weight = 1.0f;
				//计算MIS，如果是delta光源忽略MIS
				if (!IsDeltaLight(light.flags)) {
					weight = PowerHeuristic(1, lightPdf, 1, scatteringPdf);
				}
				Ld += f*Li*weight / lightPdf;

				//PBRT的实现:
				////光源如果是delta光源的话，就不需要考虑MIS了
				//if (IsDeltaLight(light.flags)) {
				//	Ld += f*Li / lightPdf;
				//}
				////否则计算MIS
				//else {
				//	Float weight = PowerHeuristic(1, lightPdf, 1, scatteringPdf);
				//	Ld += f*Li*weight/lightPdf;
				//}
				//PBRT的实现 END
			}
		}
	}

	//采样BSDF MIS
	if (!IsDeltaLight(light.flags)) {
		Spectrum f;
		bool sampledSpecular=false;
		BxDFType sampledType;
		if (it.IsSurfaceInteraction()) {
			const SurfaceInteraction& si = (const SurfaceInteraction&)(it);
			f=si.bsdf->Sample_f(si.wo, &wi, uScattering, &scatteringPdf, bsdfFlags, &sampledType);
			f *= AbsDot(wi, si.shading.n);//隐式cos
			sampledSpecular = (BSDF_SPECULAR&sampledType) != 0;
		}
		else {
			const MediumInteraction& mi = (const MediumInteraction&)(it);
			Float p = mi.phase->Sample_p(mi.wo,&wi, uScattering);
			Assert(!std::isnan(p));
			f = Spectrum(p);
			scatteringPdf = p;
		}

		if (!f.IsBlack() && scatteringPdf > 0) {
			Float weight = 1.0f;
			//如果bsdf不是specular的情况
			if (!sampledSpecular) {
				lightPdf = light.Pdf_Li(it, wi);//计算采样到light的概率
				//如果lightPdf==0,那当前光源不可能产生能量给当前方向
				if (lightPdf == 0) {
					return Ld;
				}
				//计算权重
				weight = PowerHeuristic(1, scatteringPdf, 1, lightPdf);
			}

			//在计算好权重以及确定光源不是delta光源后，计算光源产生的Li
			SurfaceInteraction lightIsect;
			Ray ray = it.SpawnRay(wi);
			//这边活该需要考虑MP带来的影响
			Spectrum Tr(1.0f);
			bool found =  handleMedia?scene.IntersectTr(ray,sampler,&lightIsect,&Tr):scene.Intersect(ray, &lightIsect);
			//能相交就是arealight
			if (found) {
				if (lightIsect.primitive->GetAreaLight() == &light) {
					Li = lightIsect.Le(-wi);
				}
			}
			else {//infi light
				Li = light.Le(ray);
			}
			if (!Li.IsBlack()) {
				//Tr是MP对能量的衰减
				Ld += f*Li*Tr*weight/ scatteringPdf;
			}
		}
	}
	//返回经过MIS加权计算的direct radiacne
	return Ld;
}



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
		auto a=(AbsDot(ref.shading.n, wo) * AbsDot(ref.n, wi));
		auto b=(AbsDot(ref.n, wo) * AbsDot(ref.shading.n, wi));
		auto correct=a/b; 
		if(b==0){
			return 0;
		}
		assert(!std::isnan(correct));
		return correct;
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
			ray=si.SpawnRay(wi);
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

	path[0] = Vertex::CreateCamera(&camera,ray,beta);

	return RandomWalk(scene,ray,sampler,arena,beta,dirPdf, maxDepth-1,TransportMode::Radiance,path+1)+1;
}

//生成光源SubPath
int GenerateLightSubPath(const Scene& scene, Sampler& sampler, MemoryArena& arena,const Distribution1D& lightDis,Float time, int maxDepth, Vertex* path) {
	//采样光源
	Float pdfChoice,pdfDir, pdfPos;
	RayDifferential ray;
	Normal3f normal;
	int index=lightDis.SampleDiscrete(sampler.Get1DSample(),&pdfChoice);
	const std::shared_ptr<Light> light= scene.lights[index];
	Spectrum Le= light->Sample_Le(sampler.Get2DSample(), sampler.Get2DSample(),time,&ray,&normal,&pdfPos,&pdfDir);
	if (pdfChoice == 0 || pdfDir == 0 || pdfPos == 0 || Le.IsBlack()) {
		return 0;
	}

	//生成节点，并且执行RandomWalk
	//注意第一个节点的beta和pdf,以及传入RandomWalk中的beta和pdf
	path[0] = Vertex::CreateLight(EndpointInteraction(light.get(),ray,normal), Le, pdfChoice*pdfPos);
	auto beta = Le*AbsDot(ray.d, normal) / (pdfChoice*pdfDir*pdfPos);
	int numVertices = RandomWalk(scene,ray,sampler,arena,beta,pdfDir, maxDepth-1,TransportMode::Importance,path+1);
	//TODO InfiniteLight相关
	return numVertices + 1;
}

//几何因子函数
Spectrum  G(const Scene& scene, Sampler& sampler,const Vertex& v1,const Vertex& v2){
	auto d=v1.p()-v2.p();
	Float g=1.0/d.LengthSquared();
	d*=g;//标准化
	if(v1.IsOnSurface()){
		g*=AbsDot(v1.ns(),d);
	}
	if(v2.IsOnSurface()){
		g*=AbsDot(v2.ns(),d);
	}
	VisibilityTester tester=VisibilityTester(v1.GetInteraction(),v2.GetInteraction());
	return g*tester.Tr(scene,sampler);
}

//链接两个子路径并且计算能量
Spectrum ConnectBDPT(const Scene& scene,Vertex* lightVertices,Vertex* cameraVertices,int s,int t,Sampler& sampler,const Distribution1D& lightDistri,const std::unordered_map<const Light *, size_t>& lightToIndex, const Camera& camera, Point2f*raster){
	
	Vertex sampled;//额外采样的vertex
	Spectrum L;
	//一条完整的路径的情况
	if(s==0){
		Vertex& cp=cameraVertices[t-1];
		if(cp.IsLight()){
			L=cp.Le(scene,cameraVertices[t-2])*cp.beta;
		}
	}
	//有一个光源的情况
	// direct情况
	else if(s==1){
		Vertex& cp=cameraVertices[t-1];
		if(cp.IsConnectable()){
			Float lightPdf;
			int index=lightDistri.SampleDiscrete(sampler.Get1DSample(),&lightPdf);
			auto light=scene.lights[index];
			Vector3f wi;
			Float pdf;
			VisibilityTester vis;
			auto Le=light->Sample_Li(cp.GetInteraction(),sampler.Get2DSample(),&wi,&pdf,&vis);
			if(pdf>0&&!Le.IsBlack()){
				EndpointInteraction lp=EndpointInteraction(vis.P1(),light.get());
				sampled=Vertex::CreateLight(lp,Le/(lightPdf*pdf),0);
				sampled.pdfFwd=sampled.PdfLightOrigin(scene,cp,lightDistri,lightToIndex);
				L=cp.beta*cp.f(sampled,TransportMode::Radiance)*sampled.beta;
				if(cp.IsOnSurface()){
					L=L*AbsDot(wi,cp.ns());
				}
				if(!L.IsBlack()){
					//只有当camera subpath和light都提供能量的时候，再判断是否遮挡
					L=L*vis.Tr(scene,sampler);
				}
			}
			
		}
	}
	//Camera SubPath只包含一个顶点
	else if(t == 1){
		Vertex& lp = lightVertices[s - 1];
		if (lp.IsConnectable()) {
			Vector3f wi;
			Float pdf;
			VisibilityTester vis;
			auto we=camera.Sample_Wi(lp.GetInteraction(),sampler.Get2DSample(),&wi,&pdf, raster,&vis);
			if (pdf != 0 && !we.IsBlack()) {
				//生成相应的相机点，并且计算相应的贡献
				//这里要注意相应的beta的计算
				EndpointInteraction ep = EndpointInteraction(vis.P1(),&camera);
				sampled = Vertex::CreateCamera(ep, &camera,we/ pdf);
				L = lp.beta*lp.f(sampled,TransportMode::Importance)*sampled.beta;
				if (lp.IsOnSurface()) {
					L = L*AbsDot(wi, lp.ns());
				}
				if (!L.IsBlack()) {
					L = L*vis.Tr(scene, sampler);
				}
			}
		}
	}
	else if(s>1&&t>1){
		Vertex& lp=lightVertices[s-1];
		Vertex& cp=cameraVertices[t-1];
		if(lp.IsConnectable()&&cp.IsConnectable()){
			L=lp.beta*lp.f(cp,TransportMode::Importance)*cp.f(lp,TransportMode::Radiance)*cp.beta;
			if(!L.IsBlack()){
				L=L*G(scene,sampler,lp,cp);
			}
		}
	}

	return L;
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

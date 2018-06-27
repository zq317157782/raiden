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
#include "filters/box.h"


//临时修改某个指针内的值的Helper类
template<typename type>
class ScopedAssignment {
private:
	type* _target; //目标指针
	type _backup;  //备份
public:
	ScopedAssignment(type* target=nullptr, type value = type()):_target(target){
		if (target) {
			_backup = (*target);
			*_target = value;
		}
	}

	~ScopedAssignment() {
		if (_target) {
			*_target = _backup;
		}
	}
	ScopedAssignment(const ScopedAssignment&) = delete;
	ScopedAssignment& operator=(const ScopedAssignment&) = delete;
	ScopedAssignment& operator=(ScopedAssignment&& right) {
		_target = right._target;
		_backup = right._backup;
		right._target = nullptr;
		return (*this);
	}
};

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
				if(mode==TransportMode::Radiance){
					vertex=Vertex::CreateLight(EndpointInteraction(ray),beta,pdfFwd);
					++bounces;
				}
				break;
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
		//LInfo << preV.pdfRev<<" "<< pdfRev;
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
	
	if (path[0].IsInfiniteLight()) {
		//除了path[0]外还有其他的LightSubpath顶点
		if (numVertices > 0) {
			//更新Path[1]的fwdPdf
			path[1].pdfFwd = pdfPos;
			//做投影衰减
			if (path[1].IsOnSurface()) {
				//这里为啥不是ns呢，需要考虑下
				path[1].pdfFwd *= AbsDot(ray.d, path[1].ng());
			}
		}

		path[0].pdfFwd = InfiniteLightDensity(scene, lightDis, ray.d);
	}

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

//计算相应的MIS系数的函数
//使用的是Balance启发
Float MISWeight(const Scene&scene, Vertex* lightVertices, Vertex* cameraVertices, int s, int t,Vertex& sampled, const Distribution1D& lightDistri, const std::unordered_map<const Light *, size_t>& lightToIndex) {
	//各只有一个顶点的情况下，就只有一种策略
	if (s + t == 2) {
		return 1;
	}

	//获取相应的顶点，以及前面的顶点
	Vertex* lp = s > 0 ? &lightVertices[s - 1] : nullptr;
	Vertex* cp = t > 0 ? &cameraVertices[t - 1] : nullptr;
	Vertex* lpPre = s > 1 ? &lightVertices[s - 2] : nullptr;
	Vertex* cpPre = t > 1 ? &cameraVertices[t - 2] : nullptr;

	//先判断是否需要使用重新采样的顶点来代替原来的顶点
	//只发生在s==1或者t==1的情况下
	ScopedAssignment<Vertex> a1;
	if (s == 1) {
		a1 = { lp,sampled };
	}
	else if (t == 1) {
		a1 = { cp,sampled };
	}


	ScopedAssignment<bool> a2,a3;
	if (lp) {
		a2 = { &lp->delta,false };
	}
	if (cp) {
		a3 = { &cp->delta,false };
	}
	
	ScopedAssignment<Float> a4,a5;
	//开始修正相应的pdf
	if (lp) {
		//这里不可能出现t==0的情况，所以不需要考虑
		a4 = { &lp->pdfRev,cp->Pdf(scene,cpPre,*lp) };
	}
	if (lpPre) {
		a5 = { &lpPre->pdfRev,lp->Pdf(scene,cp,*lpPre) };
	}
	ScopedAssignment<Float> a6, a7;
	if (cp) {
		a6 = { &cp->pdfRev,s > 0 ? lp->Pdf(scene,lpPre,*cp) : cp->PdfLightOrigin(scene,*cpPre,lightDistri,lightToIndex) };
	}
	if (cpPre) {
		a7 = { &cpPre->pdfRev,s > 0 ? cp->Pdf(scene,lp,*cpPre) : cp->PdfLight(scene,*cpPre) };
	}
	//开始使用简化后的公式，计算mis系数
	Float sumRi = 0;
	auto remap0 = [](Float f)->Float {return f == 0 ? 1 : f; };
	Float ri = 1;
	for (int i = s-1; i >= 0; --i) {
		ri *= remap0(lightVertices[i].pdfRev) / remap0(lightVertices[i].pdfFwd);
		bool deltaLightVertex = i > 0 ? lightVertices[i - 1].delta : lightVertices[0].IsDeltaLight();
		if (!lightVertices[i].delta && !deltaLightVertex) {
			sumRi += ri;
		}
	}
	ri = 1;
	for (int i = t - 1; i > 0; --i) {
		ri *= remap0(cameraVertices[i].pdfRev) / remap0(cameraVertices[i].pdfFwd);
		if (!cameraVertices[i].delta && !cameraVertices[i - 1].delta) {
			sumRi += ri;
		}
	}

	return 1 / (sumRi + 1);
}

//链接两个子路径并且计算能量
Spectrum ConnectBDPT(const Scene& scene,Vertex* lightVertices,Vertex* cameraVertices,int s,int t,Sampler& sampler,const Distribution1D& lightDistri,const std::unordered_map<const Light *, size_t>& lightToIndex, const Camera& camera, Point2f*raster){
	
	Vertex sampled;//额外采样的vertex
	Spectrum L;

	//额外判断t是Infinite光源的情况
	if (t > 1 && s != 0 && cameraVertices[t - 1].type == VertexType::Light) {
		return 0;
	}

	//一条完整的路径的情况
	if(s==0){
		Vertex& cp=cameraVertices[t-1];
		if(cp.IsLight()){
			L=cp.Le(scene,cameraVertices[t-2])*cp.beta;
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
				EndpointInteraction ep = EndpointInteraction(vis.P0(),&camera);
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
	//有一个光源的情况
	// direct情况
	else if (s == 1) {
		Vertex& cp = cameraVertices[t - 1];
		if (cp.IsConnectable()) {
			Float lightPdf;
			int index = lightDistri.SampleDiscrete(sampler.Get1DSample(), &lightPdf);
			auto light = scene.lights[index];
			Vector3f wi;
			Float pdf;
			VisibilityTester vis;
			auto Le = light->Sample_Li(cp.GetInteraction(), sampler.Get2DSample(), &wi, &pdf, &vis);
			if (pdf>0 && !Le.IsBlack()) {
				EndpointInteraction lp = EndpointInteraction(vis.P1(), light.get());
				sampled = Vertex::CreateLight(lp, Le / (lightPdf*pdf), 0);
				sampled.pdfFwd = sampled.PdfLightOrigin(scene, cp, lightDistri, lightToIndex);
				L = cp.beta*cp.f(sampled, TransportMode::Radiance)*sampled.beta;
				if (cp.IsOnSurface()) {
					L = L*AbsDot(wi, cp.ns());
				}
				if (!L.IsBlack()) {
					//只有当camera subpath和light都提供能量的时候，再判断是否遮挡
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
	
	if (!L.IsBlack()) {
		Float mis=MISWeight(scene, lightVertices, cameraVertices,s,t,sampled,lightDistri,lightToIndex);
		assert(!IsNaN(mis));
		L = L*mis;
	}

	return L;
}


inline int BufferIndex(int s, int t) {
	int above = s + t - 2;
	return s + above * (5 + above) / 2;
}

void BDPTIntegrator::Render(const Scene& scene){
	//初始化光源分布
	_lightDistribution = ComputeLightSampleDistribution(_lightStrategy, scene);
	//从Light指针到index的映射
	std::unordered_map<const Light *, size_t> lightToIndex;
	for (size_t i = 0; i < scene.lights.size(); ++i) {
		lightToIndex[scene.lights[i].get()] = i;
	}

	//获得样本的范围
	auto sampleBounds = _camera->film->GetSampleBounds();
	auto sampleExtent = sampleBounds.Diagonal();//获得宽高
	int tileSize = 16;//tile的宽和高的大小
	int nTileX = (sampleExtent.x + tileSize - 1) / tileSize;
	int nTileY = (sampleExtent.y + tileSize - 1) / tileSize;
	Point2i tileNum = Point2i(nTileX, nTileY);
	ProgressReporter reporter(tileNum.x*tileNum.y, "Rendering");
	
	
	//生成调试用的Films
	//这个count是怎么计算出来的还得研究研究
	const int bufferCount = (1 + _maxDepth)*(6 + _maxDepth) / 2;
	std::vector<std::unique_ptr<Film>> weightFilms(bufferCount);
	//Debug代码
	{
		for (int depth = 0; depth <= _maxDepth; ++depth) {
			for (int s = 0; s <= depth + 2; ++s) {
				int t = depth + 2 - s;
				if (t == 0 || (s == 1 && t == 1)) {
					continue;
				}

				std::ostringstream stringStream;
				stringStream << "bdpt_d" << depth << "_s" << s << "_t" << t << ".png";
				std::string filename = stringStream.str();

				weightFilms[BufferIndex(s, t)] = std::unique_ptr<Film>(new Film(
					_camera->film->fullResolution,
					Bound2f(Point2f(0, 0), Point2f(1, 1)),
					std::unique_ptr<Filter>(CreateBoxFilter(ParamSet())), filename, 1.0f));
			}
		}
	}

	//<并行循环体,循环tile>
	//
	ParallelFor2D([&](const Point2i& tile) {

		//计算每个tile的seed
		int seed = tile.x + tileNum.x*tile.y;
		//根据seed,克隆Sampler
		auto tileSampler = _sampler->Clone(seed);

		//计算当前tile所覆盖的sampleBounds
		int x0 = sampleBounds.minPoint.x + tile.x*tileSize;
		int y0 = sampleBounds.minPoint.y + tile.y*tileSize;

		int x1 = std::min(x0 + tileSize, sampleBounds.maxPoint.x);
		int y1 = std::min(y0 + tileSize, sampleBounds.maxPoint.y);

		Bound2i tileSampleBounds = Bound2i(Point2i(x0, y0), Point2i(x1, y1));

		//根据tileSampleBounds获得FilmTile
		auto filmTile = _camera->film->GetFilmTile(tileSampleBounds);
		MemoryArena arena;

		//<循环体,循环pixel>
		for (auto pixel : tileSampleBounds) {
			//针对每个像素做处理
			tileSampler->StartPixel(pixel);

			//检查像素是否在积分器负责的范围内
			//这里把检查放在StartPixel后面v3有个解释：
			// Do this check after the StartPixel() call; this keeps
			// the usage of RNG values from (most) Samplers that use
			// RNGs consistent, which improves reproducability /
			// debugging.
			if (!InsideExclusive(pixel, _pixelBound)) {
				continue;
			}

			//<循环体,循环样本点>
			do
			{
				//当前的film样本点
				auto filmPos = (Point2f)pixel + tileSampler->Get2DSample();

				//分配两个数组，分别存放Camera subpath和Light subpath
				//长度都是maxDepth+1,因为maxDepth代表边长，所以顶点要多一个，然后相机额外还要多一个存放和光源相交的顶点
				Vertex* cameraVertices = arena.Alloc<Vertex>(_maxDepth + 2);
				Vertex* lightVertices = arena.Alloc<Vertex>(_maxDepth + 1);

				//生成两条subpath
				int nCamera = GenerateCameraSubPath(scene, *tileSampler, arena, *_camera, _maxDepth + 2, filmPos, cameraVertices);
				//PBRT这里使用了PowerDistribution
				const Distribution1D *lightDistr = _lightDistribution->Lookup(cameraVertices[0].p());

				int nLight = GenerateLightSubPath(scene, *tileSampler, arena, *lightDistr, cameraVertices[0].time(), _maxDepth + 1, lightVertices);

				//遍历所有的SubPath顶点，并且计算相应的连接下的FullPath的贡献
				//相机不需要考虑t==0的情况，因为不考虑LightPath的EndPoint是Lens的情况
				Spectrum L(0);
				Point2f raster;
				for (int t = 1; t <= nCamera; ++t) {
					for (int s = 0; s <= nLight; ++s) {
						//TODO 和PT的Depth貌似有区别，需要再研究研究  
						int depth = s + t - 2;
						//跳过两个SubPath都只有一个顶点的情况，以及深度越界的情况
						if ((s == 1 && t == 1) || depth<0 || depth>_maxDepth) {
							continue;
						}
						//计算相应的FullPath的贡献，并且做记录
						auto radiance = ConnectBDPT(scene, lightVertices, cameraVertices, s, t, *tileSampler, *lightDistr, lightToIndex, *_camera, &raster);
						
						//Debug代码
						{
							Spectrum value = radiance;
							
							weightFilms[BufferIndex(s, t)]->AddSplat(raster,value);
						}
						
						if (t == 1) {
							//只包含1个相机点
							_camera->film->AddSplat(raster, radiance);
						}
						else {
							L += radiance;
						}

					}
				}
				filmTile->AddSample(filmPos, L, 1);
				//重置当前路径样本所依赖的空间
				arena.Reset();
			} while (tileSampler->StartNextSample());
		}
		//合并filmTile
		_camera->film->MergeFilmTile(std::move(filmTile));
		reporter.Update();
	}, tileNum);
	reporter.Done();
	//这里需要传入采样率，来调整splat进去的能量的权重
	_camera->film->WriteImage(1.0 / _sampler->samplesPerPixel);

	//DEBUG代码
	{
		const Float invSampleCount = 1.0f / _sampler->samplesPerPixel;
		for (size_t i = 0; i < weightFilms.size(); ++i) {
			if (weightFilms[i]) {
				weightFilms[i]->WriteImage(invSampleCount);
			}
		}
	}
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

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
#include <unordered_map>
#include "lightdistrib.h"
#include "parallel.h"
#include "sampler.h"
#include "memory.h"
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
	EndpointInteraction(const Camera* cam, const Ray& ray) :
			Interaction(ray.o, ray.time, ray.medium), camera(cam) {
	}
	EndpointInteraction(const Interaction& interaction, const Camera* cam) :
			Interaction(interaction), camera(cam) {
	}
	//初始化代表光源的点
	EndpointInteraction(const Light* _light, const Ray& ray, const Normal3f& normal) :
			Interaction(ray.o, ray.time, ray.medium), light(_light) {
		this->n = normal; //这里是离开光源的射线所在的点所在的表面的法线
	}
	//这里对应的光源表面的法线信息已经在interaction中了，所以不需要显示提供法线
	EndpointInteraction(const Interaction& interaction, const Light* _light) :
			Interaction(interaction), light(_light) {
	}
};

//详见Veach的论文[Robust Monte Carlo Methods for Light Transport],解释得非常到位
//只有的表面交点的情况下，才会出现这种情况
Float CorrectShadingNormal(const SurfaceInteraction& ref, const Vector3f& wo,
		const Vector3f& wi, TransportMode mode);



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
	//area measure
	Float pdfFwd = 0;	//采样这个点的概率
	Float pdfRev = 0;	//逆向采样这个点的概率

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

	//返回time
	Float time() const {
		return GetInteraction().time;
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
			LWarning<<"Unimplement for this type!";
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
			//这里没有判断Specular成分是否存在
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
		if (type == VertexType::Light && ei.light
				&& ::IsDeltaLight(ei.light->flags)) {
			return true;
		}
		return false;
	}

	//判断是否是InfiniteLight
	bool IsInfiniteLight() const {
		return type == VertexType::Light && (ei.light == nullptr || (ei.light->flags&(int)LightFlags::Infinite) != 0);
	}

	//获取这个点的自发光能量
	Spectrum Le(const Scene& scene, const Vertex& v) const {
		//先判断是否是光源
		if (!IsLight()) {
			return Spectrum(0);
		}
		Vector3f w = v.p() - p();
		if (w.LengthSquared() == 0) {
			return Spectrum(0);
		}
		w = Normalize(w);
		if (IsInfiniteLight()) {
			Spectrum Le(0);
			for (auto& light : scene.lights) {
				Le = Le + light->Le(Ray(p(), -w));
			}
			return Le;
		}
		else {
			const AreaLight* light = si.primitive->GetAreaLight();
			return light->L(si, w);
		}
	}

	//从立体角pdf转变到area pdf
	Float ConvertDensity(Float pdf, const Vertex& next) const {
		//TODO InfiniteLight 相关
		Vector3f w = p() - next.p();
		if (w.LengthSquared() == 0) {
			return 0;
		}
		Float invRadius2 = 1.0 / (w.LengthSquared());
		if (next.IsOnSurface()) {
			pdf = pdf * Dot(w * std::sqrt(invRadius2), next.ng());
		}
		return pdf * invRadius2;
	}

	//采样一个光源到另外一个vertex的概率

	Float PdfLight(const Scene& scene, const Vertex& v) const {

		Vector3f w = v.p() - p();
		Float invLengthSquared = 1.0 / w.LengthSquared();
		w = w * std::sqrt(invLengthSquared);	//标准化
		
		if (IsInfiniteLight()) {
			//TODO InfiniteLight 相关
			return 0;
		}
		else {
			Assert(IsLight());		//首先判断当前Vetex是否是光源
			const Light* light = nullptr;		//初始化指向光源的指针
			if (type == VertexType::Light) {
				light = ei.light;
			}
			else {
				light = si.primitive->GetAreaLight();		//区域光情况
			}
			Assert(light != nullptr);		//判断光源不为空
			Float pdfPos, pdfDir;		//(立体角)
			light->Pdf_Le(Ray(p(), w, time()), ng(), &pdfPos, &pdfDir);
			//转换到area度量
			Float pdf = pdfDir * invLengthSquared;		//(area)
			if (IsOnSurface()) {
				pdf *= AbsDot(w, v.ng());		//回忆起几何衰减系数
			}
			return pdf;
		}
	}

	Float PdfLightOrigin(const Scene& scene, const Vertex& v,
			const Distribution1D& distrib,
			const std::unordered_map<const Light *, size_t> &lightToDistrIndex) const {
		Vector3f w = v.p() - p();
		if (w.LengthSquared() == 0) {
			return 0;
		}
		w = Normalize(w);
		if (IsInfiniteLight()) {
			//TODO InfiniteLight 相关
		}
		else {
			Assert(IsLight());//首先判断当前Vetex是否是光源
			const Light* light = nullptr;//初始化指向光源的指针
			if (type == VertexType::Light) {
				light = ei.light;
			}
			else {
				light = si.primitive->GetAreaLight();//区域光情况
			}
			Assert(light != nullptr);		//判断光源不为空

			int index = lightToDistrIndex.find(light)->second;
			Float pdfChoice = distrib.DiscretePDF(index);//选中这个光源的概率
			Float pdfPos, unused;
			light->Pdf_Le(Ray(p(), w, time()), ng(), &pdfPos, &unused);
			//这里只考虑位置，不考虑方向
			return pdfChoice*pdfPos;
		}
	}

	Float Pdf(const Scene& scene, const Vertex* pre, const Vertex& next) const {
		if (type == VertexType::Light) {
			return PdfLight(scene, next);
		}

		//指向next的向量
		Vector3f wn = next.p() - p();
		if (wn.LengthSquared() == 0) {
			return 0;
		}
		wn = Normalize(wn);

		//指向pre的向量
		Vector3f wp;
		if (pre) {
			wp = pre->p() - p();
			if (wp.LengthSquared() == 0) {
				return 0;
			}
			wp = Normalize(wp);
		} else {
			//pre不存在的情况只有Camera,因为Light在上一步已经处理了
			Assert(type == VertexType::Camera);
		}

		Float pdf = 0, unused;		//unused!PUPUPU!
		if (type == VertexType::Camera) {
			ei.camera->Pdf_We(ei.SpawnRay(wn), &unused, &pdf);		//采样重要性的概率
		} else if (type == VertexType::Surface) {
			pdf = si.bsdf->Pdf(wp, wn);
		} else if (type == VertexType::Medium) {
			pdf = mi.phase->P(wp, wn);
		} else {
			Error("Unimplemented in Vertex::Pdf!");
		}
		//转换到area measurement
		return ConvertDensity(pdf, next);
	}

	static inline Vertex CreateMedium(const MediumInteraction& mi, const Spectrum& beta, Float pdf, const Vertex& prev);
	static inline Vertex CreateSurface(const SurfaceInteraction& si, const Spectrum& beta, Float pdf, const Vertex& prev);
	static inline Vertex CreateLight(const EndpointInteraction& ei, const Spectrum& beta, Float pdf);
	static inline Vertex CreateLight(const Light* light,const Ray& ray,const Normal3f& n, const Spectrum& Le, Float pdf);
	static inline Vertex CreateCamera(const Camera* camera,const Ray& ray,const Spectrum& beta);
	static inline Vertex CreateCamera(const Interaction& it, const Camera* camera, const Spectrum& beta);
};

//基于medium创建vertex
inline Vertex Vertex::CreateMedium(const MediumInteraction& mi,const Spectrum& beta,Float pdf,const Vertex& prev){
	Vertex v(mi,beta);
	//转换立体角pdf到area pdf
	v.pdfFwd=prev.ConvertDensity(pdf,v);
	return v;
}

//基于surface创建vertex
inline Vertex Vertex::CreateSurface(const SurfaceInteraction& si, const Spectrum& beta, Float pdf, const Vertex& prev){
	Vertex v(si, beta);
	//转换立体角pdf到area pdf
	v.pdfFwd = prev.ConvertDensity(pdf, v);
	return v;
}

inline Vertex Vertex::CreateLight(const EndpointInteraction& ei,const Spectrum& beta, Float pdf) {
	Vertex v(VertexType::Light,ei,beta);
	v.pdfFwd = pdf;
	return v;
}

inline Vertex CreateLight(const Light* light, const Ray& ray, const Normal3f& n, const Spectrum& Le, Float pdf) {
	Vertex v(VertexType::Light, EndpointInteraction(light, ray,n), Le);
}

inline Vertex Vertex::CreateCamera(const Camera* camera, const Ray& ray, const Spectrum& beta){
	
	return Vertex(VertexType::Camera,EndpointInteraction(camera,ray), beta);
}

inline Vertex Vertex::CreateCamera(const Interaction& it, const Camera* camera, const Spectrum& beta){
	return Vertex(VertexType::Camera, EndpointInteraction(it,camera), beta);
};

int GenerateCameraSubPath(const Scene& scene, Sampler& sampler, MemoryArena& arena, const Camera& camera, int maxDepth, const Point2f& pFilm, Vertex* path);

//双向路径追踪
class BDPTIntegrator : public Integrator {
private:
	std::shared_ptr<const Camera> _camera;
	std::shared_ptr<Sampler> _sampler;
	const int _maxDepth;
	const Bound2i _pixelBound;
public:
	BDPTIntegrator(const std::shared_ptr<const Camera>& camera, std::shared_ptr<Sampler>& sampler,int maxDepth,const Bound2i& pixelBound) :
		_camera(camera), _sampler(sampler), _maxDepth(maxDepth), _pixelBound(pixelBound) {
	}
	virtual void Render(const Scene& scene) override {
		//获得样本的范围
		auto sampleBounds = _camera->film->GetSampleBounds();
		auto sampleExtent = sampleBounds.Diagonal();//获得宽高
		int tileSize = 16;//tile的宽和高的大小
		int nTileX = (sampleExtent.x + tileSize - 1) / tileSize;
		int nTileY = (sampleExtent.y + tileSize - 1) / tileSize;
		Point2i tileNum = Point2i(nTileX, nTileY);
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
					//当前的file样本点
					auto filmPos = (Point2f)pixel + tileSampler->Get2DSample();

					//分配两个数组，分别存放Camera subpath和Light subpath
					//长度都是maxDepth+1,因为maxDepth代表边长，所以顶点要多一个，然后相机额外还要多一个存放和光源相交的顶点
					Vertex* cameraVertices = arena.Alloc<Vertex>(_maxDepth + 2);
					Vertex* lightVertices  = arena.Alloc<Vertex>(_maxDepth + 1);
					
					//生成两条subpath
					int nCamera = GenerateCameraSubPath(scene, *tileSampler,arena,*_camera, _maxDepth + 2,filmPos, cameraVertices);
					int nLight = 0;

					//遍历所有的SubPath顶点，并且计算相应的连接下的FullPath的贡献
					//相机不需要考虑t==0的情况，因为不考虑LightPath的EndPoint是Lens的情况
					Spectrum L(0);
					for (int t = 1; t <= nCamera; ++t) {
						for (int s = 0; s <= nLight; ++s) {
							//TODO 和PT的Depth貌似有区别，需要再研究研究  
							int depth = s + t - 2;
							//跳过两个SubPath都只有一个顶点的情况，以及深度越界的情况
							if ((s == 1 && t == 1) || depth<0 || depth>_maxDepth) {
								continue;
							}
							//计算相应的FullPath的贡献，并且做记录
						}
					}
					filmTile->AddSample(filmPos, L,1);
						
					//重置当前路径样本所依赖的空间
					arena.Reset();
				} while (tileSampler->StartNextSample());
			}

		}, tileNum);

		_camera->film->WriteImage();
	}
};


BDPTIntegrator *CreateBDPTIntegrator(const ParamSet &params,
		std::shared_ptr<Sampler> sampler, std::shared_ptr<const Camera> camera);

#endif /* SRC_INTEGRATORS_BDPT_H_ */

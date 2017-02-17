/*
 * sppm.h
 *
 *  Created on: 2017年2月15日
 *      Author: zhuqian
 */

#ifndef SRC_INTEGRATORS_SPPM_H_
#define SRC_INTEGRATORS_SPPM_H_
#include "raiden.h"
#include "integrator.h"
#include "camera.h"
#include "film.h"
#include "parallel.h"
#include "samplers/random.h"
#include "progressreporter.h"
#include "scene.h"
#include "light.h"
#include "memory.h"
#include "reflection.h"

//PBRT所使用的哈希函数，会发生碰撞
inline unsigned int hash(const Point3i &p, int hashSize) {
	return (unsigned int)((p.x * 73856093) ^ (p.y * 19349663) ^
		(p.z * 83492791)) %
		hashSize;
}

//返回在grid中的索引
static bool ToGrid(const Point3f& p, const Bound3f& gridBound, const int gridRes[3],Point3i* index) {
	bool inBound = true;
	Vector3f offset = gridBound.Offset(p);
	for (int i = 0; i < 3; ++i) {
		(*index)[i] = gridRes[i] * offset[i];
		inBound &= ((*index)[i] >= 0 && (*index)[i] < gridRes[i]);
		(*index)[i] = Clamp((*index)[i], 0, gridRes[i] - 1);
	}
	return inBound;
}

//统计SPPM单个像素信息的结构
struct SPPMPixel {
	Spectrum Ld;//直接光(累积)
	Float radius;//像素的搜索半径
	struct VisiblePoint {
	public:
		Point3f p;//vp位置
		Vector3f wo;//入射方向
		const BSDF* bsdf=nullptr;//BSDF模型
		Spectrum beta;//throughout
	public:
		VisiblePoint() {}
		VisiblePoint(const Point3f& pos,const Vector3f& w,BSDF* bsdf,const Spectrum& beta):p(pos),wo(w),bsdf(bsdf),beta(beta){}
	} vp;
};


//SPPM Linked list node
struct SPPMPixelListNode
{
public:
	SPPMPixel* pixel;
	SPPMPixelListNode* next;
};

//SPPM 随机渐进式光子映射
class SPPMIntegrator:public Integrator{
private:
	std::shared_ptr<const Camera> _camera;
	const int _numIteration;//ray-photon pass个数
	const int _maxDepth; //path的最大深度
	const Float _initRadius;
public:

	SPPMIntegrator(std::shared_ptr<const Camera>& camera,int numIteration,int maxDepth,Float initRadius):_camera(camera),_numIteration(numIteration), _maxDepth(maxDepth), _initRadius(initRadius){
	}

	virtual void Render(const Scene& scene){
		//获取有效的像素范围，然后计算像素个数
		const Bound2i& pixelBound=_camera->film->croppedPixelBound;
		int numPixel=pixelBound.Area();//面积便是像素个数
		Vector2i tileExtent=pixelBound.Diagonal();
		int tileSize=16;//这是pbrt的标配tile大小>_<
		//计算tile的分辨率
		Point2i tileNum=Point2i((tileExtent.x+tileSize-1)/tileSize,(tileExtent.y+tileSize-1)/tileSize);
		Float rayDifferentialScale = 1.0 / std::sqrt(_numIteration);
		//样本生成器
		//todo PBRT中这里使用了Halton Sampler
		RandomSampler sampler(_numIteration);

		std::unique_ptr<SPPMPixel[]> pixels = std::unique_ptr<SPPMPixel[]>(new SPPMPixel[numPixel]);
		for (int i = 0; i < numPixel; ++i) {
			pixels[i].radius = _initRadius;
		}
		//初始化进度条
		ProgressReporter reporter(2*_numIteration,"Rendering");
		for(int iter=0;iter<_numIteration;++iter){
			//首先是 camera passs
			//产生visible point(hit point)的pass


			std::vector<MemoryArena> threadMemoryArens(MaxThreadIndex());
			ParallelFor2D([&](Point2i tile){
				//获取当前tile索引
				int tileIndex=tile.y*tileNum.x+tile.x;
				//克隆样本生成器
				std::unique_ptr<Sampler> tileSampler=sampler.Clone(tileIndex);

				//计算当前tile占据的像素范围
				int x0=pixelBound.minPoint.x+tile.x*tileSize;
				int x1=std::min(x0+tileSize,pixelBound.maxPoint.x);
				int y0=pixelBound.minPoint.y+tile.y*tileSize;
				int y1=std::min(y0+tileSize,pixelBound.maxPoint.y);
				Bound2i tileBound(Point2i(x0,y0),Point2i(x1,y1));
				for(Point2i raster:tileBound){
					
					//设置样本生成器
					tileSampler->StartPixel(raster);
					tileSampler->SetSampleNumber(iter);
					//获取相机样本
					CameraSample cs=tileSampler->GetCameraSample(raster);

					RayDifferential ray;
					SurfaceInteraction ref;
					Spectrum beta=_camera->GenerateRayDifferential(cs,&ray);//生成射线
					ray.ScaleRayDifferential(rayDifferentialScale);
					//获取当前像素对应的SPPMPixel像素
					Vector2i temp =raster- pixelBound.minPoint;
					int pixelOffset = temp.x + temp.y*(pixelBound.maxPoint.x - pixelBound.minPoint.x);
					SPPMPixel& pixel = pixels[pixelOffset];

					bool isSpecular=false;
					//遍历生成路径
					for (int depth = 0; depth < _maxDepth; ++depth) {
						if (!scene.Intersect(ray, &ref)) {
							//没有任何图元相交，返回infinite光源能量
							for (int i = 0; i < scene.lights.size(); ++i) {
								pixel.Ld = pixel.Ld + beta*scene.lights[i]->Le(ray);
							}
							break;
						}
						//到这里是有和图元相交，但是还要判断是否是medium的边界
						MemoryArena& arena = threadMemoryArens[ThreadIndex];//获取当前线程使用的内存区域
						ref.ComputeScatteringFunctions(ray,arena,true);
						//射中medium的边界，重新生成射线
						if (!ref.bsdf) {
							--depth;
							ray = ref.SpawnRay(ray.d);
							continue;
						}
						BSDF& bsdf = *ref.bsdf;//获取当前相交点的BSDF模型
						Vector3f wo = -ray.d;
						//判断是否要考虑自发光
						if (depth == 0 || isSpecular) {
							pixel.Ld = pixel.Ld + beta*ref.Le(wo);
						}
						//采样直接光s
						pixel.Ld = pixel.Ld + beta*UniformSampleOneLight(ref, scene, arena,*tileSampler);
						
						bool isDiffuse = bsdf.NumComponents(BxDFType(BSDF_DIFFUSE | BSDF_REFLECTION | BSDF_TRANSMISSION)) > 0;
						bool isGlossy = bsdf.NumComponents(BxDFType(BSDF_GLOSSY | BSDF_REFLECTION | BSDF_TRANSMISSION)) > 0;
						
						//当交点为diffuse交点 或者是路径最后一个顶点并且是glossy的时候，生成 visible point
						if (isDiffuse || (isGlossy && (depth == _maxDepth - 1))) {
							pixel.vp = { ref.p,ref.wo,&bsdf,beta };
							break;
						} 
						//小于_maxDpeth-1是因为当depth==(_maxDpeth-1)的时候，不需要继续生成新的路径射线了
						if (depth < (_maxDepth - 1)) {
							Vector3f wi;
							Float pdf;
							BxDFType sampledType;
							Spectrum f=bsdf.Sample_f(wo, &wi, tileSampler->Get2DSample(), &pdf, BSDF_ALL, &sampledType);
							if (pdf == 0 || f.IsBlack()) {
								break;
							}
							if (sampledType&BSDF_SPECULAR) {
								isSpecular = true;
							}

							//更新throughout
							beta = beta*f*AbsDot(ref.shading.n, wi) / pdf;
							//俄罗斯罗盘
							if (beta.y() < 0.25f) {
								Float prob = std::min(beta.y(), 1.0f);
								if (tileSampler->Get1DSample() > prob) {
									break;
								}
								beta = beta / prob;
							}
							//生成射线
							ray = (RayDifferential)ref.SpawnRay(wi);
						}
					}
				}
			},tileNum);
			reporter.Update();
			//开始建立grid加速后续photon对vp的访问

			//开始建立grid
			Bound3f gridBound;
			Float maxRadius=0;
			for (int i = 0; i < numPixel; ++i) {
				if (pixels[i].vp.beta.IsBlack()) {
					continue;
				}
				Bound3f pixelBound = Expand(Bound3f(pixels[i].vp.p), pixels[i].radius);
				gridBound = Union(gridBound, pixelBound);
				maxRadius = std::max(maxRadius, pixels[i].radius);
			}

			Vector3f gridDiagonal = gridBound.Diagonal();
			Float maxDiagonal = MaxComponent(gridDiagonal);
			int  gridBaseRes = maxDiagonal / maxRadius;
			Assert(gridBaseRes > 0);
			int gridRes[3];
			for (int i = 0; i < 3; ++i) {
				gridRes[i] = std::max(1,(int)(gridBaseRes*gridDiagonal[i]/maxDiagonal));
			}
			//grid
			std::vector<std::atomic<SPPMPixelListNode*>> grid(numPixel);
			//并行的为grid加节点
			ParallelFor([&](int pixelIndex) {
				//std::cout << pixelIndex << std::endl;
				MemoryArena& arena = threadMemoryArens[ThreadIndex];
				SPPMPixel &pixel = pixels[pixelIndex];
				Point3i pMin,pMax;
				ToGrid(pixel.vp.p - Vector3f(pixel.radius, pixel.radius, pixel.radius), gridBound, gridRes,&pMin);
				ToGrid(pixel.vp.p + Vector3f(pixel.radius, pixel.radius, pixel.radius), gridBound, gridRes, &pMax);
				
				for (int z = pMin.z; z <= pMax.z; ++z) {
					for (int y = pMin.y; y <= pMax.y; ++y) {
						for (int x = pMin.x; x <= pMax.x; ++x) {
							uint32_t index= hash(Point3i(x, y, z), numPixel);
							SPPMPixelListNode* node=arena.Alloc<SPPMPixelListNode>();
							node->pixel = &pixel;
							node->next = grid[index];
							while (!grid[index].compare_exchange_weak(node->next, node));
						}
					}
				}
				//std::cout << pMin << " " << pMax << std::endl;
			}, numPixel, 4096);
			//然后是 photon pass
			reporter.Update();
		}
		reporter.Done();
	}
};

SPPMIntegrator *CreateSPPMIntegrator(const ParamSet &params,
	std::shared_ptr<Sampler> sampler, std::shared_ptr<const Camera> camera);


#endif /* SRC_INTEGRATORS_SPPM_H_ */
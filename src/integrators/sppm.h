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
#include "samplers/halton.h"
#include "progressreporter.h"
#include "scene.h"
#include "light.h"
#include "memory.h"
#include "reflection.h"
#include "lowdiscrepancy.h"

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
	std::atomic<int> M;//累积的光子个数
	AtomicFloat Phi[Spectrum::numSample];//累积的能量
	Float N;
	Spectrum tau;
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

public:
	SPPMPixel() {M=0;}
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
	const int _photonNumPreIteration;//每次追踪的光子数
	const int _writeFrequency;
public:

	SPPMIntegrator(std::shared_ptr<const Camera>& camera,int numIteration,int maxDepth,Float initRadius,int photonNumPreIteration,int writeFrequency):_camera(camera),_numIteration(numIteration), _maxDepth(maxDepth), _initRadius(initRadius),_photonNumPreIteration((photonNumPreIteration>0)? photonNumPreIteration :camera->film->croppedPixelBound.Area()), _writeFrequency(writeFrequency){
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
		HaltonSampler sampler(_numIteration, pixelBound);
		std::unique_ptr<SPPMPixel[]> pixels = std::unique_ptr<SPPMPixel[]>(new SPPMPixel[numPixel]);
		for (int i = 0; i < numPixel; ++i) {
			pixels[i].radius = _initRadius;
		}

		//初始化光源分布,使用基于power的分布
		std::unique_ptr<Distribution1D> lightDistrib=ComputeLightPowerDistribution(scene);
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
						//采样直接光,使用的也是power分布
						pixel.Ld = pixel.Ld + beta*UniformSampleOneLight(ref, scene, arena,*tileSampler,lightDistrib.get());
						
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
				//如果像素的visible point的贡献为0的话，跳过这个像素
				if (pixel.vp.beta.IsBlack()) {
					return;
				}
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
			std::vector<MemoryArena> photonMemoryArenas(MaxThreadIndex());

			ParallelFor([&](int pixelIndex) {
				//获取为光子分配空间的内存区域
				MemoryArena& photonArena = photonMemoryArenas[ThreadIndex];
				
				int haltonDim = 0;
				uint64_t haltonIndex = (uint64_t)_photonNumPreIteration*(uint64_t)iter + pixelIndex;

				//选择光源
				Float lightPdf = 0;
				Float lightSample=RadicalInverse(haltonDim++, haltonIndex);
				//根据Power分布，选择光源
				int lightIndex =lightDistrib->SampleDiscrete(lightSample,&lightPdf);
				std::shared_ptr<Light> light = scene.lights[lightIndex];

				//采样光源的pos和dir
				Point2f lightPosSample(RadicalInverse(haltonDim, haltonIndex), RadicalInverse(haltonDim + 1, haltonIndex));
				Point2f lightDirSample(RadicalInverse(haltonDim + 2, haltonIndex), RadicalInverse(haltonDim + 3, haltonIndex));
				Float time = Lerp(RadicalInverse(haltonDim + 4, haltonIndex), _camera->shutterOpen, _camera->shutterEnd);
				haltonDim += 5;

				RayDifferential photonRay;
				Normal3f nLight;//光源法线
				Float posPdf;//光源Pos pdf
				Float dirPdf;//光源Dir Pdf
				Spectrum Le=light->Sample_Le(lightPosSample, lightDirSample, time, &photonRay, &nLight, &posPdf, &dirPdf);
				if (posPdf == 0 || dirPdf == 0|| Le.IsBlack()) {
					return;
				}
				//光源累计的beta
				Spectrum beta=Le*(AbsDot(nLight,photonRay.d))/(lightPdf*posPdf*dirPdf);
				if (beta.IsBlack()) {
					return;
				}
				SurfaceInteraction ref;
				for (int depth=0; depth < _maxDepth; ++depth) {

					if (!scene.Intersect(photonRay, &ref)) {
						break;
					}

					if (depth > 0) {
						//累计photon的贡献
						Point3i gridPoint;
						if (ToGrid(ref.p, gridBound, gridRes, &gridPoint)) {
							int gridIndex = hash(gridPoint, numPixel);
							for (SPPMPixelListNode* listNode = grid[gridIndex].load(std::memory_order_relaxed); listNode!= nullptr; listNode = listNode->next) {
								SPPMPixel& pixel = *listNode->pixel;
								if (DistanceSquared(pixel.vp.p, ref.p) > pixel.radius*pixel.radius) {
									continue;//不在半径范围内
								}
								Vector3f wi = -photonRay.d;
								//计算贡献
								Spectrum phi = beta*pixel.vp.bsdf->f(pixel.vp.wo,wi);
								//累计贡献
								for (int i = 0; i < Spectrum::numSample; ++i) {
									pixel.Phi[i].Add(phi[i]);
								}
								//更新总计累积光子数
								++pixel.M;
							}
						}
					}
					ref.ComputeScatteringFunctions(photonRay, photonArena, true, TransportMode::Importance);
					//判断射中的是medium interface
					if (!ref.bsdf) {
						photonRay = ref.SpawnRay(photonRay.d);
						--depth;
						continue;
					}

					BSDF& photonBSDF = *ref.bsdf;
					
					//寻找新的photon反射方向
					Vector3f wo = -photonRay.d;
					Vector3f wi;
					Float bsdfPdf;
					BxDFType flags;
					Point2f bsdfSample(RadicalInverse(haltonDim, haltonIndex), RadicalInverse(haltonDim + 1, haltonIndex));
					haltonDim += 2;
					Spectrum f=ref.bsdf->Sample_f(wo,&wi, bsdfSample,&bsdfPdf,BSDF_ALL,&flags);
					if (bsdfPdf == 0 || f.IsBlack()) {
						break;
					}
					Spectrum betaNew = beta*f*AbsDot(ref.shading.n, wi) / bsdfPdf; 

					//俄罗斯罗盘
					Float quitProb=std::max(0.0f, (1 - betaNew.y()/beta.y()));
					if (RadicalInverse(haltonDim++, haltonIndex) > quitProb) {
						beta = betaNew / (1.0f - quitProb);
						photonRay = (RayDifferential)ref.SpawnRay(wi);
					}
					else {
						break;
					}
				}
				photonArena.Reset();
			}, _photonNumPreIteration, 8192);

			//更新Pixel中的数据

			//pbrt中gamma是每个遍历都计算一次，包含除法操作，我觉得这个除法的开销是无用且昂贵的
			constexpr Float gamma = (Float)2 / (Float)3;
			ParallelFor([&](int pixelIndex) {
				SPPMPixel& pixel = pixels[pixelIndex];
				if (pixel.M > 0) {
					Float nNew = pixel.N + pixel.M*gamma;
					Float rNew = pixel.radius*std::sqrt(nNew / (pixel.N + pixel.M));
					Spectrum phi;
					for (int i = 0; i < Spectrum::numSample; ++i) {
						phi[i] = pixel.Phi[i];
					}
					//计算新的累计值
					pixel.tau = (pixel.tau + pixel.vp.beta*phi)*(rNew*rNew) / (pixel.radius*pixel.radius);
					pixel.N = nNew;
					pixel.radius = rNew;
					pixel.M = 0;
					for (int i = 0; i < Spectrum::numSample; ++i) {
						pixel.Phi[i] = 0;
					}
				}
				pixel.vp.beta = 0.0;
				pixel.vp.bsdf = nullptr;
			}, numPixel, 4096);

			//向外部写出Image
			int offset = 0;
			if ((iter + 1) == _numIteration||(iter%_writeFrequency==0)) {
				std::unique_ptr<Spectrum[]> image(new Spectrum[numPixel]);
				int x0 = pixelBound.minPoint.x;
				int x1 = pixelBound.maxPoint.x;
				int y0 = pixelBound.minPoint.y;
				int y1 = pixelBound.maxPoint.y;
				int numPhoton = (iter + 1)*_photonNumPreIteration;
				for (int y = y0; y < y1; ++y) {
					for (int x = x0; x < x1; ++x) {
						SPPMPixel& pixel = pixels[(y - y0)*(x1 - x0) + (x - x0)];
						Spectrum L = pixel.Ld / (iter + 1);//直接光成分的mean(Direct算法)
						L = L + pixel.tau / (numPhoton*Pi*pixel.radius*pixel.radius);//计算间接光成分(SPPM算法)
						image[offset++]=L;
						//image[offset++]=Spectrum(pixel.radius);
					}
				}
				_camera->film->SetImage(image.get());
				_camera->film->WriteImage();

			}
			reporter.Update();
		}
		reporter.Done();
	}
};

SPPMIntegrator *CreateSPPMIntegrator(const ParamSet &params,
	std::shared_ptr<Sampler> sampler, std::shared_ptr<const Camera> camera);


#endif /* SRC_INTEGRATORS_SPPM_H_ */

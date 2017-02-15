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

//SPPM 随机渐进式光子映射
class SPPMIntegrator:public Integrator{
private:
	std::shared_ptr<const Camera> _camera;
	const int _numIteration;//ray-photon pass个数
public:

	SPPMIntegrator(std::shared_ptr<const Camera>& camera,int numIteration):_camera(camera),_numIteration(numIteration){
	}

	virtual void Render(const Scene&){
		//获取有效的像素范围，然后计算像素个数
		const Bound2i& pixelBound=_camera->film->croppedPixelBound;
		int pixelNum=pixelBound.Area();//面积便是像素个数
		Vector2i tileExtent=pixelBound.Diagonal();
		int tileSize=16;//这是pbrt的标配tile大小>_<
		//计算tile的分辨率
		Point2i tileNum=Point2i((tileExtent.x+tileSize-1)/tileSize,(tileExtent.y+tileSize-1)/tileSize);

		//样本生成器
		//todo PBRT中这里使用了Halton Sampler
		RandomSampler sampler(_numIteration);

		//初始化进度条
		ProgressReporter reporter(2*_numIteration,"Rendering");
		for(int iter=0;iter<_numIteration;++iter){
			//首先是 camera pass
			//产生visible point(hit point)的pass
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
					RayDifferential ray;
					//获取相机样本
					CameraSample cs=tileSampler->GetCameraSample(raster);
					//生成射线
					Float beta=_camera->GenerateRayDifferential(cs,&ray);
				}
			},tileNum);
			reporter.Update();
			//然后是 photon pass
			reporter.Update();
		}
		reporter.Done();
	}
};

SPPMIntegrator *CreateSPPMIntegrator(const ParamSet &params,
	std::shared_ptr<Sampler> sampler, std::shared_ptr<const Camera> camera);


#endif /* SRC_INTEGRATORS_SPPM_H_ */

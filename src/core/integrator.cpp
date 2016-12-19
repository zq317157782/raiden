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
void SamplerIntegrator::RenderScene(const Scene& scene) {
	//首先计算需要的tile数
	Bound2i filmBound = _camera->film->GetSampleBounds();
	Debug("[SamplerIntegrator::RenderScene][filmBound:"<<filmBound<<"]");
	Vector2i filmExtent = filmBound.Diagonal();
	const int tileSize = 16; //默认是16*16为1个tile
	int numTileX = (filmExtent.x + tileSize - 1) / tileSize;
	int numTileY = (filmExtent.y + tileSize - 1) / tileSize;
	Point2i numTile(numTileX, numTileY);
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
//<<并行循环体结束>>
		}, numTile);
		_camera->film->WriteImage();
}


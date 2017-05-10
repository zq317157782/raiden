/*
 * ao.h
 *
 *  Created on: 2017年5月10日
 *      Author: zhuqian
 */

#ifndef SRC_INTEGRATORS_AO_H_
#define SRC_INTEGRATORS_AO_H_
#include "raiden.h"
#include "integrator.h"
#include "sampler.h"
#include "film.h"
#include "scene.h"
//烘焙AO贴图的积分器
class AOIntegrator: public Integrator {
private:
	std::shared_ptr<Sampler> _sampler; //采样器
	Film* _film; //AO贴图
public:
	AOIntegrator(const std::shared_ptr<Sampler>& sampler, Film* film) :
			_sampler(sampler), _film(film) {
	}
	virtual void Render(const Scene&scene) override {
			//首先计算需要的tile数
			Bound2i filmBound = _film->GetSampleBounds();
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
					std::unique_ptr<FilmTile> filmTile=_film->GetFilmTile(tileBound);
					//遍历tile
					for(Point2i pixel:tileBound) {
						localSampler->StartPixel(pixel); //开始一个像素
						do {
							//计算样本点对应的UV坐标
							Point2f uv=pixel+localSampler->Get2DSample();
							uv.x/=filmExtent.x;
							uv.y/=filmExtent.y;

							//通过uv坐标找到相应的local point

							//生成射线
							RayDifferential ray;
							//根据每个像素中包含的样本数，缩放射线微分值
							ray.ScaleRayDifferential(1.0f/std::sqrt((Float)localSampler->samplesPerPixel));
							Spectrum L(0.0f);//总共的radiance之和
							filmTile->AddSample(pixel, L, 1);
							arena.Reset();
						}while (localSampler->StartNextSample());
					}
					//合并tile
					_film->MergeFilmTile(std::move(filmTile));
					reporter.Update();
		//<<并行循环体结束>>
				}, numTile);
				reporter.Done();
				_film->WriteImage();
	}
};

#endif /* SRC_INTEGRATORS_AO_H_ */

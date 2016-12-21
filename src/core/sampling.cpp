/*
 * sampling.cpp
 *
 *  Created on: 2016年12月21日
 *      Author: zhuqian
 */
#include "sampling.h"
#include "rng.h"
void StratifiedSample1D(Float *samples, int nsamples, RNG &rng,
                        bool jitter){
	Float invNumSamples=1.0f/nsamples;
	for(int i=0;i<nsamples;++i){
		Float delta=jitter?rng.UniformFloat():0.5f;
		Float sampleVaule=(i+delta)*invNumSamples;
		samples[i]=std::min(sampleVaule,OneMinusEpsilon);
	}
}

void StratifiedSample2D(Point2f *samples, int nx,int ny, RNG &rng,
                        bool jitter){
	Float invNX=1.0f/nx;
	Float invNY=1.0f/ny;
	for(int j=0;j<ny;++j){
		for(int i=0;i<nx;++i){
			Float dx=jitter?rng.UniformFloat():0.5f;
			Float dy=jitter?rng.UniformFloat():0.5f;
			samples->x=std::min((i+dx)*invNX,OneMinusEpsilon);
			samples->y=std::min((j+dy)*invNY,OneMinusEpsilon);
			++samples;//步进到下一个Point2f
		}
	}
}



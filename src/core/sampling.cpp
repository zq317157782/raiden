/*
 * sampling.cpp
 *
 *  Created on: 2016年12月21日
 *      Author: zhuqian
 */
#include "sampling.h"
#include "rng.h"
void StratifiedSample1D(Float *samples, int nsamples, RNG &rng, bool jitter) {
	Float invNumSamples = 1.0f / nsamples;
	for (int i = 0; i < nsamples; ++i) {
		Float delta = jitter ? rng.UniformFloat() : 0.5f;
		Float sampleVaule = (i + delta) * invNumSamples;
		samples[i] = std::min(sampleVaule, OneMinusEpsilon);
	}
}

void StratifiedSample2D(Point2f *samples, int nx, int ny, RNG &rng,
		bool jitter) {
	Float invNX = 1.0f / nx;
	Float invNY = 1.0f / ny;
	for (int j = 0; j < ny; ++j) {
		for (int i = 0; i < nx; ++i) {
			Float dx = jitter ? rng.UniformFloat() : 0.5f;
			Float dy = jitter ? rng.UniformFloat() : 0.5f;
			samples->x = std::min((i + dx) * invNX, OneMinusEpsilon);
			samples->y = std::min((j + dy) * invNY, OneMinusEpsilon);
			++samples; //步进到下一个Point2f
		}
	}
}

Point2f RejectionSampleDisk(RNG& rng) {
	Float x, y;
	do {
		x = rng.UniformFloat();
		y = rng.UniformFloat();
	} while (x * x + y * y > 1);
	return Point2f(x, y);
}


Vector3f UniformSampleHemisphere(const Point2f &u){
	Float z=u.x;//半球范围内 z的定义域是[0-1]
	Float r=std::sqrt(std::max(1.0f-z*z,0.0f));
	Float phi=2*Pi*u.y;//计算得到phi
	//然后在xy平面上求x和y
	Float x=std::cos(phi)*r;
	Float y=std::sin(phi)*r;
	return Vector3f(x,y,z);
}

Float UniformHemispherePdf(){
	return Inv2Pi;
}

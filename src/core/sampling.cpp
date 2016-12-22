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

Vector3f UniformSampleHemisphere(const Point2f &u) {
	Float z = u.x; //半球范围内 z的定义域是[0-1]
	Float r = std::sqrt(std::max(1.0f - z * z, 0.0f));
	Float phi = 2 * Pi * u.y; //计算得到phi
	//然后在xy平面上求x和y
	Float x = std::cos(phi) * r;
	Float y = std::sin(phi) * r;
	return Vector3f(x, y, z);
}

Float UniformHemispherePdf() {
	return Inv2Pi;
}

Vector3f UniformSampleSphere(const Point2f &u) {
	Float z = -(u.x * 2 - 1);
	Float r = std::sqrt(std::max(1.0f - z * z, 0.0f));
	Float phi = 2 * Pi * u.y;	//计算得到phi
	//然后在xy平面上求x和y
	Float x = std::cos(phi) * r;
	Float y = std::sin(phi) * r;
	return Vector3f(x, y, z);
}

Float UniformSpherePdf() {
	return Inv4Pi;
}

Point2f UniformSampleDisk(const Point2f &u) {
	Assert(u.x >= 0);
	Float r = std::sqrt(u.x);
	Float theta = 2 * Pi * u.y;
	Float x = std::cos(theta) * r;
	Float y = std::sin(theta) * r;
	return Point2f(x, y);
}

Point2f ConcentricSampleDisk(const Point2f &u) {
	Point2f uOffset =  u*2.0f - Vector2f(1, 1);
	//返回退化的情况
	if (uOffset.x == 0 && uOffset.y == 0) {
		return Point2f(0, 0);
	}

	Float theta, r;
	if (std::abs(uOffset.x) > std::abs(uOffset.y)) {
		r = uOffset.x;
		theta = PiOver4 * (uOffset.y / uOffset.x);
	} else {
		r = uOffset.y;
		theta = PiOver2 - PiOver4 * (uOffset.x / uOffset.y);
	}
	return Point2f(r * std::cos(theta), r * std::sin(theta));
}

void LatinHypercube(Float *samples, int nSamples, int nDim, RNG &rng){
	Float invNumSamples=1.0f/nSamples;
	//先生成对角线样本
	for(int i=0;i<nSamples;++i){
		for(int j=0;j<nDim;++j){
			Float sample=(i+rng.UniformFloat())*invNumSamples;
			samples[nDim*i+j]=std::min(sample,OneMinusEpsilon);
		}
	}
	//对每个维度的样本进行乱序排列
	//这里把x轴想象成nDim ,y轴想象成nSamples的二维数组
	for(int i=0;i<nDim;++i){
		for(int j=0;j<nSamples;++j){
			int other=rng.UniformUInt32(nSamples-j)+j;
			std::swap(samples[nDim*j+i],samples[nDim*other+i]);
		}
	}
}

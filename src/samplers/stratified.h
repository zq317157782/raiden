/*
 * stratified.h
 *
 *  Created on: 2017年2月4日
 *      Author: zhuqian
 */

#ifndef SRC_SAMPLERS_STRATIFIED_H_
#define SRC_SAMPLERS_STRATIFIED_H_
#include "raiden.h"
#include "sampler.h"
#include "sampling.h"
//分层采样器
class StratifiedSampler: public PixelSampler {
private:
	const bool _jitterSamples; //是否开启抖动
	const int _xPixelSamples;
	const int _yPixelSamples;

public:
	StratifiedSampler(int xPixelSamples, int yPixelSamples, bool jittered,
			int numD) :
			PixelSampler(xPixelSamples * yPixelSamples, numD), _jitterSamples(
					jittered), _xPixelSamples(xPixelSamples), _yPixelSamples(
					yPixelSamples) {

	}

	//重写StartPixel,为样本做分层采样
	virtual void StartPixel(const Point2i& p) override {
		//生成1维分层样本
		for (int i = 0; i < _samples1D.size(); ++i) {
			StratifiedSample1D(&_samples1D[i][0],
					_xPixelSamples * _yPixelSamples, _rng, _jitterSamples);
			Shuffle(&_samples1D[i][0], _xPixelSamples * _yPixelSamples, 1,
					_rng);
		}

		//生成2维分层样本
		for (int i = 0; i < _samples2D.size(); ++i) {
			StratifiedSample2D(&_samples2D[i][0], _xPixelSamples,
					_yPixelSamples, _rng, _jitterSamples);
			Shuffle(&_samples2D[i][0], _xPixelSamples * _yPixelSamples, 1,
					_rng);
		}

		//随机排列数组
		for (int i = 0; i < _samples1DArraySize.size(); ++i) {
			int size = _samples1DArraySize[i];
			for (int j = 0; j < samplesPerPixel; ++j) {
				StratifiedSample1D(&_sampleArray1D[i][j * size],size, _rng, _jitterSamples);
				Shuffle(&_sampleArray1D[i][j * size], size, 1,_rng);
			}
		}

		//生成超拉丁立方样本的数组，只限于2维变量
		for (int i = 0; i < _samples2DArraySize.size(); ++i) {
			int size = _samples2DArraySize[i];
			for (int j = 0; j < samplesPerPixel; ++j) {
				LatinHypercube(&_sampleArray2D[i][j * size].x, size, 2, _rng);
			}
		}

		PixelSampler::StartPixel(p);
	}

	//返回分层采样器的克隆
	virtual std::unique_ptr<Sampler> Clone(int seed = 0/*用来设置随机策略的种子数据*/) const
			override {
		StratifiedSampler * ss = new StratifiedSampler(*this);
		ss->_rng.SetSequence(seed);
		return std::unique_ptr<Sampler>(ss);
	}
};

Sampler *CreateStratifiedSampler(const ParamSet &params);

#endif /* SRC_SAMPLERS_STRATIFIED_H_ */

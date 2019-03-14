/*
 * halton.h
 *
 *  Created on: 2017年2月23日
 *      Author: zhuqian
 */

#ifndef SRC_SAMPLERS_HALTON_H_
#define SRC_SAMPLERS_HALTON_H_
#include "raiden.h"
#include "sampler.h"
#include "rng.h"
#include "lowdiscrepancy.h"
#include "mmath.h"

//Halton样本器
//Halton序列的理论部分理解了，但是实现部分中寻找相应像素的index值没理解，直接复制自PBRT的代码，待以后实现
//PBRT建议看Gru ̈nschloß et al. (2012)对于算法的解释

static void extendedGCD(uint64_t a, uint64_t b, int64_t *x, int64_t *y);
static uint64_t multiplicativeInverse(int64_t a, int64_t n) {
	int64_t x, y;
	extendedGCD(a, n, &x, &y);
	return Mod(x, n);
}

static void extendedGCD(uint64_t a, uint64_t b, int64_t *x, int64_t *y) {
	if (b == 0) {
		*x = 1;
		*y = 0;
		return;
	}
	int64_t d = a / b, xp, yp;
	extendedGCD(b, a % b, &xp, &yp);
	*x = yp;
	*y = xp - (d * yp);
}

static constexpr int kMaxResolution = 128;

class HaltonSampler: public GlobalSampler {
private:
	static std::vector<uint16_t> _radicalInversePermutations; //记录着重排列表

	Point2i _baseScales, _baseExponents;

	int _sampleStride;

	int _multInverse[2];
	//当前需要计算偏移值得像素坐标
	mutable Point2i _pixelForOffset = Point2i(std::numeric_limits<int>::max(),
			std::numeric_limits<int>::max());
	//当前像素中第一个sample的index
	mutable int64_t _offsetForCurrentPixel;

public:
	HaltonSampler(int64_t samplesPerPixel, const Bound2i& pixelBound) :
			GlobalSampler(samplesPerPixel) {
		//计算重排列表
		if (_radicalInversePermutations.empty()) {
			RNG rng;
			_radicalInversePermutations = ComputeRadicalInversePermutations(
					rng);
		}
		//计算缩放
		Vector2i res = pixelBound.maxPoint - pixelBound.minPoint;
		for (int i = 0; i < 2; ++i) {
			int base = (i == 0) ? 2 : 3;
			int scale = 1, exp = 0;
			while (scale < std::min(kMaxResolution, res[i])) {
				scale *= base;
				++exp;
			}
			_baseScales[i] = scale;
			_baseExponents[i] = exp;
		}

		//遍历所有pixel一次的跨度
		_sampleStride = _baseScales[0] * _baseScales[1];

		_multInverse[0] = multiplicativeInverse(_baseScales[1], _baseScales[0]);
		_multInverse[1] = multiplicativeInverse(_baseScales[0], _baseScales[1]);

		_offsetForCurrentPixel = 0;
	}

	const uint16_t *PermutationForDimension(int dim) const {
		if (dim >= PrimeTableSize) {
			LError<<"HaltonSampler can only sample " << PrimeTableSize << " " "dimensions.";
		}
		return &_radicalInversePermutations[PrimeSums[dim]];
	}

	//从当前的pixel和sampleNumber 映射到全局的index
	virtual int64_t GetIndexForSample(int64_t sampleNum) const override {
		if (_currentPixel != _pixelForOffset) {
			_offsetForCurrentPixel = 0;
			if (_sampleStride > 1) {
				Point2i pm(Mod(_currentPixel[0], kMaxResolution),
						Mod(_currentPixel[1], kMaxResolution));
				for (int i = 0; i < 2; ++i) {
					uint64_t dimOffset =
							(i == 0) ?
									InverseRadicalInverse<2>(pm[i],
											_baseExponents[i]) :
									InverseRadicalInverse<3>(pm[i],
											_baseExponents[i]);
					_offsetForCurrentPixel += dimOffset
							* (_sampleStride / _baseScales[i])
							* _multInverse[i];
				}
				_offsetForCurrentPixel %= _sampleStride;
			}
			_pixelForOffset = _currentPixel;
		}

		return _offsetForCurrentPixel + sampleNum * _sampleStride;
	}

	//提供全局index和维度，返回index样本的dimension维度的值，对于第一和第二个样本，返回在当前pixel中的偏移
	virtual Float SampleDimension(int64_t index, int dimension) const override {
		
		if (dimension == 0) {
			return RadicalInverse(dimension, index >> _baseExponents[0]);
		} else if (dimension == 1) {
			return RadicalInverse(dimension, index / _baseScales[1]);
		} else {
			return ScrambledRadicalInverse(dimension, index,
					PermutationForDimension(dimension));
		}
	}

	virtual std::unique_ptr<Sampler> Clone(int seed = 0) const override {
		return std::unique_ptr<Sampler>(new HaltonSampler(*this));
	}
};

HaltonSampler *CreateHaltonSampler(const ParamSet &params,
		const Bound2i &sampleBounds);

#endif /* SRC_SAMPLERS_HALTON_H_ */

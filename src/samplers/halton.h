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

static constexpr int kMaxResolution = 128;

class HaltonSampler :public GlobalSampler {
private:
	static std::vector<uint16_t> _radicalInversePermutations;//记录着重排列表

	Point2i _baseScales, _baseExponents;

	int _sampleStride;

public:
	HaltonSampler(int64_t samplesPerPixel,const Bound2i& pixelBound) :GlobalSampler(samplesPerPixel) {
		//计算重排列表
		if (_radicalInversePermutations.empty()) {
			RNG rng;
			_radicalInversePermutations = ComputeRadicalInversePermutations(rng);
		}
		//计算缩放
		Vector2i res = pixelBound.maxPoint - pixelBound.minPoint;
		for (int i = 0; i < 2; ++i) {
			int base = (i == 0) ? 2 : 3;
			int scale = 1, exp = 0;
			while (scale < std::min(res[i], kMaxResolution)) {
				scale *= base;
				++exp;
			}
			_baseScales[i] = scale;
			_baseExponents[i] = exp;
		}
		 
		//遍历所有pixel一次的跨度
		_sampleStride = _baseScales[0] * _baseScales[1];
	}
};

#endif /* SRC_SAMPLERS_HALTON_H_ */

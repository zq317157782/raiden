/*
 * random.h
 *
 *  Created on: 2016年12月6日
 *      Author: zhuqian
 */

#ifndef SRC_SAMPLERS_RANDOM_H_
#define SRC_SAMPLERS_RANDOM_H_
#include "raiden.h"
#include "sampler.h"
#include "rng.h"
class RandomSampler: public Sampler {
private:
	RNG _rng;
public:
	RandomSampler(int64_t spp) :
			Sampler(spp) {
	}
	virtual Float Get1DSample() override {
		return _rng.UniformFloat();
	}
	virtual Point2f Get2DSample() override {
		return Point2f(_rng.UniformFloat(), _rng.UniformFloat());
	}
	virtual std::unique_ptr<Sampler> Clone(int seed=0) const override{
		RandomSampler *rs=new RandomSampler(samplesPerPixel);
		rs->_rng=RNG(seed);
		return std::unique_ptr<RandomSampler>(rs);

	}
};
Sampler *CreateRandomSampler(const ParamSet &params);
#endif /* SRC_SAMPLERS_RANDOM_H_ */

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

	//重写StartPixel,为样本做分层采样
	virtual void StartPixel(const Point2i& p) override {

		//为一维数组生成随机样本
		for(int i=0;i<_samples1DArraySize.size();++i){
			int size=_samples1DArraySize[i];
			for(int j=0;j<size*samplesPerPixel;++j){
				_sampleArray1D[i][j]=_rng.UniformFloat();
			}
		}

		//为二维数组生成随机样本
		for(int i=0;i<_samples2DArraySize.size();++i){
			int size=_samples2DArraySize[i];
			for(int j=0;j<size*samplesPerPixel;++j){
				_sampleArray2D[i][j].x=_rng.UniformFloat();
				_sampleArray2D[i][j].y=_rng.UniformFloat();
			}
		}
		Sampler::StartPixel(p);
	}

	virtual Float Get1DSample() override {
		return _rng.UniformFloat();
	}
	virtual Point2f Get2DSample() override {
		return Point2f(_rng.UniformFloat(), _rng.UniformFloat());
	}
	virtual std::unique_ptr<Sampler> Clone(int seed=0) const override{
		RandomSampler *rs=new RandomSampler(samplesPerPixel);
		rs->_rng.SetSequence(seed);
		return std::unique_ptr<RandomSampler>(rs);
	}
	virtual bool SetSampleNumber(int num) override{
			return Sampler::SetSampleNumber(num);
	}
};
Sampler *CreateRandomSampler(const ParamSet &params);
#endif /* SRC_SAMPLERS_RANDOM_H_ */

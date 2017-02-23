/*
 * halton.cpp
 *
 *  Created on: 2017年2月23日
 *      Author: zhuqian
 */
#include "paramset.h"
#include "halton.h"
std::vector<uint16_t> HaltonSampler::_radicalInversePermutations;

HaltonSampler *CreateHaltonSampler(const ParamSet &params,
                                   const Bound2i &sampleBounds){
	 int nsamp = params.FindOneInt("pixelsamples", 16);
	 return new HaltonSampler(nsamp, sampleBounds);
}




/*
 * random.cpp
 *
 *  Created on: 2016年12月6日
 *      Author: zhuqian
 */
#include "random.h"
#include "paramset.h"
Sampler *CreateRandomSampler(const ParamSet &params) {
    int ns = params.FindOneInt("pixelsamples", 4);
    return new RandomSampler(ns);
}



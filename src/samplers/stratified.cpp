/*
 * stratified.cpp
 *
 *  Created on: 2017年2月4日
 *      Author: zhuqian
 */
#include "stratified.h"
#include "paramset.h"
Sampler *CreateStratifiedSampler(const ParamSet &params){
	bool jitter=params.FindOneBool("jitter",true);
	int  xsamples=params.FindOneInt("xsamples",4);
	int  ysamples=params.FindOneInt("ysamples",4);
	int  d=params.FindOneInt("dimensions",4);
	return new StratifiedSampler(xsamples,ysamples,d,jitter);
}




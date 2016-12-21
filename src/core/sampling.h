/*
 * sampling.h
 *
 *  Created on: 2016年12月21日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_SAMPLING_H_
#define SRC_CORE_SAMPLING_H_
#include "raiden.h"
#include "geometry.h"
//1维分层采样
void StratifiedSample1D(Float *samples, int nsamples, RNG &rand,
                        bool jitter = true);
//二维分层采样
void StratifiedSample2D(Point2f *samples, int nx,int ny, RNG &rand,
                        bool jitter = true);
#endif /* SRC_CORE_SAMPLING_H_ */

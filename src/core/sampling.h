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

//使用拒绝采样方法来采样圆盘
Point2f RejectionSampleDisk(RNG& rand);

//均匀采样半球方向 返回半球坐标系下的向量
Vector3f UniformSampleHemisphere(const Point2f &u);
//返回均匀采样半球的pdf
Float UniformHemispherePdf();
//均匀采样整个球体
Vector3f UniformSampleSphere(const Point2f &u);
//均匀采样球体的pdf
Float UniformSpherePdf();
//均匀采样disk
Point2f UniformSampleDisk(const Point2f &u);
#endif /* SRC_CORE_SAMPLING_H_ */

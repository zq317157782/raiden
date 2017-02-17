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
#include "RNG.h"
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
//同心轴SQUARE到同心轴DISK
Point2f ConcentricSampleDisk(const Point2f &u);
//rooking-jiter
void LatinHypercube(Float *samples, int nSamples, int nDim, RNG &rng);
//cos分布的半球采样
Vector3f CosineSampleHemisphere(const Point2f &u);

//均匀采样Cone的PDF
Float UniformConePdf(Float cosThetaMax);

//MIS中使用的权重计算方法
inline Float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf) {
	return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}
inline Float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf) {
	Float f = nf * fPdf, g = ng * gPdf;
	return (f * f) / (f * f + g * g);
}

//重排列
template<typename T>
void Shuffle(T* samples,int count,int d,RNG&  rng){
	for(int i=0;i<count;++i){
		int other=i+rng.UniformUInt32(count-i);//随机获取一个在i左边的索引
		//交换i和other之间两个样本的所有维度数据
		for(int j=0;j<d;++j){
			std::swap(samples[i*d+j],samples[other*d+j]);//交换两个变量
		}
	}
}

//反转2进制的位数(eg.b1000(8)=>b0001(1))
inline uint32_t ReverseBits32(uint32_t n) {
	//这里使用了2进制方便的位操作计算
	n=(n & 0xFFFF0000) >> 16 | (n & 0x0000FFFF) << 16;
	n=(n & 0xFF00FF00) >> 8  | (n & 0x00FF00FF) << 8;
	n=(n & 0xF0F0F0F0) >> 4 | (n & 0x0F0F0F0F) << 4;
	n=(n & 0xCCCCCCCC) >> 2 | (n & 0x33333333) << 2;
	n=(n & 0xAAAAAAAA) >> 1 | (n & 0x55555555) << 1;
	return n;
}

inline uint64_t ReverseBits64(uint64_t n) {
	return (ReverseBits32(n >> 32) | ReverseBits32(n) << 32);
}


//用来计算某个base下的RadicalInverse
//这里使用模板是为了减少以BASE为低的时候的除法操作
template <int base>
static Float RadicalInverseSpecialized(uint64_t a) {
	Float invBase = 1.0f / (Float)base;
	uint64_t reverse = 0;
	Float invBaseN = 1;
	while (a) {
		uint64_t next = a*invBase;
		uint64_t digit = a - next*base;
		reverse = reverse*base + digit;
		invBaseN *= invBase;
		a = next;
	}
	return std::min(reverse*invBaseN,OneMinusEpsilon);
}
#endif /* SRC_CORE_SAMPLING_H_ */

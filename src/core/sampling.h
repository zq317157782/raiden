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
void StratifiedSample1D(Float *samples, int nsamples, RNG &rand, bool jitter =
		true);
//二维分层采样
void StratifiedSample2D(Point2f *samples, int nx, int ny, RNG &rand,
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
Float CosineHemispherePdf(Float cosTheta);
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
void Shuffle(T* samples, int count, int d, RNG& rng) {
	for (int i = 0; i < count; ++i) {
		int other = i + rng.UniformUInt32(count - i); //随机获取一个在i左边的索引
		//交换i和other之间两个样本的所有维度数据
		for (int j = 0; j < d; ++j) {
			std::swap(samples[i * d + j], samples[other * d + j]);		//交换两个变量
		}
	}
}

//经过将近两个月的理论复习，终于又开始编码了，兴奋ING。ワクワクスル　2017年5月2号
//1维分段函数分布，定义域为[0~1]
struct Distribution1D {
private:
	std::vector<Float> _funcs;		//存放各个函数值
	std::vector<Float> _cdfs;		//存放各个区间的cdf数
	float _funcInt;		//函数在[0~1]区间之间的定积分值
public:
	//f是分段函数的值,n是值得个数
	Distribution1D(Float* f, int n) :
			_funcs(f, f + n), _cdfs(n + 1) {
		_cdfs[0] = 0;
		for (int i = 1; i < n + 1; ++i) {
			_cdfs[i] = _cdfs[i - 1] + _funcs[i] / n;
		}
		_funcInt = _cdfs[n];		//获取积分值
		//标准化CDF
		if (_funcInt == 0) {
			//积分值为0的情况
			for (int i = 1; i < n + 1; ++i) {
				_cdfs[i] = Float(i) / Float(n);
			}
		} else {
			//积分值不为0的情况
			for (int i = 1; i < n + 1; ++i) {
				_cdfs[i] /= _funcInt;
			}
		}
	}

	//返回函数个数
	int Count() const {
		return _funcs.size();
	}

	//采样连续函数
	//返回作用域[0~1]
	//u是样本值，pdf是概率密度函数,offset=>[cdfs[offset]<=ret<cdfs[offset+1]]

	Float SampleContinuous(Float u, Float* pdf, int* offset = nullptr) const {
		//使用二分查找寻找到offset
		int OFFSET = FindInterval(_cdfs.size(),
				[&](int index) {return _cdfs[index]<=u;});
		if (offset) {
			*offset = OFFSET;
		}
		//du [0~1]
		Float du = (u - _cdfs[OFFSET]);
		Float temp = (_cdfs[OFFSET + 1] - _cdfs[OFFSET]);
		//这里做判断是因为可能_cdfs[OFFSET+1]==_cdfs[OFFSET],导致除以0;
		if (temp > 0) {
			du /= temp;
		}

		//pdf等于函数值除以积分值
		if (pdf) {
			*pdf = _funcs[OFFSET] / _funcInt;
		}
		return (Float(OFFSET) + du) / Count();	//计算返回值
	}

	//采样离散分布
	//uRemapped代表U在x和x+1之间的多少之间
	int SampleDiscrete(Float u, Float *pdf, Float* uRemapped = nullptr) const {
		//使用二分查找寻找到offset
		int offset = FindInterval(_cdfs.size(),
				[&](int index) {return _cdfs[index]<=u;});
		//pdf等于函数值除以积分值
		if (pdf) {
			//这里要比连续版本多一个Count,因为要乘以区间delta
			*pdf = _funcs[offset] / (_funcInt * Count());
		}
		if (uRemapped) {
			//这里没有像连续版本一样判断_cdfs[offset + 1]==_cdfs[offset]的情况
			//难道是默认离散版本不会出现这种情况?
			*uRemapped = (u - _cdfs[offset])
					/ (_cdfs[offset + 1] - _cdfs[offset]);
		}
		return offset;
	}

	//返回离散版本的PDF
	int DiscretePDF(int index) const {
		Assert(index >= 0);
		Assert(index < Count());
		return _funcs[index] / (_funcInt * Count());
	}
};

#endif /* SRC_CORE_SAMPLING_H_ */

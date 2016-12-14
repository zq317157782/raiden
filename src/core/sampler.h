/*
 * sampler.h
 *
 *  Created on: 2016年12月6日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_SAMPLER_H_
#define SRC_CORE_SAMPLER_H_
#include "raiden.h"
//采样器
// sample[x,y,t,u,v,[array],[array],[array],...]
class Sampler{
private:
	size_t _array1DOffset;
	size_t _array2DOffset;
public:
	const int64_t samplesPerPixel;
protected:
	Point2i _currentPixel;//当前处理的像素点
	int64_t _currentPixelSampleIndex;//当前像素的index
	std::vector<int> _samplesArray1DSize;//记录1D样本数组的大小
	std::vector<int> _samplesArray2DSize;//记录2D样本数组的大小
	std::vector<std::vector<Float>> _sample1DArray;//1D样本数组
	std::vector<std::vector<Point2f>> _sample2DArray;//2D样本数组
public:
	Sampler(int64_t samplesPerPixel):samplesPerPixel(samplesPerPixel){
		Assert(samplesPerPixel>0);
	}
	virtual ~Sampler(){}
	virtual Float Get1DSample()=0;//获取一维样本
	virtual Point2f Get2DSample()=0;//获取二维样本


	//申请一个1D样本数组
	void Request1DArray(int n){
		Assert(n==RoundCount(n));
		_samplesArray1DSize.push_back(n);
		_sample1DArray.push_back(std::vector<Float>(n*samplesPerPixel));
	}
	//申请一个2D样本数组
	void Request2DArray(int n){
		Assert(n==RoundCount(n));
		_samplesArray2DSize.push_back(n);
		_sample2DArray.push_back(std::vector<Point2f>(n*samplesPerPixel));
	}
	//对样本个数做优化
	virtual int RoundCount(int n) const{
		return n;
	}
	//获取一个相机样本
	CameraSample GetCameraSample(const Point2i& pRaster){
		CameraSample sample;
		sample.pFilm=(Point2f)pRaster+Get2DSample();
		sample.time=Get1DSample();
		sample.pLen=Get2DSample();
		return sample;
	}
	//获得当前的样本索引
	int64_t CurrentSampleNumber() const{
		return _currentPixelSampleIndex;
	}

	//设置当前的样本索引
	virtual bool SetSampleNumber(int num){
		_array1DOffset=0;
		_array2DOffset=0;
		_currentPixelSampleIndex=num;
		return _currentPixelSampleIndex<samplesPerPixel;
	}

	//开始一个新的像素
	virtual void StartPixel(const Point2i& p){
		_currentPixel=p;
		_currentPixelSampleIndex=0;
		_array1DOffset=0;
		_array2DOffset=0;
	}
	//开始一个新的样本
	virtual bool StartNextSample(){
		_array1DOffset=0;
		_array2DOffset=0;
		return ++_currentPixelSampleIndex<samplesPerPixel;
	}

	//返回一个包含n个样本的数组，返回的时候要根据requert的时候申请的大小做检查
	virtual Float* Get1DArray(int n){
		Assert(_array1DOffset<_samplesArray1DSize.size());
		Assert(_samplesArray1DSize[_array1DOffset]==n);
		return &_sample1DArray[_array1DOffset][samplesPerPixel*n];
	}

	virtual Point2f* Get2DArray(int n){
		Assert(_array2DOffset<_samplesArray2DSize.size());
		Assert(_samplesArray2DSize[_array2DOffset]==n);
		return &_sample2DArray[_array2DOffset][samplesPerPixel*n];
	}

	//克隆方法，用来克隆本sampler
	virtual std::unique_ptr<Sampler> Clone(int seed=0/*用来设置随机策略的种子数据*/) const=0;
};




#endif /* SRC_CORE_SAMPLER_H_ */

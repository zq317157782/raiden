/*
 * sampler.h
 *
 *  Created on: 2016年12月6日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_SAMPLER_H_
#define SRC_CORE_SAMPLER_H_
#include "raiden.h"
#include "geometry.h"
#include "camera.h"
#include "RNG.h"
//采样器
// sample[x,y,t,u,v,[array],[array],[array],...]
class Sampler {
private:
	size_t _array1DOffset;
	size_t _array2DOffset;
public:
	const int64_t samplesPerPixel;
protected:
	Point2i _currentPixel; //当前处理的像素点
	int64_t _currentPixelSampleIndex; //当前像素的index
	std::vector<int> _samples1DArraySize; //记录1D样本数组的大小
	std::vector<int> _samples2DArraySize; //记录2D样本数组的大小
	std::vector<std::vector<Float>> _sampleArray1D; //1D样本数组
	std::vector<std::vector<Point2f>> _sampleArray2D; //2D样本数组
public:
	Sampler(int64_t samplesPerPixel) :
			samplesPerPixel(samplesPerPixel) {
		Assert(samplesPerPixel > 0);
		_currentPixelSampleIndex = 0;
		_array1DOffset = 0;
		_array2DOffset = 0;
	}
	virtual ~Sampler() {
	}
	virtual Float Get1DSample()=0; //获取一维样本
	virtual Point2f Get2DSample()=0; //获取二维样本

	//申请一个1D样本数组
	void Request1DArray(int n) {
		Assert(n == RoundCount(n));
		_samples1DArraySize.push_back(n);
		_sampleArray1D.push_back(std::vector<Float>(n * samplesPerPixel));
	}
	//申请一个2D样本数组
	void Request2DArray(int n) {
		Assert(n == RoundCount(n));
		_samples2DArraySize.push_back(n);
		_sampleArray2D.push_back(std::vector<Point2f>(n * samplesPerPixel));
	}
	//对样本个数做优化
	virtual int RoundCount(int n) const {
		return n;
	}
	//获取一个相机样本
	CameraSample GetCameraSample(const Point2i& pRaster) {
		CameraSample sample;
		sample.pFilm = (Point2f) pRaster + Get2DSample();
		sample.time = Get1DSample();
		sample.pLen = Get2DSample();
		return sample;
	}
	//获得当前的样本索引
	int64_t CurrentSampleNumber() const {
		return _currentPixelSampleIndex;
	}

	//设置当前的样本索引
	virtual bool SetSampleNumber(int num) {
		_array1DOffset = 0;
		_array2DOffset = 0;
		_currentPixelSampleIndex = num;
		return _currentPixelSampleIndex < samplesPerPixel;
	}

	//开始一个新的像素
	virtual void StartPixel(const Point2i& p) {
		_currentPixel = p;
		_currentPixelSampleIndex = 0;
		_array1DOffset = 0;
		_array2DOffset = 0;
	}
	//开始一个新的样本
	virtual bool StartNextSample() {
		_array1DOffset = 0;
		_array2DOffset = 0;
		return (++_currentPixelSampleIndex) < samplesPerPixel;
	}

	//返回一个包含n个样本的数组，返回的时候要根据requert的时候申请的大小做检查
	virtual Float* Get1DArray(int n) {
		if (_array1DOffset == _samples1DArraySize.size()) {
			return nullptr;
		}
		Assert(_currentPixelSampleIndex < samplesPerPixel);
		Assert(_samples1DArraySize[_array1DOffset] == n);
		return &_sampleArray1D[_array1DOffset++][_currentPixelSampleIndex * n];
	}

	virtual Point2f* Get2DArray(int n) {
		if (_array2DOffset == _samples2DArraySize.size()) {
			return nullptr;
		}
		Assert(_currentPixelSampleIndex < samplesPerPixel);
		Assert(_samples2DArraySize[_array2DOffset] == n);
		return &_sampleArray2D[_array2DOffset++][_currentPixelSampleIndex * n];
	}

	//克隆方法，用来克隆本sampler
	virtual std::unique_ptr<Sampler> Clone(
			int seed = 0/*用来设置随机策略的种子数据*/) const=0;
};

//像素采样器:用于需要为整个像素事前生成样本的采样算法，而不能生成on fly
//只预计算no-array样本
class PixelSampler: public Sampler {
protected:
	std::vector<std::vector<Float>> _samples1D; //预先计算的1维样本
	std::vector<std::vector<Point2f>> _samples2D; //预先计算的2维样本
	int _cur1DPos;
	int _cur2DPos;
	RNG _rng; //用于生成均匀随机样本
public:
	PixelSampler(int64_t samplesPerPixel, int numD) :
			Sampler(samplesPerPixel) {
		_cur1DPos = 0;
		_cur2DPos = 0;
		//开始填充未生成的样本
		for (int i = 0; i < numD; ++i) {
			_samples1D.push_back(std::vector<Float>(samplesPerPixel));
			_samples2D.push_back(std::vector<Point2f>(samplesPerPixel));
		}
	}

	virtual bool StartNextSample() override {
		_cur1DPos = 0;
		_cur2DPos = 0;
		return Sampler::StartNextSample();
	}

	virtual bool SetSampleNumber(int num) override {
		_cur1DPos = 0;
		_cur2DPos = 0;
		return Sampler::SetSampleNumber(num);
	}

	//先返回预计算的样本，然后超出预计算的样本维度，返回均匀采样的样本
	virtual Float Get1DSample() override { //获取一维样本
		if (_cur1DPos < _samples1D.size()) {
			return _samples1D[_cur1DPos++][_currentPixelSampleIndex];
		} else {
			return _rng.UniformFloat();
		}
	}

	//同上，只是2维样本
	//先返回预计算的样本，然后超出预计算的样本维度，返回均匀采样的样本
	virtual Point2f Get2DSample() override {
		if (_cur2DPos < _samples2D.size()) {
			return _samples2D[_cur2DPos++][_currentPixelSampleIndex];
		} else {
			return Point2f(_rng.UniformFloat(), _rng.UniformFloat());
		}
	}
};

//全局采样器，采样的时候以整个image空间为采样空间
class GlobalSampler:public Sampler{
private:
	int _dimension;//记录当前的样本维度
	int64_t _globalIndex;//记录当前样本的全局索引
	//前5个样本值是给CameraSample的
	static const int _arrayStartDim = 5;
	int _arrayEndDim;//数组样本的结束
public:
	GlobalSampler(int64_t samplesPerPixel) :Sampler(samplesPerPixel) {}

	//从当前的pixel和sampleNumber 映射到全局的index
	virtual int64_t GetIndexForSample(int64_t) const=0;
	//提供全局index和维度，返回index样本的dimension维度的值，对于第一和第二个样本，返回在当前pixel中的偏移
	virtual Float SampleDimension(int64_t index,int dimension) const=0;


	virtual void StartPixel(const Point2i& p) override{
		Sampler::StartPixel(p);
		_dimension = 0;//每次开始新的像素,要重置维度
		_globalIndex = GetIndexForSample(0);
		//计算数组样本的维度
		_arrayEndDim = _arrayStartDim + _sampleArray1D.size() + 2 * _sampleArray2D.size();

		//生成1D数组样本
		for (int i = 0; i < _samples1DArraySize.size(); ++i) {
			int nSample = _samples1DArraySize[i]* samplesPerPixel;
			for (int j = 0; j < nSample; ++j) {
				int64_t index=GetIndexForSample(j);//根据当前的像素和样本索引，获取样本的内部索引
				_sampleArray1D[i][j] = SampleDimension(index, _arrayStartDim + i);
			}
		}

		//生成2D数组样本
		int dim = _arrayStartDim + _sampleArray1D.size();
		for (int i = 0; i < _samples2DArraySize.size(); ++i) {
			int nSample = _samples2DArraySize[i] * samplesPerPixel;
			for (int j = 0; j < nSample; ++j) {
				int64_t index = GetIndexForSample(j);//根据当前的像素和样本索引，获取样本的内部索引
				_sampleArray2D[i][j].x = SampleDimension(index,dim);
				_sampleArray2D[i][j].y = SampleDimension(index, dim+1);
			}
			dim += 2;//步进到下个维度
		}

	}

	virtual bool StartNextSample() override {
		_dimension = 0;
		_globalIndex = GetIndexForSample(_currentPixelSampleIndex+1);//计算全局索引
		return Sampler::StartNextSample();
	}

	virtual bool SetSampleNumber(int num) override {
		_dimension = 0;
		_globalIndex = GetIndexForSample(_currentPixelSampleIndex + 1);//计算全局索引
		return Sampler::SetSampleNumber(num);
	}

	virtual Float Get1DSample() override {
		//判断是否跳过数组维度
		if (_dimension >= _arrayStartDim&&_dimension<_arrayEndDim) {
			_dimension = _arrayEndDim;
		}
		//返回样本值
		return SampleDimension(_globalIndex, _dimension++);
	}
	virtual Point2f Get2DSample() override {
		if ((_dimension+1)>= _arrayStartDim&&_dimension<_arrayEndDim) {
			_dimension = _arrayEndDim;
		}
		Float x= SampleDimension(_globalIndex, _dimension);
		Float y= SampleDimension(_globalIndex, _dimension+1);
		_dimension += 2;
		return Point2f(x, y);
	}
};

#endif /* SRC_CORE_SAMPLER_H_ */

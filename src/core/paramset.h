/*
 * paramset.h
 *
 *  Created on: 2016年12月15日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_PARAMSET_H_
#define SRC_CORE_PARAMSET_H_
#include "raiden.h"

//代表一个参数以及它的值
template <typename T>
struct ParamSetItem{
	const std::string name;//参数名
	const std::unique_ptr<T[]> values;//参数的值
	const int nValues;//参数的长度大于1代表数组
	mutable bool lookUp=false;//判断是否已经查询过这个变量
	ParamSetItem(const std::string& name,std::unique_ptr<T[]> values,int nValues=1)
	:name(name),values(std::move(values)),nValues(nValues){}
};

//代表一个参数集
class ParamSet{
private:
	std::vector<std::shared_ptr<ParamSetItem<bool>>> _bools;
	std::vector<std::shared_ptr<ParamSetItem<int>>> _ints;
	std::vector<std::shared_ptr<ParamSetItem<Float>>> _floats;
public:
	ParamSet(){}
	//bool类型相关操作
	void AddBool(const std::string& name,std::unique_ptr<bool[]> values,int nValues);
	bool EraseBool(const std::string& name);
	const bool* FindBool(const std::string& name,int *nValues) const;
	const bool FindOneBool(const std::string& name,int defaultValue) const;
	//Float类型相关操作
	void AddFloat(const std::string& name,std::unique_ptr<Float[]> values,int nValues);
	bool EraseFloat(const std::string& name);
	const Float* FindFloat(const std::string& name,int *nValues) const;
	const Float FindOneFloat(const std::string& name,int defaultValue) const;
	//int类型相关操作
	void AddInt(const std::string& name,std::unique_ptr<int[]> values,int nValues);
	bool EraseInt(const std::string& name);
	const int* FindInt(const std::string& name,int *nValues) const;
	const int FindOneInt(const std::string& name,int defaultValue) const;
};

#endif /* SRC_CORE_PARAMSET_H_ */

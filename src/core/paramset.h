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
	ParamSetItem(const std::string& name,const std::unique_ptr<T[]>& values,int nValues=1):
	name(name),values(std::move(values)),nValues(nValues){}
};



#endif /* SRC_CORE_PARAMSET_H_ */

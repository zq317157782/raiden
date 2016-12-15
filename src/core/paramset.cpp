/*
 * paramset.cpp
 *
 *  Created on: 2016年12月15日
 *      Author: zhuqian
 */
#include "paramset.h"
//定义一个宏定义用来添加参数
//这里要求name,values,nValues这三个形参的名字要固定
#define ADD_PARAM_TYPE(T,_list) \
(_list).emplace_back(new ParamSetItem<T>(name,std::move(values),nValues));

//查找指向相应参数值得指针
//这里要求name,values,nValues这三个形参的名字要固定
#define LOOKUP_PTR(_list)\
for(auto &v:_list){\
		if(v->name==name){\
			*nValues=v->nValues;\
			v->lookUp=true;\
			return v->values.get();\
		}\
}\
return nullptr;

//这里是寻找1个参数值得宏定义
#define LOOKUP_ONE(_list)\
for(auto &v:_list){\
		if(v->name==name&&v->nValues==1){\
			v->lookUp=true;\
			return v->values[0];\
		}\
	}\
	return defaultValue;

void ParamSet::AddBool(const std::string& name, std::unique_ptr<bool[]> values,
		int nValues) {
	EraseBool(name);
	ADD_PARAM_TYPE(bool, _bools)
}
void ParamSet::AddFloat(const std::string& name,
		std::unique_ptr<Float[]> values, int nValues) {
	EraseFloat(name);
	ADD_PARAM_TYPE(Float, _floats);
}

void ParamSet::AddInt(const std::string& name,
		std::unique_ptr<int[]> values, int nValues) {
	EraseInt(name);
	ADD_PARAM_TYPE(int, _ints);
}

bool ParamSet::EraseBool(const std::string& name) {
	for (int i = 0; i < _bools.size(); ++i) {
		if (_bools[i]->name == name) {
			_bools.erase(_bools.begin() + i);
			return true;
		}
	}
	return false;
}

bool ParamSet::EraseFloat(const std::string& name) {
	for (int i = 0; i < _floats.size(); ++i) {
		if (_floats[i]->name == name) {
			_floats.erase(_floats.begin() + i);
			return true;
		}
	}
	return false;
}

bool ParamSet::EraseInt(const std::string& name) {
	for (int i = 0; i < _ints.size(); ++i) {
		if (_ints[i]->name == name) {
			_ints.erase(_ints.begin() + i);
			return true;
		}
	}
	return false;
}



const bool* ParamSet::FindBool(const std::string& name, int *nValues) const {
	LOOKUP_PTR(_bools)
}

const Float* ParamSet::FindFloat(const std::string& name, int *nValues) const{
	LOOKUP_PTR(_floats)
}

const int* ParamSet::FindInt(const std::string& name, int *nValues) const{
	LOOKUP_PTR(_ints);
}



const bool ParamSet::FindOneBool(const std::string& name,
		int defaultValue) const {
	LOOKUP_ONE(_bools)
}

const Float ParamSet::FindOneFloat(const std::string& name, int defaultValue) const{
	LOOKUP_ONE(_floats)
}

const int ParamSet::FindOneInt(const std::string& name, int defaultValue) const{
	LOOKUP_ONE(_ints)
}




/*
 * raiden.h
 *
 *  Created on: 2016年11月9日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_RAIDEN_H_
#define SRC_CORE_RAIDEN_H_
//raiden.h包含所有全局设置
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <assert.h>

//定义一个Float宏 可能指向float可能指向double
typedef float Float;



//根据编译选项设置断言宏
#ifdef DEBUG_BUILD
#define Assert(x) assert(x)
#else
#define Assert(x) ((void)0)
#endif


template<typename T> class Vector3;
template<typename T> class Vector2;
template<typename T> class Point3;
template<typename T> class Point2;


//float类型相应的IEEE标准的BIT格式
//最高位是符号位，然后8位是指数,接下来23位是值，
//指数为0的时候没有默认最高位的1
//指数为255的时候，值为0则为无穷大，值不为0则为NaN
inline uint32_t FloatToBits(float f){
	Assert(!std::isnan(f));
	uint32_t bits=0;
	std::memcpy(&bits,&f,sizeof(float));
	return bits;
}
//从BIT形式转换会float类型
inline float BitsToFloat(uint32_t bits){
	float f=0;
	std::memcpy(&f,&bits,sizeof(uint32_t));
	return f;
}

//获取下一个大于本float变量的float变量
//1.先判断是否是无限值，是的话直接返回
//2.如果是负0的话，先转换成正0，因为下面的比较需要0是一个正0
//3.转换成BIT形式，并且比较BIT是否大于0，大于++，小于--
//4.再度转换回float，并且返回
inline float NextFloatUp(float f){
	Assert(!std::isnan(f));
	if(std::isinf(f)&&f>0.0f)
		return f;
	if(f==-0.0f)
		f=0.0f;
	uint32_t bits=FloatToBits(f);
	if(bits>=0) ++bits;
	else --bits;
	return BitsToFloat(bits);
}

//获取下一个小于本float变量的float变量
inline float NextFloatDown(float f){
	Assert(!std::isnan(f));
	if(std::isinf(f)&&f<0.0f)
		return f;
	if(f==-0.0f)
		f=0.0f;
	uint32_t bits=FloatToBits(f);
	if(bits>=0) --bits;
	else ++bits;
	return BitsToFloat(bits);
}

//todo double operation

//这个MachineEpsion是数值分析下的MachineEpsion；为2的-24次方；
//C++标准库提供的Epsion是大于1的ULP，为2的-23次方
//所以需要再除以2
static constexpr Float MachineEpsion=0.5f*std::numeric_limits<Float>::epsilon();

#endif /* SRC_CORE_RAIDEN_H_ */

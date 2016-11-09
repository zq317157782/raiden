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
#define Float float



//todo 根据编译选项设置断言宏
#define DEBUG_BUILD
#ifdef DEBUG_BUILD
#define Assert(x) assert(x)
#else
#define Assert(x)
#endif

template<typename T> class Vector3;

#endif /* SRC_CORE_RAIDEN_H_ */

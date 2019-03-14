
/*
 * port.h
 *
 *  Created on: 2018年7月7日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_PLATFORM_H_
#define SRC_CORE_PLATFORM_H_
#pragma once

#include "raiden.h"

#if defined(__APPLE__)
	#define RAIDEN_IS_OSX  //是OSX操作系统
#else
	#define RAIDEN_IS_WIN //是WINDOWS操作系统
#endif

#if defined(_MSC_VER)
	#define RAIDEN_COMPILER_MSC
#elif defined(__clang__)
	#define RAIDEN_COMPILER_CLANG
#elif defined(__GNUC__)
	#define RAIDEN_COMPILER_GNUC
#endif 

#ifndef L1_CACHE_LINE_SIZE
	#define L1_CACHE_LINE_SIZE 64 //默认的cache line
#endif //L1_CACHE_LINE_SIZE

#ifdef _WIN32
	#include <intrin.h>
#endif


#if defined(__GNUC__) || defined(__clang__)
#define FINLINE                inline __attribute__((always_inline))
#define NOINLINE               __attribute__((noinline))
#define EXPECT_TAKEN(a)        __builtin_expect(!!(a), true)
#define EXPECT_NOT_TAKEN(a)    __builtin_expect(!!(a), false)
#define MAYBE_UNUSED           __attribute__((unused))
#elif defined(_MSC_VER)
#define FINLINE                __forceinline
#define NOINLINE               __declspec(noinline)
#define MM_ALIGN16             __declspec(align(16))
#define EXPECT_TAKEN(a)        (a)
#define EXPECT_NOT_TAKEN(a)    (a)
#define MAYBE_UNUSED           
#else
#error Unsupported compiler!
#endif


#endif /* SRC_CORE_PLATFORM_H_ */

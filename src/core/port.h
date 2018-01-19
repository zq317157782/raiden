/*
 * port.h
 *
 *  Created on: 2016年12月14日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_PORT_H_
#define SRC_CORE_PORT_H_

#if defined(__APPLE__)
	#define IS_OSX  //是OSX操作系统
#else
	#define IS_WIN //是WINDOWS操作系统
#endif

#if defined(_MSC_VER)
	#define COMPILER_MS
#elif defined(__clang__)
	#define COMPILER_CLANG 
#endif

#ifndef L1_CACHE_LINE_SIZE
#define L1_CACHE_LINE_SIZE 64 //默认的cache line
#endif




#endif /* SRC_CORE_PORT_H_ */

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
#endif //L1_CACHE_LINE_SIZE

#ifdef IS_WIN
	#ifdef COMPILER_MS
		#ifdef _M_IX86_FP 
			#define SIMD_ENABLE
			#ifdef __AVX__ 
				#define SIMD_AVX
			#elif __AVX2__
				#define SIMD_AVX2
			#else
				#if _M_IX86_FP==0
					#define SIMD_IA32
				#elif  _M_IX86_FP == 1
					#define SIMD_SSE	
				#elif  _M_IX86_FP == 2
					#define SIMD_SSE2
				#endif
			#endif
		#endif //_M_IX86_FP
	#endif // COMPILER_MS
#endif // IS_WIN






#endif /* SRC_CORE_PORT_H_ */

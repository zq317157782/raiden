/*
 * memory.cpp
 *
 *  Created on: 2016年12月14日
 *      Author: zhuqian
 */
#include "memory.h"
void *AllocAligned(size_t size){
	//return memalign(L1_CACHE_LINE_SIZE,size); //Linux下
#ifdef IS_OSX
	//OSX操作系统下
	void* ptr;
	if(posix_memalign(&ptr,L1_CACHE_LINE_SIZE,size)!=0){
		ptr=nullptr;
	}
	return ptr;
#elif defined(IS_WIN) && defined(COMPILER_MS)
	//Windows操作系统下并且使用MSC
	//Assert(false);
	return _aligned_malloc(size, L1_CACHE_LINE_SIZE);
#endif
}


void FreeAligned(void * ptr){
	if(!ptr){
		return;
	}
#ifdef IS_OSX
	free(ptr);
#elif defined(IS_WIN) && defined(COMPILER_MS)
	//Windows操作系统下并且使用MSC
	//Assert(false);
	_aligned_free(ptr);
#endif
}

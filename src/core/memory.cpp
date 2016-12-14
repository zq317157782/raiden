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
#elif defined(IS_WIN)
	//Windows操作系统下
	Assert(false);
#endif
}


template <typename T>
T *AllocAligned(size_t size){
	return AllocAligned(size*sizeof(T));
}

void FreeAligned(void * ptr){
	if(!ptr){
		return;
	}
#ifdef IS_OSX
	free(ptr);
#elif defined(IS_WIN)
	Assert(false);
#endif
}

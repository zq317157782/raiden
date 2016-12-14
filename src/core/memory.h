/*
 * memory.h
 *
 *  Created on: 2016年12月14日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_MEMORY_H_
#define SRC_CORE_MEMORY_H_
#include "raiden.h"

//cache friendly方式分配内存
void *AllocAligned(size_t size);

template <typename T>
T *AllocAligned(size_t size);

//释放cache friendly的内存
void FreeAligned(void *);

#endif /* SRC_CORE_MEMORY_H_ */

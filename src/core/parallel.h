/*
 * parallel.h
 *
 *  Created on: 2016年12月2日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_PARALLEL_H_
#define SRC_CORE_PARALLEL_H_
#include "raiden.h"
#include <atomic>
//原子操作的Float类型
//c++不支持原子的Float操作
class AtomicFloat{
private:
#ifdef FLOAT_IS_DOUBLE
	std::atomic<uint64_t> _bits;
#else
	std::atomic<uint32_t> _bits;
#endif
public:
	explicit AtomicFloat(Float f){
		_bits=FloatToBits(f);
	}

	operator float() const{
		return BitsToFloat(_bits);
	}

	//把float变量赋值给AtomicFloat变量
	Float operator=(Float f){
		_bits=FloatToBits(f);
		return f;
	}

	void Add(Float d){
#ifdef FLOAT_IS_DOUBLE
		uint64_t oldBits=_bits,newBits;
#else
		uint32_t oldBits=_bits,newBits;
#endif
		do{
			newBits=FloatToBits(BitsToFloat(oldBits)+d);
		} while(!_bits.compare_exchange_weak(oldBits,newBits));
	}
};


//当前线程索引,mian线程为0，其余的线程依次加1
extern thread_local int ThreadIndex;

//初始化工作线程池
void ParallelInit();
void ParallelCleanup();

//并行的for循环，是并行的关键
//openMP中也有类似的并行for循环
void ParallelFor(std::function<void(int64_t)> func,int count,int chunkSize=1);
//返回当前系统核心数
int NumSystemCores();
#endif /* SRC_CORE_PARALLEL_H_ */

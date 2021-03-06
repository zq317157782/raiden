﻿/*
 * memory.h
 *
 *  Created on: 2016年12月14日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_MEMORY_H_
#define SRC_CORE_MEMORY_H_
#include "raiden.h"
#include <list>
#include "platform.h"

//往arena分配的地址上初始化一个Type类型的变量
#define ARENA_ALLOC(arena, Type) new ((arena).Alloc(sizeof(Type))) Type

//cache friendly方式分配内存
void *AllocAligned(size_t size);

template<typename T>
T *AllocAligned(size_t size) {
	return (T*) AllocAligned(size * sizeof(T));
}

//释放cache friendly的内存
void FreeAligned(void *);

class alignas(L1_CACHE_LINE_SIZE) MemoryArena {
private:
	uint8_t* _currentBlock = nullptr; //当前指向的内存区域
	size_t _currentAllocSize = 0; //当前可以使用的最大内存数
	size_t _currentBlockPos = 0; //当前block下free内存位置
	const size_t blockSize; //默认的block分配大小
	std::list<std::pair<size_t, uint8_t*>> _usedBlocks; //存放满了的block
	std::list<std::pair<size_t, uint8_t*>> _availableBlocks; //存放可以使用的block

public:
	MemoryArena(size_t blockSize = 262144/*256kb*/) :
			blockSize(blockSize) {
	}
	//分配内存
	void * Alloc(size_t nBytes) {
		//1.让nBytes 16位对齐
		//2.寻找可用的block,没有的话创建新的block
		//3.从block中分配相应的内存出去

		//1.让nBytes 16位对齐
		nBytes = ((nBytes + 15) & (~15));
		//2.寻找可用的block,没有的话创建新的block
		if (_currentBlockPos + nBytes > _currentAllocSize) {
			//当前的block不足够分配nBytes相应的内存的时候
			if (_currentBlock) {
				//保存已经满了的block
				_usedBlocks.push_back(
						std::make_pair(_currentAllocSize, _currentBlock));
				_currentAllocSize = 0;
				_currentBlock = nullptr;
			}
			for (auto it = _availableBlocks.begin();
					it != _availableBlocks.end(); ++it) {
				if (it->first > nBytes) {
					_currentAllocSize = it->first;
					_currentBlock = it->second;
					_availableBlocks.erase(it);
					break;
				}
			}
			if (!_currentBlock) {
				_currentAllocSize = std::max(nBytes, blockSize);
				_currentBlock = AllocAligned<uint8_t>(_currentAllocSize);
			}
			_currentBlockPos = 0;
		}

		//返回指向相应空间的指针
		uint8_t* ptr = _currentBlock + _currentBlockPos;
		_currentBlockPos = _currentBlockPos + nBytes;
		return ptr;
	}

	template<typename T>
	T* Alloc(size_t nBytes=1,
			bool runConstructor = true/*这个参数决定是否调用T类型的默认构造函数*/) {
		T* ptr = (T*) Alloc(nBytes * sizeof(T));
		if (runConstructor) {
			for (size_t i = 0; i < nBytes; ++i) {
				new (&ptr[i]) T();
			}
		}
		return ptr;
	}

	//reset方法非常简单
	void Reset() {
		_currentBlockPos = 0;
		_availableBlocks.splice(_availableBlocks.begin(), _usedBlocks);
	}

	//析构函数
	~MemoryArena() {
		FreeAligned(_currentBlock);		//释放当前block
		//释放所有已经使用的block
		for (auto& block : _usedBlocks) {
			FreeAligned(block.second);
		}
		//释放所有可用的block
		for (auto& block : _availableBlocks) {
			FreeAligned(block.second);
		}
	}

	size_t TotalAllocated() const {
		size_t total = _currentAllocSize;
		for (auto& block : _usedBlocks) {
			total += block.first;
		}
		for (auto& block : _availableBlocks) {
			total += block.first;
		}
		return total;
	}
private:
	//删除赋值和复制函数
	MemoryArena(const MemoryArena&) = delete;
	MemoryArena& operator=(const MemoryArena&) = delete;

};


//在内存分布上是以block分布来储存的2D数组,可以降低访问临近内存区域时候的时候的cache miss概率
template<typename T,int logBlockSize>
class BlockedArray{
private:
	T* _data;
	int _uRes,_vRes;
	int _uBlocks;
	int RoundUp(int v) const{
		return (v+BlockSize()-1)&~(BlockSize()-1);
	}
public:
	BlockedArray(int uRes,int vRes,const T* raw=nullptr):_uRes(uRes),_vRes(vRes),_uBlocks(RoundUp(uRes)>>logBlockSize){
		
		int nAlloc=RoundUp(uRes)*RoundUp(vRes);
		_data=AllocAligned<T>(nAlloc);//分配空间
		for(int i=0;i<nAlloc;++i){
			//初始化数据
			new (&_data[i]) T();
		}
		//有初始化数据的情况
		if(raw){
			for(int i=0;i<uRes;++i){
				for(int j=0;j<vRes;++j){
					(*this)(i,j)=raw[j*_uRes+i];
				}
			}
		}
	}

	//返回一个block的大小(byte)
	constexpr int BlockSize() const{
		return (1<<logBlockSize);
	}

	//返回block编号
	int Block(int a) const{
		return a>>logBlockSize;
	}

	//返回在block中的offset
	int Offset(int a) const{
		return a&(BlockSize()-1);
	}

	T& operator()(int u,int v) {
		int uBlock=Block(u);
		int vBlock=Block(v);
		int uOffset=Offset(u);
		int vOffset=Offset(v);

		//block间的offset
		int offset=(vBlock*_uBlocks+uBlock)*BlockSize()*BlockSize();
		//block内的offset
		offset=offset+vOffset*BlockSize()+uOffset;
		
		return _data[offset];
	}

	const T& operator()(int u,int v) const{
		int uBlock=Block(u);
		int vBlock=Block(v);
		int uOffset=Offset(u);
		int vOffset=Offset(v);

		//block间的offset
		int offset=(vBlock*_uBlocks+uBlock)*BlockSize()*BlockSize();
		//block内的offset
		offset=offset+vOffset*BlockSize()+uOffset;
		
		return _data[offset];
	}

	int USize() const{
		return _uRes;
	}

	int VSize() const{
		return _vRes;
	}
};


#endif /* SRC_CORE_MEMORY_H_ */

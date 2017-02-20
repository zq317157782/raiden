#include "raiden.h"
#include "rng.h"
//反转2进制的位数(eg.b1000(8)=>b0001(1))
inline uint32_t ReverseBits32(uint32_t n) {
	//这里使用了2进制方便的位操作计算
	n = (n & 0xFFFF0000) >> 16 | (n & 0x0000FFFF) << 16;
	n = (n & 0xFF00FF00) >> 8 | (n & 0x00FF00FF) << 8;
	n = (n & 0xF0F0F0F0) >> 4 | (n & 0x0F0F0F0F) << 4;
	n = (n & 0xCCCCCCCC) >> 2 | (n & 0x33333333) << 2;
	n = (n & 0xAAAAAAAA) >> 1 | (n & 0x55555555) << 1;
	return n;
}

inline uint64_t ReverseBits64(uint64_t n) {
	return (ReverseBits32(n >> 32) | ReverseBits32(n) << 32);
}


//用来计算某个base下的RadicalInverse
//这里使用模板是为了减少以BASE为低的时候的除法操作
template <int base>
static Float RadicalInverseSpecialized(uint64_t a) {
	Float invBase = 1.0f / (Float)base;
	uint64_t reverse = 0;
	Float invBaseN = 1;
	while (a) {
		uint64_t next = a*invBase;
		uint64_t digit = a - next*base;
		reverse = reverse*base + digit;
		invBaseN *= invBase;
		a = next;
	}
	return std::min(reverse*invBaseN, OneMinusEpsilon);
}

Float RadicalInverse(int baseIndex, uint64_t a);
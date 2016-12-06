/*
 * rng.h
 *
 *  Created on: 2016年12月6日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_RNG_H_
#define SRC_CORE_RNG_H_
#include "raiden.h"

static const double DoubleOneMinusEpsilon = 0x1.fffffffffffffp-1;
static const float FloatOneMinusEpsilon = 0x1.fffffep-1;


#ifdef FLOAT_IS_DOUBLE
static const Float OneMinusEpsilon = DoubleOneMinusEpsilon;
#else
static const Float OneMinusEpsilon = FloatOneMinusEpsilon;
#endif

#define PCG32_DEFAULT_STATE 0x853c49e6748fea9bULL
#define PCG32_DEFAULT_STREAM 0xda3e39cb94b95bdbULL
#define PCG32_MULT 0x5851f42d4c957f2dULL
class RNG {
  public:
    // RNG Public Methods
    RNG();
    RNG(uint64_t sequenceIndex) { SetSequence(sequenceIndex); }
    void SetSequence(uint64_t sequenceIndex);
    uint32_t UniformUInt32();
    uint32_t UniformUInt32(uint32_t b) {
        uint32_t threshold = (~b + 1u) % b;
        while (true) {
            uint32_t r = UniformUInt32();
            if (r >= threshold) return r % b;
        }
    }
    Float UniformFloat() {
        return std::min(OneMinusEpsilon, Float(UniformUInt32() * 0x1p-32f));
    }
    template <typename Iterator>
    void Shuffle(Iterator begin, Iterator end) {
        for (Iterator it = end - 1; it > begin; --it)
            std::iter_swap(it,
                           begin + UniformUInt32((uint32_t)(it - begin + 1)));
    }
    void Advance(int64_t idelta) {
        uint64_t cur_mult = PCG32_MULT, cur_plus = _inc, acc_mult = 1u,
                 acc_plus = 0u, delta = (uint64_t)idelta;
        while (delta > 0) {
            if (delta & 1) {
                acc_mult *= cur_mult;
                acc_plus = acc_plus * cur_mult + cur_plus;
            }
            cur_plus = (cur_mult + 1) * cur_plus;
            cur_mult *= cur_mult;
            delta /= 2;
        }
        _state = acc_mult * _state + acc_plus;
    }
    int64_t operator-(const RNG &other) const {
        Assert(_inc==other._inc);
        uint64_t cur_mult = PCG32_MULT, cur_plus = _inc, cur_state = other._state,
                 the_bit = 1u, distance = 0u;
        while (_state != cur_state) {
            if ((_state & the_bit) != (cur_state & the_bit)) {
                cur_state = cur_state * cur_mult + cur_plus;
                distance |= the_bit;
            }
            Assert((_state & the_bit)==(cur_state & the_bit));
            the_bit <<= 1;
            cur_plus = (cur_mult + 1ULL) * cur_plus;
            cur_mult *= cur_mult;
        }
        return (int64_t)distance;
    }

  private:
    uint64_t _state, _inc;
};

inline RNG::RNG() : _state(PCG32_DEFAULT_STATE), _inc(PCG32_DEFAULT_STREAM) {}
inline void RNG::SetSequence(uint64_t initseq) {
    _state = 0u;
    _inc = (initseq << 1u) | 1u;
    UniformUInt32();
    _state += PCG32_DEFAULT_STATE;
    UniformUInt32();
}

inline uint32_t RNG::UniformUInt32() {
    uint64_t oldstate = _state;
    _state = oldstate * PCG32_MULT + _inc;
    uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = (uint32_t)(oldstate >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
}



#endif /* SRC_CORE_RNG_H_ */

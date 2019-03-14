#pragma once
#include <xmmintrin.h>//sse
#include <emmintrin.h>//sse2

#define RAIDEN_SSE_ALIGNAS alignas(16)

// //SSE4.1's _mm_blendv_ps 
// __m128 _sse_blendv_ps(__m128 a, __m128 b, __m128 mask){
//     return _mm_or_ps(_mm_and_ps(mask,b),_mm_andnot_ps(mask,a));
// }
#include <benchmark.h>
#include "raiden.h"
#include "mmath.h"
static void BM_PowInteger(benchmark::State& state) {
  Float i=0;
  for (auto _ : state){   
      benchmark::DoNotOptimize(i+=Pow<1000>(2));
  }
   
}
// Register the function as a benchmark
BENCHMARK(BM_PowInteger);


static void BM_Pow_STD(benchmark::State& state) {
  Float i=0;
  for (auto _ : state){
      benchmark::DoNotOptimize(i+=std::pow(2,1000));
  }
   
}
// Register the function as a benchmark
BENCHMARK(BM_Pow_STD);




FINLINE uint32_t FloatToBits_v1(float f)
{
    Assert(!std::isnan(f));
    uint32_t bits = 0;
    std::memcpy(&bits, &f, sizeof(float));
    return bits;
}

static void BM_FloatToBits_v1(benchmark::State& state) {
  uint32_t i=0;
  for (auto _ : state){
      benchmark::DoNotOptimize(i+=FloatToBits_v1(1000.0f));
  }
}

BENCHMARK(BM_FloatToBits_v1);
FINLINE uint32_t FloatToBits_v2(float f)
{
    Assert(!std::isnan(f));
    union
    {
      float f;
      uint32_t ui;
    } v;

    v.f=f;
    return v.ui;
}

static void BM_FloatToBits_v2(benchmark::State& state) {
  uint32_t i=0;
  for (auto _ : state){
      benchmark::DoNotOptimize(i+=FloatToBits_v1(1000.0f));
  }
}


BENCHMARK(BM_FloatToBits_v2);




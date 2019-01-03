#include <benchmark.h>
#include "raiden.h"
static void BM_PowInteger(benchmark::State& state) {
  for (auto _ : state){
      Float i=0;
      benchmark::DoNotOptimize(i+=Pow<10>(2));
  }
   
}
// Register the function as a benchmark
BENCHMARK(BM_PowInteger);


static void BM_Pow(benchmark::State& state) {
  for (auto _ : state){
      Float i=0;
      benchmark::DoNotOptimize(i+=std::pow(2,10));
  }
   
}
// Register the function as a benchmark
BENCHMARK(BM_Pow);

//
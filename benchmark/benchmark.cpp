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


static void BM_Pow(benchmark::State& state) {
  Float i=0;
  for (auto _ : state){
      benchmark::DoNotOptimize(i+=std::pow(2,1000));
  }
   
}
// Register the function as a benchmark
BENCHMARK(BM_Pow);

//
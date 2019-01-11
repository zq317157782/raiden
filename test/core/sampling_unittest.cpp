#include "raiden.h"
#include "gtest.h"
#include "sampling.h"


TEST(TrimmedLogisticPdf,Normalizition){
    int count=30000;
    RNG rng;
    Float sum=0;
    for(int i=0;i<count;++i){
        Float x=rng.UniformFloat();
        sum+=TrimmedLogisticPdf(x,1,0,1);
    }
    sum/=count;
    EXPECT_GT(sum,0.99);
    EXPECT_LT(sum,1.01);
}
#include "gtest.h"
#include "bssrdf.h"
#include <iostream>
#include <fstream>
#include "sampling.h"
#include "interpolation.h"

TEST(DuffusionEquation,ComputeBeamDiffusionBSSRDF){
    BSSRDFTable table(8,8);
    ComputeBeamDiffusionBSSRDF(0,1.5,&table);
    //非线性分布，集中在1
    //(1-std::exp(-8*i/(Float)(7)))/(1-std::exp(-8.0f))
    EXPECT_FLOAT_EQ(table.albedoSamples[0],(1-std::exp(-8*0/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[1],(1-std::exp(-8*1/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[2],(1-std::exp(-8*2/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[3],(1-std::exp(-8*3/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[4],(1-std::exp(-8*4/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[5],(1-std::exp(-8*5/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[6],(1-std::exp(-8*6/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[7],1);
    //指数分布
    EXPECT_FLOAT_EQ(table.radiusSamples[0],0);
    EXPECT_FLOAT_EQ(table.radiusSamples[1],0.0024999999);
    EXPECT_FLOAT_EQ(table.radiusSamples[2],0.003);
    EXPECT_FLOAT_EQ(table.radiusSamples[3],0.0036000002);
    EXPECT_FLOAT_EQ(table.radiusSamples[4],0.0043200003);
    EXPECT_FLOAT_EQ(table.radiusSamples[5],0.0051840004);
    EXPECT_FLOAT_EQ(table.radiusSamples[6],0.0062208008);
    EXPECT_FLOAT_EQ(table.radiusSamples[7],0.0074649611);
}

TEST(SeparableBSSRDFA,Sw){
    auto Sw=[](const Vector3f& wi,Float eta){
         float c=1-2*FresnelMoment1(1/eta);
        //wi是BSSRDF局部空间的
        return (1-FrDielectric(CosTheta(wi),1,eta))/(c*Pi);
    };
    RNG rng;
    Spectrum sum=0;
    const int numSample=30000;
    for(int i=0;i<numSample;++i){
        auto wi= UniformSampleHemisphere({rng.UniformFloat(),rng.UniformFloat()});
        auto f=Sw(wi,1.5);
        sum=sum+f*AbsCosTheta(wi);
    }
    sum=sum/numSample/UniformHemispherePdf();
    Float rgb[3];
    sum.ToRGB(rgb);
    EXPECT_LE(rgb[0],1.01);
    EXPECT_GE(rgb[0],0.99);

}


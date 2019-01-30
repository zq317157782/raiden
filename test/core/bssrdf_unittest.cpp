#include "gtest.h"
#include "bssrdf.h"
TEST(DuffusionEquation,ComputeBeamDiffusionBSSRDF){
    BSSRDFTable table(8,8);
    ComputeBeamDiffusionBSSRDF(0,1.5,&table);
    //非线性分布，集中在1
    EXPECT_FLOAT_EQ(table.albedoSamples[0],0);
    EXPECT_FLOAT_EQ(table.albedoSamples[1],0.63233268);
    EXPECT_FLOAT_EQ(table.albedoSamples[2],0.86495489);
    EXPECT_FLOAT_EQ(table.albedoSamples[3],0.95053178);
    EXPECT_FLOAT_EQ(table.albedoSamples[4],0.98201376);
    EXPECT_FLOAT_EQ(table.albedoSamples[5],0.99359536);
    EXPECT_FLOAT_EQ(table.albedoSamples[6],0.99785602);
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

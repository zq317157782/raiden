#include "raiden.h"
#include "interpolation.h"
#include "gtest.h"
TEST(CatmullRomWeights,CatmullRomWeights){
   Float nodes[5]={-1,0,1,2,3};
   Float weights[4];
   int offset;
  
   CatmullRomWeights(5,nodes,0,&offset,weights);
   EXPECT_EQ(offset,0);
   EXPECT_EQ(weights[0],0);
   EXPECT_EQ(weights[1],1);
   EXPECT_EQ(weights[2],0);
   EXPECT_EQ(weights[3],0);

}

TEST(CatmullRomWeights,CatmullRomWeights2){
   Float nodes[5]={-1,0,1,2,3};
   Float weights[4];
   int offset;
  
   CatmullRomWeights(5,nodes,1,&offset,weights);
   EXPECT_EQ(offset,1);
   EXPECT_EQ(weights[0],0);
   EXPECT_EQ(weights[1],1);
   EXPECT_EQ(weights[2],0);
   EXPECT_EQ(weights[3],0);

}

TEST(CatmullRomWeights,LeftEnd){
   Float nodes[4]={0,1,2,3};
   Float weights[4];
   int offset;
  
   CatmullRomWeights(4,nodes,0,&offset,weights);
   EXPECT_EQ(offset,-1);
   EXPECT_EQ(weights[0],0);
   EXPECT_EQ(weights[1],1);
   EXPECT_EQ(weights[2],0);
   EXPECT_EQ(weights[3],0);

}


TEST(CatmullRomWeights,RightEnd){
   Float nodes[4]={0,1,2,3};
   Float weights[4];
   int offset;
  
   CatmullRomWeights(4,nodes,3,&offset,weights);
   EXPECT_EQ(offset,1);
   EXPECT_EQ(weights[0],0);
   EXPECT_EQ(weights[1],0);
   EXPECT_EQ(weights[2],1);
   EXPECT_EQ(weights[3],0);

}

TEST(SampleCatmullRom,SampleCatmullRom){
    Float fval;
    Float pdf;

    Float x[4]={1,2,3,4};
    Float f[4]={1,2,3,4};
    Float F[4]={1,3,6,10};
    Float ret=SampleCatmullRom(4,x,f,F,0.5f,&fval,&pdf);
    EXPECT_EQ(ret,2);
    EXPECT_EQ(fval,2);
    EXPECT_EQ(pdf,0.2f);

    ret=SampleCatmullRom(4,x,f,F,0.75f,&fval,&pdf);
    EXPECT_EQ(ret,3);
    EXPECT_EQ(fval,3);
    EXPECT_EQ(pdf,3/(Float)10);
}

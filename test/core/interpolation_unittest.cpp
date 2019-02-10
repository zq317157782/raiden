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

TEST(SampleCatmullRom,Uniform){
    Float fval;
    Float pdf;

    Float x[5]={1,2,3,4,5};
    Float f[5]={1,1,1,1,1};
    Float F[5]={1,2,3,4,5};
    Float ret=SampleCatmullRom(5,x,f,F,0.5f,&fval,&pdf);
    EXPECT_EQ(ret,2.5);
    EXPECT_EQ(fval,1);
    EXPECT_FLOAT_EQ(pdf,0.2);
}


TEST(SampleCatmullRom2D,Uniform){
    Float fval;
    Float pdf;

    Float x[5]={1,2,3,4,5};
    Float y[5]={1,2,3,4,5};
    Float f[25]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    Float F[25]={1,2,3,4,5,1,2,3,4,5,1,2,3,4,5,1,2,3,4,5,1,2,3,4,5};
    Float ret=SampleCatmullRom2D(5,5,x,y,f,F,3,0.5f,&fval,&pdf);
    EXPECT_EQ(ret,2.5);
    EXPECT_EQ(fval,1);
    EXPECT_FLOAT_EQ(pdf,0.2);
}


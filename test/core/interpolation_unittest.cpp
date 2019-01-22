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

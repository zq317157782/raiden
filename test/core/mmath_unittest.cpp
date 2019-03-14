#include "raiden.h"
#include "gtest.h"
#include "mmath.h"
TEST(Math,Clamp){
  auto r=Clamp(1,0.2,1.5f);
  EXPECT_EQ(r,1);
}

TEST(Math,Pow){
    EXPECT_EQ(Pow<10>(2),1024);
}

TEST(Math,SafeSqrt){
  EXPECT_EQ(SafeSqrt(-4),0);
  EXPECT_EQ(SafeSqrt(4),2);
}

TEST(Math,SafeASin){
  EXPECT_FLOAT_EQ(SafeASin(1),Pi/2);
  EXPECT_FLOAT_EQ(SafeASin(-1),-Pi/2);
}

TEST(Math,Rcp){
   Float a=2.0;
   Float b=Rcp(a);
   EXPECT_FLOAT_EQ(b,0.5f);
}

TEST(Math,Rsqrt){
   Float a=4.0;
   Float b=Rsqrt(a);
   EXPECT_FLOAT_EQ(b,0.5f);
}
#include "raiden.h"
#include "gtest.h"
TEST(Raiden,Clamp){
  auto r=Clamp(1,0.2,1.5f);
  EXPECT_EQ(r,1);
}

TEST(Raiden,Pow){
    EXPECT_EQ(Pow<10>(2),1024);
}

TEST(Raiden,SafeSqrt){
  EXPECT_EQ(SafeSqrt(-4),0);
  EXPECT_EQ(SafeSqrt(4),2);
}

TEST(Raiden,SafeASin){
  EXPECT_FLOAT_EQ(SafeASin(1),Pi/2);
  EXPECT_FLOAT_EQ(SafeASin(-1),-Pi/2);
}
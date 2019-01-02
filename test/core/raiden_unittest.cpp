#include "raiden.h"
#include "gtest.h"
TEST(Raiden,Clamp){
  auto r=Clamp(1,0.2,1.5f);
  EXPECT_EQ(r,1);
}

TEST(Raiden,Pow){
    EXPECT_EQ(Pow<10>(2),1024);
}
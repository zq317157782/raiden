#include "raiden.h"
#include "gtest.h"

TEST(FloatArithmetic,FloatToBits){
    float one=1;
    auto one_bits=FloatToBits(one);
    
    EXPECT_EQ(one_bits&0x3f800000,0x3f800000);
    EXPECT_EQ(one_bits&(~0x3f800000),0x00000000);

}


TEST(FloatArithmetic,BitsToFloat){
    uint32_t one_bits=0x3f800000;
    auto one=BitsToFloat(one_bits);
    EXPECT_FLOAT_EQ(one,1.0f);
}

TEST(FloatArithmetic,NextFloatUp){
    float one=1;
    float next_one=NextFloatUp(one);
    EXPECT_FLOAT_EQ(next_one,1.0000001f);
}

TEST(FloatArithmetic,NextFloatDown){
    float one=1;
    float next_one=NextFloatDown(one);
    EXPECT_FLOAT_EQ(next_one,0.99999994f);
}
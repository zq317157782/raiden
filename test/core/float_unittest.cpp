#include "raiden.h"
#include "gtest.h"
#include "errfloat.h"

TEST(Float,FloatToBits){
    float one=1;
    auto one_bits=FloatToBits(one);
    
    EXPECT_EQ(one_bits&0x3f800000,0x3f800000);
    EXPECT_EQ(one_bits&(~0x3f800000),0x00000000);

}


TEST(Float,BitsToFloat){
    uint32_t one_bits=0x3f800000;
    auto one=BitsToFloat(one_bits);
    EXPECT_FLOAT_EQ(one,1.0f);
}

TEST(Float,NextFloatUp){
    float one=1;
    float next_one=NextFloatUp(one);
    EXPECT_FLOAT_EQ(next_one,1.0000001f);
}

TEST(Float,NextFloatDown){
    float one=1;
    float next_one=NextFloatDown(one);
    EXPECT_FLOAT_EQ(next_one,0.99999994f);
}



TEST(EFloat,Arithmetic){
    EFloat a1=1;
    EFloat a2=1;
    auto sum=a1+a2;
    //EXPECT_FLOAT_EQ(sum,2);
    EXPECT_FLOAT_EQ(sum.LowerBound(),2);
    EXPECT_FLOAT_EQ(sum.UpperBound(),2);
}

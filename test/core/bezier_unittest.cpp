#include "raiden.h"
#include "bezier.h"
#include "gtest.h"
TEST(Bezier,Blossom){
    Point3f p0(0,0,0);
    Point3f p1(0.2,0.2,0.2);
    Point3f p2(0.8,0.8,0.8);
    Point3f p3(1,1,1);
    CubicBezier curve(p0,p1,p2,p3);

    auto p=Blossom(curve,0,0,0);
    EXPECT_FLOAT_EQ(p.x,0);
    EXPECT_FLOAT_EQ(p.y,0);
    EXPECT_FLOAT_EQ(p.z,0);

    p=Blossom(curve,0,0,1);
    EXPECT_FLOAT_EQ(p.x,0.2);
    EXPECT_FLOAT_EQ(p.y,0.2);
    EXPECT_FLOAT_EQ(p.z,0.2);

    p=Blossom(curve,0,1,1);
    EXPECT_FLOAT_EQ(p.x,0.8);
    EXPECT_FLOAT_EQ(p.y,0.8);
    EXPECT_FLOAT_EQ(p.z,0.8);

    p=Blossom(curve,1,1,1);
    EXPECT_FLOAT_EQ(p.x,1);
    EXPECT_FLOAT_EQ(p.y,1);
    EXPECT_FLOAT_EQ(p.z,1);
}


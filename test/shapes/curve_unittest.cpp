#include "raiden.h"
#include "gtest.h"
#include "shapes/curve.h"

TEST(Curve,ObjectBound){
    Transform t;
    auto common=std::make_shared<CurveCommon>(Point3f(0,0,0),Point3f(0.2,0,0),Point3f(0.8,0,0),Point3f(1,0,0),1,1,CurveType::FLAT);
    Curve curve(&t,&t,false,common,0,1);
    auto bound=curve.ObjectBound();
    EXPECT_FLOAT_EQ(bound.minPoint.x,-0.5);
    EXPECT_FLOAT_EQ(bound.minPoint.y,-0.5);
    EXPECT_FLOAT_EQ(bound.minPoint.z,-0.5);

    EXPECT_FLOAT_EQ(bound.maxPoint.x,1.5);
    EXPECT_FLOAT_EQ(bound.maxPoint.y,0.5);
    EXPECT_FLOAT_EQ(bound.maxPoint.z,0.5);
}
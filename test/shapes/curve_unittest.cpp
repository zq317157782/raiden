#include "raiden.h"
#include "gtest.h"
#include "geometry.h"
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


TEST(Curve,Intersect){
    Transform t;
    auto common=std::make_shared<CurveCommon>(Point3f(0,0,0),Point3f(-0.2,0,0),Point3f(-0.8,0,0),Point3f(-1,0,0),1,1,CurveType::FLAT);
    Curve curve(&t,&t,false,common,0,1);
    Ray ray;
    ray.o=Point3f(0,0,-1);
    ray.d=Vector3f(0,0,2);
    Float hit;
    SurfaceInteraction is;
    bool f=curve.Intersect(ray,&hit,&is);
    EXPECT_TRUE(f);
    EXPECT_FLOAT_EQ(hit,0.5);
    EXPECT_EQ(is.n,Normal3f(0,0,1));

    ray.o=Point3f(0,-1,0);
    ray.d=Vector3f(0,2,0);
    f=curve.Intersect(ray,&hit,&is);
    EXPECT_TRUE(f);
    EXPECT_FLOAT_EQ(hit,0.5);
    EXPECT_EQ(is.n,Normal3f(0,1,0));
}
#include "raiden.h"
#include "geometry.h"
#include "gtest.h"

TEST(Vector3f,Add){
  Vector3f v1(-1);
  Vector3f v2(1);
  auto v3=v1+v2;
  EXPECT_FLOAT_EQ(0.0, v3.x)<<"test operater+";
  //这里隐式调用了Vector3的构造函数
  v3+=0.5;
  EXPECT_FLOAT_EQ(0.5, v3.x)<<"test operater+=";
  v3+=std::numeric_limits<Float>::infinity();
  EXPECT_FLOAT_EQ(std::numeric_limits<Float>::infinity(), v3.x)<<"test infinity";
}

TEST(Vector3f,Minus){
  Vector3f v1(1);
  Vector3f v2(1);
  auto v3=v1-v2;
  EXPECT_FLOAT_EQ(0.0, v3.x)<<"test operater-";
  v3-=0.5;
  EXPECT_FLOAT_EQ(-0.5, v3.x)<<"test operater-=";
  v3-=std::numeric_limits<Float>::infinity();
  EXPECT_FLOAT_EQ(-std::numeric_limits<Float>::infinity(), v3.x)<<"test infinity";
}

TEST(Vector3f,Multi){
  Vector3f v1(2);
  Vector3f v2(5);
  auto v3=v1*5;
  EXPECT_FLOAT_EQ(10, v3.x)<<"test operater*";
  v3*=5;
  EXPECT_FLOAT_EQ(50, v3.x)<<"test operater*=";
  v3*=std::numeric_limits<Float>::infinity();
  EXPECT_FLOAT_EQ(std::numeric_limits<Float>::infinity(), v3.x)<<"test infinity";
}

TEST(Vector3f,Div){
  Vector3f v1(1);
  auto v2=v1/5;
  EXPECT_FLOAT_EQ(0.2, v2.x)<<"test operater/";
  v2/=0.2;
  EXPECT_FLOAT_EQ(1.0, v2.x)<<"test operater/=";
  v2/=std::numeric_limits<Float>::infinity();
  EXPECT_FLOAT_EQ(0, v2.x)<<"test infinity";
}

TEST(Vector3f,Conversion){
    Vector3f v1(1);
    auto p1=Point3f(v1);
    EXPECT_FLOAT_EQ(p1.x,v1.x)<<"test Vector3f to Point3f conversion";
    EXPECT_FLOAT_EQ(p1.y,v1.y)<<"test Vector3f to Point3f conversion";
    EXPECT_FLOAT_EQ(p1.z,v1.z)<<"test Vector3f to Point3f conversion";
    auto n1=Normal3f(v1);
    EXPECT_FLOAT_EQ(n1.x,v1.x)<<"test Normal3f to Point3f conversion";
    EXPECT_FLOAT_EQ(n1.y,v1.y)<<"test Normal3f to Point3f conversion";
    EXPECT_FLOAT_EQ(n1.z,v1.z)<<"test Normal3f to Point3f conversion";
}

TEST(Vector3f,Dot){
    Vector3f v1(1,1,1);
    Vector3f v2(5,6,7);
    auto v1Dotv2=Dot(v1,v2);
    EXPECT_FLOAT_EQ(18,v1Dotv2)<<"test Dot";
}

TEST(Vector3f,Cross){
    Vector3f v1(1,2,3);
    Vector3f v2(4,5,6);
    auto v3=Cross(v1,v2);
    EXPECT_EQ(Vector3f(-3,6,-3),v3)<<"test Cross";

    Vector3f v4(1,1,1);
    Vector3f v5(1,1,1);
    auto v6=Cross(v4,v5);
    EXPECT_EQ(Vector3f(0,0,0),v6)<<"test degenerate Cross ";
}
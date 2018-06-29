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

TEST(Vector3f,Index){
    Vector3f v1(1,2,3);
    EXPECT_FLOAT_EQ(1,v1[0])<<"test Index 0";
    EXPECT_FLOAT_EQ(2,v1[1])<<"test Index 1";
    EXPECT_FLOAT_EQ(3,v1[2])<<"test Index 2";
    v1[0]=0;
    v1[1]=0;
    v1[2]=0;
    EXPECT_FLOAT_EQ(0,v1[0])<<"test Index 0";
    EXPECT_FLOAT_EQ(0,v1[1])<<"test Index 1";
    EXPECT_FLOAT_EQ(0,v1[2])<<"test Index 2";
}

TEST(Vector3f,NSign){
     Vector3f v1(1);
     auto v2=-v1;
     EXPECT_FLOAT_EQ(-1,v2.x)<<"test negative sign";
}

TEST(Vector3f,Length){
    Vector3f v1(0.5,0.5,0.5);
    auto l=v1.Length();
    EXPECT_FLOAT_EQ(0.86602539,l)<<"test length op";
    auto l2=v1.LengthSquared();
    EXPECT_FLOAT_EQ(0.75,l2)<<"test length op";
}

TEST(Vector2f,Add){
  Vector2f v1(-1);
  Vector2f v2(1);
  auto v3=v1+v2;
  EXPECT_FLOAT_EQ(0.0, v3.x)<<"test operater+";
  //这里隐式调用了Vector3的构造函数
  v3+=0.5;
  EXPECT_FLOAT_EQ(0.5, v3.x)<<"test operater+=";
  v3+=std::numeric_limits<Float>::infinity();
  EXPECT_FLOAT_EQ(std::numeric_limits<Float>::infinity(), v3.x)<<"test infinity";
}

TEST(Vector2f,Minus){
  Vector2f v1(1);
  Vector2f v2(1);
  auto v3=v1-v2;
  EXPECT_FLOAT_EQ(0.0, v3.x)<<"test operater-";
  v3-=0.5;
  EXPECT_FLOAT_EQ(-0.5, v3.x)<<"test operater-=";
  v3-=std::numeric_limits<Float>::infinity();
  EXPECT_FLOAT_EQ(-std::numeric_limits<Float>::infinity(), v3.x)<<"test infinity";
}

TEST(Vector2f,Multi){
  Vector2f v1(2);
  Vector2f v2(5);
  auto v3=v1*5;
  EXPECT_FLOAT_EQ(10, v3.x)<<"test operater*";
  v3*=5;
  EXPECT_FLOAT_EQ(50, v3.x)<<"test operater*=";
  v3*=std::numeric_limits<Float>::infinity();
  EXPECT_FLOAT_EQ(std::numeric_limits<Float>::infinity(), v3.x)<<"test infinity";
}

TEST(Vector2f,Div){
  Vector2f v1(1);
  auto v2=v1/5;
  EXPECT_FLOAT_EQ(0.2, v2.x)<<"test operater/";
  v2/=0.2;
  EXPECT_FLOAT_EQ(1.0, v2.x)<<"test operater/=";
  v2/=std::numeric_limits<Float>::infinity();
  EXPECT_FLOAT_EQ(0, v2.x)<<"test infinity";
}

TEST(Vector2f,Conversion){
    Vector2f v1(1);
    auto p1=Point2f(v1);
    EXPECT_FLOAT_EQ(p1.x,v1.x)<<"test Vector3f to Point3f conversion";
    EXPECT_FLOAT_EQ(p1.y,v1.y)<<"test Vector3f to Point3f conversion";
}


TEST(Normal3f,Faceforward){
    Vector3f v=Vector3f(0.5,-0.5,0);
    Normal3f n=Normal3f(0,1,0);
    auto v2=Faceforward(v,n);
    EXPECT_EQ(Vector3f(-0.5,0.5,0),v2);
}

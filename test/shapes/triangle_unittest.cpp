#include "raiden.h"
#include "gtest.h"
#include "shapes/trianglemesh.h"
#include "interaction.h"

TEST(Triangle,UVToWorldPosition){
    Transform i;
    int indices[3]={0,1,2};
    Point3f vertices[3]={Point3f(0,0,0),Point3f(1,0,0),Point3f(0,1,0)};
    
    Point2f uv(0,0);
    auto tm=std::shared_ptr<TriangleMesh>(new TriangleMesh(i,1,indices,3,vertices,nullptr,nullptr,nullptr));
    Triangle triangle(&i,&i,false,tm,0);
    UVInteraction inte;
    bool flag=triangle.UVToWorld(uv,&inte);
    EXPECT_TRUE(flag);
    EXPECT_EQ(inte.p,Point3f(0,0,0));

     uv=Point2f(1,0);
    flag=triangle.UVToWorld(uv,&inte);

    EXPECT_TRUE(flag);
    EXPECT_EQ(inte.p,Point3f(1,0,0));
    EXPECT_EQ(inte.n,Normal3f(0,0,1));
}

TEST(Triangle,UVClamp){
    Transform i;
    int indices[3]={0,1,2};
    Point3f vertices[3]={Point3f(0,0,0),Point3f(1,0,0),Point3f(0,1,0)};

    auto tm=std::shared_ptr<TriangleMesh>(new TriangleMesh(i,1,indices,3,vertices,nullptr,nullptr,nullptr));
    Triangle triangle(&i,&i,false,tm,0);

   
    auto uv0=triangle.UVClamp(Point2f(0.5,0.5));
    EXPECT_EQ(uv0,Point2f(0.5,0.5));

    auto uv1=triangle.UVClamp(Point2f(-1,0));
    EXPECT_EQ(uv1,Point2f(0,0));

     auto uv2=triangle.UVClamp(Point2f(0,-1));
     EXPECT_EQ(uv2,Point2f(0,0));

     auto uv3=triangle.UVClamp(Point2f(2,2));
     EXPECT_EQ(uv3,Point2f(1,1));

     auto uv4=triangle.UVClamp(Point2f(2,-1));
     EXPECT_EQ(uv4,Point2f(1,0));

     auto uv5=triangle.UVClamp(Point2f(2,0.5));
     EXPECT_EQ(uv5,Point2f(1,0.5));

     auto uv6=triangle.UVClamp(Point2f(2,0.8));
     EXPECT_EQ(uv6,Point2f(1,0.8));

}

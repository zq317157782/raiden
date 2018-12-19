#include "raiden.h"
#include "gtest.h"
#include "shapes/trianglemesh.h"
#include "interaction.h"

TEST(Triangle,UVToWorldPosition){
    Transform i;
    int indices[3]={0,1,2};
    Point3f vertices[3]={Point3f(0,0,0),Point3f(1,0,0),Point3f(0,1,0)};
    
    Point2f uv(0,0);

    Triangle triangle(&i,&i,false,std::shared_ptr<TriangleMesh>(new TriangleMesh(i,1,indices,3,vertices,nullptr,nullptr,nullptr)),0);
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

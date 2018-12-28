#include "raiden.h"
#include "transform.h"
#include "gtest.h"

TEST(Transform,LookAt){
    auto t=LookAt(Point3f(0,0,0),Point3f(0,0,1),Vector3f(0,1,0));
    
    EXPECT_EQ(t.GetInverseMatrix().m[0][0],1);
    EXPECT_EQ(t.GetInverseMatrix().m[1][0],0);
    EXPECT_EQ(t.GetInverseMatrix().m[2][0],0);

    EXPECT_EQ(t.GetInverseMatrix().m[0][1],0);
    EXPECT_EQ(t.GetInverseMatrix().m[1][1],1);
    EXPECT_EQ(t.GetInverseMatrix().m[2][1],0);

    EXPECT_EQ(t.GetInverseMatrix().m[0][2],0);
    EXPECT_EQ(t.GetInverseMatrix().m[1][2],0);
    EXPECT_EQ(t.GetInverseMatrix().m[2][2],1);

    
}
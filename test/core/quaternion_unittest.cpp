#include "raiden.h"
#include "quaternion.h"
#include "transform.h"
#include "gtest.h"

TEST(Quaternion,Plus){
  Quaternion q1(1,1,1,1);
  Quaternion q2(2,2,2,2);
  auto q3=q1+q2;
  EXPECT_EQ(q3,Quaternion(3,3,3,3));
  q1+=Quaternion(-1,-1,-1,-1);
  EXPECT_EQ(q1,Quaternion(0,0,0,0));
}

TEST(Quaternion,Minus){
  Quaternion q1(1,1,1,1);
  Quaternion q2(2,2,2,2);
  auto q3=q1-q2;
  EXPECT_EQ(q3,Quaternion(-1,-1,-1,-1));
  q1-=Quaternion(-1,-1,-1,-1);
  EXPECT_EQ(q1,Quaternion(2,2,2,2));
}


TEST(Quaternion,Multi){
  Quaternion q1(1,1,1,1);
  auto q2=q1*2;
  EXPECT_EQ(q2,Quaternion(2,2,2,2));
  q1*=-1;
  EXPECT_EQ(q1,Quaternion(-1,-1,-1,-1));
  auto q3=2*q1;
  EXPECT_EQ(q3,Quaternion(-2,-2,-2,-2));
}



TEST(Quaternion,Div){
  Quaternion q1(1,1,1,1);
  auto q2=q1/2;
  EXPECT_EQ(q2,Quaternion(0.5,0.5,0.5,0.5));
  q1/=-1;
  EXPECT_EQ(q1,Quaternion(-1,-1,-1,-1));
}


TEST(Quaternion,Negetive){
  Quaternion q1(1,1,1,1);
  auto q2=-q1;
  EXPECT_EQ(q2,Quaternion(-1,-1,-1,-1));
}

TEST(Quaternion,Dot){
  Quaternion q1(1,1,1,1);
  Quaternion q2(2,2,2,2);
  float cosTheta=Dot(q1,q2);
  EXPECT_FLOAT_EQ(cosTheta,8);
}

TEST(Quaternion,Normalize){
  Quaternion q1(1,1,1,1);
  auto q2=Normalize(q1);
  EXPECT_EQ(q2,Quaternion(0.5,0.5,0.5,0.5));
}

TEST(Quaternion,Slerp){
  Quaternion q1(0,0,0,1);//绕x轴旋转0度
  Quaternion q2(1,0,0,0);//绕x轴旋转180度
  auto q3=Slerp(0.5,q1,q2);//绕x轴旋转90度
  

  EXPECT_EQ(q3,Quaternion(std::sin(Pi/4)*Vector3f(1,0,0),std::cos(Pi/4)));
}

TEST(Quaternion, ToTransform) {
	Quaternion q1(1, 0, 0, 0);//绕x轴旋转180度
	Transform t = q1.ToTransform();
	Matrix4x4 mat = t.GetMatrix();
	EXPECT_EQ(mat.m[1][1], -1);
	EXPECT_EQ(mat.m[1][2], 0);
	EXPECT_EQ(mat.m[2][1], 0);
	EXPECT_EQ(mat.m[2][2], -1);
}
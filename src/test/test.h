/*
 * test.cpp
 *
 *  Created on: 2016年11月10日
 *      Author: zhuqian
 */

#ifndef SRC_TEST_TEST_CPP_
#define SRC_TEST_TEST_CPP_
#ifdef DEBUG_BUILD
#include "gtest/gtest.h"

#include "geometry.h"
TEST(Vector3,add) {
	Vector3<float> v1(1,1,1);
	Vector3<float> v2=v1+v1;
	Vector3<float> v3(2,2,2);
	ASSERT_EQ(v2,v3);
	Vector3<float> v4(0,0,0);
	Vector3<float> v5(0,0,0);
	v4+=v4;
	ASSERT_EQ(v4,v5);
}

TEST(Vector3,sub) {
	Vector3<float> v1(1,1,1);
	Vector3<float> v2=v1-v1;
	Vector3<float> v3(0,0,0);
	ASSERT_EQ(v2,v3);
	Vector3<float> v4(0,0,0);
	Vector3<float> v5(-1,-1,-1);
	v4-=Vector3<float>(1,1,1);
	ASSERT_EQ(v4,v5);
}

TEST(Vector3,mul) {
	Vector3<float> v1(2,2,2);
	Vector3<float> v2=v1*2;
	Vector3<float> v3(4,4,4);
	ASSERT_EQ(v2,v3);
	Vector3<float> v4(10,10,10);
	Vector3<float> v5(0,0,0);
	v4*=0;
	ASSERT_EQ(v4,v5);
}

TEST(Vector3,div) {
	Vector3<float> v1(2,2,2);
	Vector3<float> v2=v1/2;
	Vector3<float> v3(1,1,1);
	ASSERT_EQ(v2,v3);
	v1/=2;
	ASSERT_EQ(v1,v3);
}

TEST(Vector3,length){
	Vector3<float> v1(2,2,2);
	float magnitude=v1.Magnitude();
	ASSERT_LE(magnitude-std::sqrt(2.0*2.0+2.0*2.0+2.0*2.0),0.1E-10);
	float magnitudeSquared=v1.MagnitudeSquared();
	ASSERT_LE(magnitudeSquared-(2.0*2.0+2.0*2.0+2.0*2.0),0.1E-10);
}

TEST(Vector3,index){
	Vector3<float> v1(1,2,3);
	ASSERT_EQ(v1[0],1);
	ASSERT_EQ(v1[1],2);
	ASSERT_EQ(v1[2],3);
	v1[0]=3;
	v1[1]=2;
	v1[2]=1;
	ASSERT_EQ(v1[0],3);
	ASSERT_EQ(v1[1],2);
	ASSERT_EQ(v1[2],1);
}

void UnitTest(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
}

#endif
#endif /* SRC_TEST_TEST_CPP_ */

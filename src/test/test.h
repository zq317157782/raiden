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
	Vector3<float> v1(1, 1, 1);
	Vector3<float> v2 = v1 + v1;
	Vector3<float> v3(2, 2, 2);
	ASSERT_EQ(v2, v3);
	Vector3<float> v4(0, 0, 0);
	Vector3<float> v5(0, 0, 0);
	v4 += v4;
	ASSERT_EQ(v4, v5);
}

TEST(Vector3,sub) {
	Vector3<float> v1(1, 1, 1);
	Vector3<float> v2 = v1 - v1;
	Vector3<float> v3(0, 0, 0);
	ASSERT_EQ(v2, v3);
	Vector3<float> v4(0, 0, 0);
	Vector3<float> v5(-1, -1, -1);
	v4 -= Vector3<float>(1, 1, 1);
	ASSERT_EQ(v4, v5);
}

TEST(Vector3,mul) {
	Vector3<float> v1(2, 2, 2);
	Vector3<float> v2 = v1 * 2;
	Vector3<float> v3(4, 4, 4);
	ASSERT_EQ(v2, v3);
	Vector3<float> v4(10, 10, 10);
	Vector3<float> v5(0, 0, 0);
	v4 *= 0;
	ASSERT_EQ(v4, v5);
}

TEST(Vector3,div) {
	Vector3<float> v1(2, 2, 2);
	Vector3<float> v2 = v1 / 2;
	Vector3<float> v3(1, 1, 1);
	ASSERT_EQ(v2, v3);
	v1 /= 2;
	ASSERT_EQ(v1, v3);
}

TEST(Vector3,length) {
	Vector3<float> v1(2, 2, 2);
	float magnitude = v1.Magnitude();
	ASSERT_LE(magnitude - std::sqrt(2.0 * 2.0 + 2.0 * 2.0 + 2.0 * 2.0),
			0.1E-10);
	float magnitudeSquared = v1.MagnitudeSquared();
	ASSERT_LE(magnitudeSquared - (2.0 * 2.0 + 2.0 * 2.0 + 2.0 * 2.0), 0.1E-10);
}

TEST(Vector3,index) {
	Vector3<float> v1(1, 2, 3);
	ASSERT_EQ(v1[0], 1);
	ASSERT_EQ(v1[1], 2);
	ASSERT_EQ(v1[2], 3);
	v1[0] = 3;
	v1[1] = 2;
	v1[2] = 1;
	ASSERT_EQ(v1[0], 3);
	ASSERT_EQ(v1[1], 2);
	ASSERT_EQ(v1[2], 1);
}

TEST(Vector3,eq) {
	Vector3<float> v1(0.0, 0.0, 0.0);
	Vector3<float> v2(v1);
	ASSERT_EQ(v1, v2);
}

TEST(Vector2,add) {
	Vector2<float> v1(1, 1);
	Vector2<float> v2 = v1 + v1;
	Vector2<float> v3(2, 2);
	ASSERT_EQ(v2, v3);
	Vector2<float> v4(0, 0);
	Vector2<float> v5(0, 0);
	v4 += v4;
	ASSERT_EQ(v4, v5);
}

TEST(Vector2,sub) {
	Vector2<float> v1(1, 1);
	Vector2<float> v2 = v1 - v1;
	Vector2<float> v3(0, 0);
	ASSERT_EQ(v2, v3);
	Vector2<float> v4(0, 0);
	Vector2<float> v5(-1, -1);
	v4 -= Vector2<float>(1, 1);
	ASSERT_EQ(v4, v5);
}

TEST(Vector2,mul) {
	Vector2<float> v1(2, 2);
	Vector2<float> v2 = v1 * 2;
	Vector2<float> v3(4, 4);
	ASSERT_EQ(v2, v3);
	Vector2<float> v4(10, 10);
	Vector2<float> v5(0, 0);
	v4 *= 0;
	ASSERT_EQ(v4, v5);
}

TEST(Vector2,div) {
	Vector2<float> v1(2, 2);
	Vector2<float> v2 = v1 / 2;
	Vector2<float> v3(1, 1);
	ASSERT_EQ(v2, v3);
	v1 /= 2;
	ASSERT_EQ(v1, v3);
}

TEST(Vector2,length) {
	Vector2<float> v1(2, 2);
	float magnitude = v1.Magnitude();
	ASSERT_LE(magnitude - std::sqrt(2.0 * 2.0 + 2.0 * 2.0 + 2.0 * 2.0),
			0.1E-10);
	float magnitudeSquared = v1.MagnitudeSquared();
	ASSERT_LE(magnitudeSquared - (2.0 * 2.0 + 2.0 * 2.0 + 2.0 * 2.0), 0.1E-10);
}

TEST(Vector2,index) {
	Vector2<float> v1(1, 2);
	ASSERT_EQ(v1[0], 1);
	ASSERT_EQ(v1[1], 2);
	v1[0] = 3;
	v1[1] = 2;
	ASSERT_EQ(v1[1], 2);
	ASSERT_EQ(v1[0], 3);
}

TEST(Vector2,eq) {
	Vector2<float> v1(0.0, 0.0);
	Vector2<float> v2(v1);
	ASSERT_EQ(v1, v2);
}

TEST(Point3,add) {
	Point3<int> p1(1, 1, 1);
	Point3<int> p2 = p1 + p1;
	p2 += p1;
	Point3<int> p3(3, 3, 3);
	ASSERT_EQ(p2, p3);
}

TEST(Point3,sub) {
	Point3<int> p1(1, 1, 1);
	Vector3<int> p2 = p1 - p1;
	Vector3<int> v1(0, 0, 0);
	ASSERT_EQ(p2, v1);
}

TEST(Point3,mul) {
	Point3<int> p1(1, 1, 1);
	Point3<int> p2 = p1 * 2;
	p2 *= 2;
	Point3<int> p3(4, 4, 4);
	ASSERT_EQ(p2, p3);
}

TEST(Point3,div) {
	Point3<int> p1(1, 1, 1);
	Point3<int> p2 = p1 / 2;
	p2 /= 2;
	Point3<int> p3(0, 0, 0);
	ASSERT_EQ(p2, p3);

}

TEST(Point3,vector) {
	Point3<int> p1(1, 1, 1);
	Vector3<int> v1(1, 1, 1);
	Point3<int> p2 = p1 + v1;
	p2 += v1;
	Point3<int> p3(3, 3, 3);
	ASSERT_EQ(p2, p3);
}

TEST(Point3,index) {
	Point3<int> p1(1, 2, 3);
	ASSERT_EQ(p1[0], 1);
	ASSERT_EQ(p1[1], 2);
	ASSERT_EQ(p1[2], 3);
	p1[0] = 3;
	p1[1] = 2;
	p1[2] = 1;
	ASSERT_EQ(p1[0], 3);
	ASSERT_EQ(p1[1], 2);
	ASSERT_EQ(p1[2], 1);
}

TEST(VectorOp,distance) {
	Vector2<float> v1(1, 0);
	Vector2<float> v2(0, 1);
	ASSERT_EQ(Dot(v1, v2), 0);

	Vector2<float> v3(1, 0);
	Vector2<float> v4(-1, 0);
	ASSERT_EQ(AbsDot(v3, v4), 1);

	Vector3f biasX(1, 0, 0);
	Vector3f biasY(0, 1, 0);
	Vector3f biasZ = Cross(biasX, biasY);
	ASSERT_EQ(biasZ, Vector3f(0, 0, 1));

	Vector3f unnormalV(4, 0, 0);
	Vector3f normalV = Normalize(unnormalV);
	ASSERT_EQ(normalV, Vector3f(1, 0, 0));
	Vector3f vv = 6 * unnormalV;
	ASSERT_EQ(vv, Vector3f(24, 0, 0));

}

TEST(PointOp,distance) {
	Point3<float> p1(2, 2, 2);
	Point3<float> p2(0, 0, 0);
	ASSERT_EQ(DistanceSquared(p1, p2), 12);
	ASSERT_LE(Distance(p1, p2) - sqrt(12), 0.1e-10);
}

TEST(FloatOp,all) {
	float f = 0;
	f = NextFloatUp(f);
	ASSERT_EQ(f, BitsToFloat((uint32_t )0x00000001));
	f = NextFloatDown(f);
	ASSERT_EQ(f, 0);

	double d = 0;
	d = NextFloatUp(d);
	ASSERT_EQ(d, BitsToFloat((uint64_t )0x0000000000000001));

	ASSERT_NE(gamma(1), 1);
//ASSERT_EQ(MachineEpsion,0);
}

#include "errfloat.h"
TEST(ErrFloat,all) {
	ErrFloat f(1);
	ErrFloat f2(1);
	ErrFloat f3 = f + f2;
	ASSERT_EQ(f3.UpperBound(), NextFloatUp(2.0f));
	ASSERT_EQ(f3.LowerBound(), NextFloatDown(2.0f));
	ErrFloat f4 = f - f2;
	ASSERT_EQ(f4.UpperBound(), NextFloatUp(0.0f));
	//ASSERT_EQ(f4.LowerBound(),NextFloatDown(0.0f));//这里直接返回nan了，因为0的下一个float就是nan
	ErrFloat f5 = f * f2;
	ASSERT_EQ(f5.UpperBound(), NextFloatUp(1.0f));
	ASSERT_EQ(f5.LowerBound(), NextFloatDown(1.0f));
	ErrFloat f6 = f / f2;
	ASSERT_EQ(f6.UpperBound(), NextFloatUp(1.0f));
	ASSERT_EQ(f6.LowerBound(), NextFloatDown(1.0f));
	ErrFloat f7 = -f;
	ASSERT_EQ(f7.UpperBound(), -1.0f);
}

TEST(Normal3,add) {
	Normal3<float> v1(1, 1, 1);
	Normal3<float> v2 = v1 + v1;
	Normal3<float> v3(2, 2, 2);
	ASSERT_EQ(v2, v3);
	Normal3<float> v4(0, 0, 0);
	Normal3<float> v5(0, 0, 0);
	v4 += v4;
	ASSERT_EQ(v4, v5);
}

TEST(Normal3,sub) {
	Normal3<float> v1(1, 1, 1);
	Normal3<float> v2 = v1 - v1;
	Normal3<float> v3(0, 0, 0);
	ASSERT_EQ(v2, v3);
	Normal3<float> v4(0, 0, 0);
	Normal3<float> v5(-1, -1, -1);
	v4 -= Normal3<float>(1, 1, 1);
	ASSERT_EQ(v4, v5);
}

TEST(Normal3,mul) {
	Normal3<float> v1(2, 2, 2);
	Normal3<float> v2 = v1 * 2;
	Normal3<float> v3(4, 4, 4);
	ASSERT_EQ(v2, v3);
	Normal3<float> v4(10, 10, 10);
	Normal3<float> v5(0, 0, 0);
	v4 *= 0;
	ASSERT_EQ(v4, v5);
}

TEST(Normal3,div) {
	Normal3<float> v1(2, 2, 2);
	Normal3<float> v2 = v1 / 2;
	Normal3<float> v3(1, 1, 1);
	ASSERT_EQ(v2, v3);
	v1 /= 2;
	ASSERT_EQ(v1, v3);
}

TEST(Normal3,length) {
	Normal3<float> v1(2, 2, 2);
	float magnitude = v1.Length();
	ASSERT_LE(magnitude - std::sqrt(2.0 * 2.0 + 2.0 * 2.0 + 2.0 * 2.0),
			0.1E-10);
	float magnitudeSquared = v1.LengthSquared();
	ASSERT_LE(magnitudeSquared - (2.0 * 2.0 + 2.0 * 2.0 + 2.0 * 2.0), 0.1E-10);
}

TEST(Normal3,index) {
	Normal3<float> v1(1, 2, 3);
	ASSERT_EQ(v1[0], 1);
	ASSERT_EQ(v1[1], 2);
	ASSERT_EQ(v1[2], 3);
	v1[0] = 3;
	v1[1] = 2;
	v1[2] = 1;
	ASSERT_EQ(v1[0], 3);
	ASSERT_EQ(v1[1], 2);
	ASSERT_EQ(v1[2], 1);
}

TEST(Normal3,eq) {
	Normal3<float> v1(0.0, 0.0, 0.0);
	Normal3<float> v2(v1);
	ASSERT_EQ(v1, v2);
}

TEST(Bound3,all) {
//ASSERT_EQ(std::numeric_limits<float>::lowest(),std::numeric_limits<float>::min());
	Point3<Float> p1(1, 1, 1);
	Point3<Float> p2(2, 2, 2);
	Bound3<Float> bound(p1, p2);
	ASSERT_EQ(bound[0], p1);
	ASSERT_EQ(bound[1], p2);
	bound[1] = p1;
	ASSERT_EQ(bound[1], p1);
	bound[0] = Point3<Float>(0, 1, 2);
	bound[1] = Point3<Float>(3, 4, 5);
	ASSERT_EQ(bound.Corner(0), Point3<Float>(0, 1, 2));
	ASSERT_EQ(bound.Corner(1), Point3<Float>(3, 1, 2));
	ASSERT_EQ(bound.Corner(2), Point3<Float>(0, 4, 2));
	ASSERT_EQ(bound.Corner(3), Point3<Float>(3, 4, 2));
	ASSERT_EQ(bound.Corner(4), Point3<Float>(0, 1, 5));
	ASSERT_EQ(bound.Corner(5), Point3<Float>(3, 1, 5));
	ASSERT_EQ(bound.Corner(6), Point3<Float>(0, 4, 5));
	ASSERT_EQ(bound.Corner(7), Point3<Float>(3, 4, 5));
	bound = Union(bound, Point3<Float>(6, 6, 6));
	ASSERT_EQ(bound.Corner(0), Point3<Float>(0, 1, 2));
	ASSERT_EQ(bound.Corner(7), Point3<Float>(6, 6, 6));

	Point3<Float> b1p1(1, 1, 1);
	Point3<Float> b1p2(2, 2, 2);
	Bound3<Float> b1(b1p1, b1p2);
	Point3<Float> b2p1(2, 2, 2);
	Point3<Float> b2p2(3, 3, 3);
	Bound3<Float> b2(b2p1, b2p2);
	Bound3<Float> b3=Union(b1,b2);
	ASSERT_EQ(b3[0], Point3<Float>(1, 1, 1));
	ASSERT_EQ(b3[1], Point3<Float>(3, 3, 3));
}

void UnitTest(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
}

#endif
#endif /* SRC_TEST_TEST_CPP_ */

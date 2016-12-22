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

TEST(Vector3,util){
	Vector3f v(0.5f,0.5f,0.5f);
	Vector3f v1=Ceil(v);
	ASSERT_EQ(v1,Vector3f(1,1,1));
	Vector3f v2=Floor(v);
	ASSERT_EQ(v2,Vector3f(0,0,0));
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
	Bound3<Float> b3 = Union(b1, b2);
	ASSERT_EQ(b3[0], Point3<Float>(1, 1, 1));
	ASSERT_EQ(b3[1], Point3<Float>(3, 3, 3));
	Bound3<Float> b4 = Intersect(b1, b2);
	ASSERT_EQ(b4[0], Point3<Float>(2, 2, 2));
	ASSERT_EQ(b4[1], Point3<Float>(2, 2, 2));

	ASSERT_TRUE(Inside(Point3<Float>(2, 2, 2), b4));
	ASSERT_TRUE(Overlap(b1, b2));
	ASSERT_TRUE(Overlap(b1, b4));

	b4 = Expand(b4, 1.0f);
	ASSERT_EQ(b4[0], Point3<Float>(1, 1, 1));
	ASSERT_EQ(b4[1], Point3<Float>(3, 3, 3));
	ASSERT_EQ(b4.MaximumExtent(), 2);

}

TEST(Ray,all) {
	Ray r(Point3f(0, 0, 0), Vector3f(1, 0, 0));
	Point3f p = r(0.5f);
}

TEST(RayDifferential,all) {
	RayDifferential r(Point3f(0, 0, 0), Vector3f(1, 0, 0));
	r.hasDifferential = true;
	r.ox = Point3f(0, 0, 0);
	r.dx = Vector3f(0, 1, 0);
	r.ScaleRayDifferential(0.5f);
	ASSERT_EQ(r.dx, Vector3f(0.5, 0.5, 0));

}
#include "transform.h"
TEST(Matrix4x4,all) {
	Matrix4x4 mat;
	Matrix4x4 mat2;
	Matrix4x4 mat3 = mat * mat2;
	ASSERT_EQ(mat, mat3);
	mat = Transpose(mat);
	ASSERT_EQ(mat, mat3);
	mat = Inverse(mat);
	ASSERT_EQ(mat, mat3);
	Float aa[4][4] = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1,
			1.2, 1.3, 1.4, 1.5, 1.6 };
	Matrix4x4 mataa(aa);
	ASSERT_EQ(mataa.m[0][1], 0.2f);
	mataa = Transpose(mataa);
	ASSERT_EQ(mataa.m[0][1], 0.5f);

}

TEST(Vector3,CoordinateSystem) {
	Vector3f V(1.0f, 0.0f, 0.0f);
	Vector3f VT;
	Vector3f VB;
	CoordinateSystem(V, &VT, &VB);
	ASSERT_EQ(VT, Vector3f(0, 0, 1));
	ASSERT_EQ(VB, Vector3f(0, -1, 0));
}

TEST(Vector3,SphericalDirection) {
	Vector3f v = SphericalDirection(0.0f, 1.0f, 0.0f);
	ASSERT_EQ(v, Vector3f(0, 0, 1));
	Float theta = SphericalTheta(v);
	Float phi = SphericalTheta(v);
	ASSERT_EQ(theta, 0);
	ASSERT_EQ(phi, 0);
}

TEST(TRANSFORM,all) {
	Transform trans;
	//std::cout<<trans<<std::endl;
	Point3f point(1, 1, 1);
	Vector3f cerr(1.78814e-07, 1.78814e-07, 1.78814e-07);
	Vector3f err;
	Point3f ret = trans(point);
	ASSERT_EQ(point, ret);
	ret = trans(point, &err);
	ASSERT_EQ(point, ret);
	ret = trans(point, cerr, &err);
	ASSERT_EQ(point, ret);
	//ASSERT_EQ(err,Vector3f(1.78814e-07,1.78814e-07,1.78814e-07));

	Vector3f v1(1, 1, 1);
	Vector3f v2 = trans(v1);
	ASSERT_EQ(v2, v1);
	v2 = trans(v1, &err);
	ASSERT_EQ(v2, v1);
	v2 = trans(v1, cerr, &err);
	ASSERT_EQ(v2, v1);
	//ASSERT_EQ(err,Vector3f(1.78814e-07,1.78814e-07,1.78814e-07));

	Ray ray(Point3f(1, 1, 1), Vector3f(1, 0, 0));
	Vector3f oerr, derr;
	Vector3f coerr(1, 1, 1), cderr(1, 1, 1);
	Ray r = trans(ray, &oerr, &derr);
	//ASSERT_EQ(oerr,Vector3f(0,0,0));
	r = trans(ray, coerr, cderr, &oerr, &derr);
	//ASSERT_EQ(oerr,Vector3f(0,0,0));

	Bound3f b(Point3f(-1, -1, -1), Point3f(1, 1, 1));
	Bound3f b2 = trans(b);
	ASSERT_EQ(b.minPoint, b2.minPoint);

	ASSERT_FALSE(trans.SwapsHandedness());

	Transform transform = Translate(Vector3f(1, 1, 1));
	ret = transform(point);
	ASSERT_EQ(Point3f(2, 2, 2), ret);
	transform = RotateX(90);
	ret = transform(point);
	//ASSERT_EQ(Point3f(1,-1,1),ret);

	Point3f pp(0, 0, 0);
	Transform move_x_one = Translate(Vector3f(1, 0, 0));
	Transform move_x_two = move_x_one * move_x_one;
	pp = move_x_two(pp);
	ASSERT_EQ(pp, Point3f(2, 0, 0));
}

#include "shapes/sphere.h"
TEST(Sphere,All) {
	Transform move_z_one = Translate(Vector3f(0, 0, 2));
	Transform move_z_none = Inverse(move_z_one);
	Sphere sphere(&move_z_one, &move_z_none, false, 1, -1, 1, 360);
	ASSERT_EQ(sphere.Area(), 4 * Pi);
	Ray ray(Point3f(0, 0, 0), Vector3f(0, 0, 1));
	ASSERT_TRUE(sphere.IntersectP(ray));
	Ray ray2(Point3f(0, 0, 0), Vector3f(0, 0, -1));
	ASSERT_TRUE(!sphere.IntersectP(ray2));
	Float tHit;
	SurfaceInteraction s;
	sphere.Intersect(ray, &tHit, &s);
	//ASSERT_EQ(s.shading.dpdu,Vector3f(0,0,0));
}

TEST(Primitive,all) {

}

#include "spectrum.h"
TEST(CoefficientSpectrum,all) {
	SampledSpectrum::Init();
	CoefficientSpectrum<100> c;

	Float lambda[] = { 500, 400, 600 };
	Float vals[] = { 5, 4, 6 };
	ASSERT_FALSE(SpectrumSamplesSorted(lambda, vals, 3));
	SortSpectrumSamples(lambda, vals, 3);
	ASSERT_TRUE(SpectrumSamplesSorted(lambda, vals, 3));
	SampledSpectrum s = SampledSpectrum::FromSampled(lambda, vals, 3);
	ASSERT_NE(s[0], 4);
	Float xyz[3];
	s.ToXYZ(xyz);
	//ASSERT_EQ(xyz[2],4);

	SampledSpectrum s1 = SampledSpectrum::FromSampled(lambda, vals, 3);
	Float rgb[3];
	s1.ToRGB(rgb);
	RGBSpectrum s2 = RGBSpectrum::FromSampled(lambda, vals, 3);
	Float rgb2[3];
	s2.ToRGB(rgb2);
	//ASSERT_EQ(rgb[1],rgb2[1]);

}
#include "parallel.h"
TEST(AtomicFloat,all) {
	AtomicFloat a(0);
	a.Add(1.0f);
	ASSERT_EQ(a, 1.0f);
}

TEST(Bound2,all) {
	Bound2<int> b(Point2f(0, 0), Point2f(2, 2));
	ASSERT_EQ(b.Area(), 4);
}

#include "reflection.h"
TEST(reflection,all){
	Vector3f wo(1,0,0);
	Normal3f n(0,0,1);
	Vector3f wi=Reflect(wo,n);
	ASSERT_EQ(wi,Vector3f(-1,0,0));
	wo=Vector3f(0.5f,0.0f,0.5f);
	Vector3f wt;
	ASSERT_TRUE(Refract(wo,n,1.0f,&wt));
	ASSERT_EQ(wt,Vector3f(-0.5,0,-0.5));
}
#include "film.h"
#include "filters/box.h"
TEST(Film,all) {
	Film film(Point2i(32, 32), Bound2f(Point2f(0.1, 0.1), Point2f(0.9, 0.9)),
			std::unique_ptr<Filter>(new BoxFilter(Vector2f(1, 1))), "test");
	//film.WriteImage();
	ASSERT_EQ(film.croppedPixelBound, Bound2i(Point2i(4, 4), Point2i(28, 28)));
	ASSERT_EQ(film.GetSampleBounds(), Bound2i(Point2i(3, 3), Point2i(29, 29)));

}

#include "camera.h"
#include <cameras/pinhole.h>
TEST(Camera,all) {
	PinholeCamera camera();
}

#include "statsTest.h"
#include "stats.h"
TEST(Stats,all) {
	StatsTest st1;
	StatsTest st2;
	StatsTest st3;
	StatsTest st4;
	ReportThreadStats();
	LogStats();
}

#include "rng.h"
TEST(RNG,all) {
	RNG rng;
	float a = rng.UniformFloat();
	//ASSERT_EQ(a,rng.UniformFloat());
}

#include "samplers/random.h"
TEST(RandomSampler,all) {
	RandomSampler sampler(4);
	Float f = sampler.Get1DSample();
}
#include "scene.h"
#include "integrator.h"
#include "lights/point.h"
#include "integrators/depth.h"
#include "integrators/normal.h"
#include "accelerators/iteration.h"
//TEST(TestSceneOne,all) {
//	Transform move_z_one = Translate(Vector3f(0, 0, 3));
//	Transform move_z_none = Inverse(move_z_one);
//	std::shared_ptr<Shape> sphere(new Sphere(&move_z_one, &move_z_none, false, 2, -2, 2, 360));
//	std::shared_ptr<Primitive> primitive(new GeomPrimitive(sphere));
//	Transform lightTrans=Translate(Vector3f(3,0,0));
//	std::shared_ptr<PointLight> pl(new PointLight(lightTrans,Spectrum(10)));
//	auto primitives=std::vector<std::shared_ptr<Primitive>>();
//	primitives.push_back(primitive);
//	auto iteration=std::make_shared<Iteration>(primitives);
//	std::vector<std::shared_ptr<Light>> lights;
//	lights.push_back(pl);
//	Scene scene(iteration,lights);
//	std::shared_ptr<RandomSampler> sampler(new RandomSampler(4));
//	Transform trans = Translate(Vector3f(0, 0, 0));
//	std::shared_ptr<const Camera> camera(new PinholeCamera(trans, 0, 1,
//			new Film(Point2i(600, 600), Bound2f(Point2f(0, 0), Point2f(1, 1)),
//					std::unique_ptr<Filter>(new BoxFilter(Vector2f(1, 1))),
//					"result/TestSceneOne_randomSampler.png"),50));
//	NormalIntegrator integrator(camera,sampler,Bound2i(Point2i(100,100),Point2i(500,500)));
//
//
//	integrator.RenderScene(scene);
////	for (int j = 0; j < 256; ++j)
////		for (int i = 0; i < 256; ++i) {
////			sampler.StartPixel(Point2i(i, j));
////			do {
////				CameraSample cs = sampler.GetCameraSample(Point2i(i, j));
////				Ray r;
////				camera.GenerateRay(cs, &r);
////				if (scene.IntersectP(r)) {
////					Float rgb[3]={1,1,1};
////					Float xyz[3];
////					RGBToXYZ(rgb,xyz);
////					camera.film->GetPixel(Point2i(i, j)).xyz[0]+=xyz[0];
////					camera.film->GetPixel(Point2i(i, j)).xyz[1]+=xyz[1];
////					camera.film->GetPixel(Point2i(i, j)).xyz[2]+=xyz[2];
////					camera.film->GetPixel(Point2i(i, j)).filterWeightSum+=1;
////				}
////			}while (sampler.StartNextSample()) ;
////		}
////camera.film->WriteImage();
//}
#include "api.h"
#include "paramset.h"
TEST(TestSceneOne,use_api){
	Options option;
	option.imageFile="result/TestSceneOne.png";
	option.numThread=4;
	raidenInit(option);
	{
		ParamSet filterParam;
		filterParam.AddFloat("xwidth",std::unique_ptr<Float[]>(new Float[1]{1.0f}),1);
		filterParam.AddFloat("ywidth",std::unique_ptr<Float[]>(new Float[1]{1.0f}),1);
		raidenPixelFilter("box",filterParam);
		ParamSet filmParam;
		filmParam.AddInt("xresolution",std::unique_ptr<int[]>(new int[1]{600}),1);
		filmParam.AddInt("yresolution",std::unique_ptr<int[]>(new int[1]{600}),1);
		filmParam.AddFloat("cropwindow",std::unique_ptr<Float[]>(new Float[4]{0.0f,1.0f,0.0f,1.0f}),4);
		raidenFilm("image",filmParam);
		ParamSet cameraParam;
		cameraParam.AddFloat("distance",std::unique_ptr<Float[]>(new Float[1]{50.0f}),1);
		raidenCamera("pinhole",cameraParam);
		raidenWorldBegin();
		{
			raidenTranslate(0,0,3);
			ParamSet shapdeParam;
			shapdeParam.AddFloat("radius",std::unique_ptr<Float[]>(new Float[1]{2.0f}),1);
			raidenShape("sphere",shapdeParam);
		}
		raidenWorldEnd();
	}
	raidenCleanup();
}
#include "light.h"
TEST(Light,all){
	LightFlags f1=LightFlags::DeltaDirection;
	ASSERT_TRUE(IsDeltaLight((int)f1));
	LightFlags f2=LightFlags::Area;
	ASSERT_FALSE(IsDeltaLight((int)f2));

}


TEST(pointlight,all){
	Transform ts;
	PointLight pl(ts,Spectrum(1));
	Interaction ref;
	ref.p=Point3f(0,0,3);
	Vector3f wi;
	Float pdf;
	Spectrum le=pl.Sample_Li(ref,&wi,&pdf);
	ASSERT_EQ(wi,Vector3f(0,0,-1));
	//ASSERT_EQ(le,Spectrum(1));
}
#include "parallel.h"
TEST(parallel,all){
	ParallelInit();
	auto func1 = [](Point2i i) { Error(i);};
	Point2i count(100,100);
	//ParallelFor2D(func1,count);
	ParallelCleanup();
}
#include "memory.h"
TEST(memoryManage,all){
	void* ptr=AllocAligned(100);
	long long address=(long long)ptr;//;
	ASSERT_EQ(address%L1_CACHE_LINE_SIZE,0);
	FreeAligned(ptr);
	MemoryArena arena;
	Point2i* ptr2=arena.Alloc<Point2i>(1024);
	ASSERT_EQ(ptr2[0],Point2i(0,0));
	ASSERT_EQ(arena.TotalAllocated(),262144);
}
#include "paramset.h"
TEST(paramset,all){
	ParamSet set;
	bool* v=new bool[2];
	v[0]=true;
	set.AddBool("test",std::unique_ptr<bool[]>(v),1);
	int count;
	set.FindBool("test",&count);
	ASSERT_EQ(count,1);
	ASSERT_EQ(set.FindOneBool("test",false),true);

	int * ints=new int[3]{1,2,3};
	set.AddInt("ints",std::unique_ptr<int[]>(ints),3);
	const int * ptr=set.FindInt("ints",&count);
	ASSERT_EQ(count,3);
	ASSERT_EQ(ptr[2],3);
}
#include "sampling.h"
TEST(sampling,all){
	RNG rng;
	Float samples[4];
	StratifiedSample1D(samples,4,rng,false);
	ASSERT_EQ(samples[0],0.125f);

	Point2f ss[4];
	StratifiedSample2D(ss,2,2,rng,false);
	ASSERT_EQ(ss[0],Point2f(0.25f,0.25f));

	Point2f ss2=RejectionSampleDisk(rng);
	ASSERT_LE(ss2.x*ss2.x+ss2.y*ss2.y,1);

	Vector3f dir=UniformSampleHemisphere(Point2f(1,0));
	ASSERT_EQ(dir,Vector3f(0,0,1));

	dir=UniformSampleSphere(Point2f(0,0));
	ASSERT_EQ(dir,Vector3f(0,0,1));

	Point2f disk=UniformSampleDisk(Point2f(1,1));
	//ASSERT_EQ(disk,Point2f(1,0));
	Float array[4];
	LatinHypercube(array,2,2,rng);
	//ASSERT_EQ(array[0],1);
	Error("LatinHypercube 2sample 2dim:")
	for(int i=0;i<4;++i){
		Error(array[i]);
	}
}

void UnitTest(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
}

#endif
#endif /* SRC_TEST_TEST_CPP_ */

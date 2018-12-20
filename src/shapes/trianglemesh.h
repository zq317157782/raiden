/*
 * trianglemesh.h
 *
 *  Created on: 2017年1月6日
 *      Author: zhuqian
 */

#ifndef SRC_SHAPES_TRIANGLEMESH_H_
#define SRC_SHAPES_TRIANGLEMESH_H_
#include "raiden.h"
#include "shape.h"
#include <map>

//三角Mesh
struct TriangleMesh {
	const int numTriangles; //三角面片的个数
	const int numVertices; //顶点个数
	std::vector<int> vertexIndices; //索引
	std::unique_ptr<Point3f[]> vertices; //顶点数据
	std::unique_ptr<Normal3f[]> normals; //逐顶点法线
	std::unique_ptr<Vector3f[]> tangents; //切线
	std::unique_ptr<Point2f[]> uv; //纹理坐标

	TriangleMesh(const Transform& ObjectToWorld, int nTriangles/*三角面片个数*/,
			const int* vertexIndices/*顶点索引*/, int nVertices/*顶点个数*/,
			const Point3f* P/*顶点数组*/, const Vector3f *S/*切线数组*/,
			const Normal3f *N/*法线数组*/, const Point2f *uv/*纹理坐标数组*/);
};

//代表一个三角形
class Triangle: public Shape {
private:
	std::shared_ptr<TriangleMesh> _mesh;
	const int * _vertexIndices;

#ifdef TRIANGLE_MESH_PRECOMPUTE_DATA_IF_CAN
	Vector3f _dpdu, _dpdv;
	Normal3f _normal;
#endif//TRIANGLE_MESH_PRECOMPUTE_DATA_IF_CAN

public:
	Triangle(const Transform* ObjectToWorld, const Transform* WorldToObject,
			bool reverseOrientation, std::shared_ptr<TriangleMesh>& mesh,
			int triangleNumber) :
			Shape(ObjectToWorld, WorldToObject, reverseOrientation), _mesh(mesh) {
		_vertexIndices = &(_mesh->vertexIndices[triangleNumber * 3]);

#ifdef TRIANGLE_MESH_PRECOMPUTE_DATA_IF_CAN
		const Point3f& v1 = _mesh->vertices[_vertexIndices[0]];
		const Point3f& v2 = _mesh->vertices[_vertexIndices[1]];
		const Point3f& v3 = _mesh->vertices[_vertexIndices[2]];

		Point2f uv[3];
		GetUVs(uv);
		Vector3f dp02 = v1 - v3, dp12 = v2 - v3;

		//计算dpdu和dpdv
		Vector2f duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];

		//行列式
		Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
		//是否是退化的UV坐标
		//这里不判断是否为0是为了避免浮点数产生的误差 (determinant==0)
		bool degenerateUV = std::abs(determinant) < 1e-8;
		if (!degenerateUV) {
			Float invdet = 1 / determinant;
			_dpdu = (duv12[1] * dp02 - duv02[1] * dp12) * invdet;
			_dpdv = (-duv12[0] * dp02 + duv02[0] * dp12) * invdet;
		}
		if (degenerateUV || Cross(_dpdu, _dpdv).LengthSquared() == 0) {
			CoordinateSystem(Normalize(Cross(v3 - v1, v2 - v1)), &_dpdu, &_dpdv);
		}
		//预计算法线
		_normal=Normal3f(Normalize(Cross(dp02, dp12)));

#endif//TRIANGLE_MESH_PRECOMPUTE_DATA_IF_CAN
	}
	Bound3f ObjectBound() const override {
		const Point3f& p0 = (*worldToObject)(
				_mesh->vertices[_vertexIndices[0]]);
		const Point3f& p1 = (*worldToObject)(
				_mesh->vertices[_vertexIndices[1]]);
		const Point3f& p2 = (*worldToObject)(
				_mesh->vertices[_vertexIndices[2]]);
		return Union(Bound3f(p0, p1), p2);
	}
	Bound3f WorldBound() const override {
		const Point3f& p0 = _mesh->vertices[_vertexIndices[0]];
		const Point3f& p1 = _mesh->vertices[_vertexIndices[1]];
		const Point3f& p2 = _mesh->vertices[_vertexIndices[2]];
		return Union(Bound3f(p0, p1), p2);
	}

	//三角形的面积
	Float Area() const override {
		Vector3f l1 = _mesh->vertices[_vertexIndices[1]]
				- _mesh->vertices[_vertexIndices[0]];
		Vector3f l2 = _mesh->vertices[_vertexIndices[2]]
				- _mesh->vertices[_vertexIndices[0]];
		return Cross(l1, l2).Length() * 0.5f;
	}

	bool Intersect(const Ray& ray, Float* tHit,
			SurfaceInteraction* surfaceIsect, bool testAlpha = true) const
					override;
	bool IntersectP(const Ray& ray, bool testAlpha = true) const override;
	void GetUVs(Point2f uv[3]) const {
		if (_mesh->uv) {
			uv[0] = _mesh->uv[_vertexIndices[0]];
			uv[1] = _mesh->uv[_vertexIndices[1]];
			uv[2] = _mesh->uv[_vertexIndices[2]];
		} else {
			uv[0] = Point2f(0, 0);
			uv[1] = Point2f(1, 0);
			uv[2] = Point2f(1, 1);
		}
	}


	Interaction Sample(const Point2f& uv, Float *pdf) const override {
		LWarning<<"Triangle::Sample is unimplemented!";
		Assert(false);
		return Interaction();
	}
	Interaction Sample(const Interaction& ref, const Point2f& u, Float *pdf) const override {
		LWarning<<"Triangle::Sample is unimplemented!";
		Assert(false);
		return Interaction();
	}

	//从uv坐标映射到三角面世界坐标系的函数,可能不是一一对应
	bool UVToWorld(const Point2f& uv,UVInteraction* insect)const {
		//获取当前三角面的3个uv坐标
		Point2f uvs[3];
		GetUVs(uvs);
		//使用和判断射线和三角面相交一样的思路来判断，提供的uv是否在三角面内
		//计算edge函数
		Float e2=(uvs[1].x-uvs[0].x)*(uv.y-uvs[0].y)-(uv.x-uvs[0].x)*(uvs[1].y-uvs[0].y);
		Float e0=(uvs[2].x-uvs[1].x)*(uv.y-uvs[1].y)-(uv.x-uvs[1].x)*(uvs[2].y-uvs[1].y);
		Float e1=(uvs[0].x-uvs[2].x)*(uv.y-uvs[2].y)-(uv.x-uvs[2].x)*(uvs[0].y-uvs[2].y);
#ifdef FLOAT_IS_DOUBLE
#else
	if (e0 == 0 || e1 == 0 || e2 == 0) {
		e2=((double)uvs[1].x-(double)uvs[0].x)*((double)uv.y-(double)uvs[0].y)-((double)uv.x-(double)uvs[0].x)*((double)uvs[1].y-(double)uvs[0].y);
		e0=((double)uvs[2].x-(double)uvs[1].x)*(double)(uv.y-(double)uvs[1].y)-((double)uv.x-(double)uvs[1].x)*((double)uvs[2].y-(double)uvs[1].y);
		e1=((double)uvs[0].x-(double)uvs[2].x)*((double)uv.y-(double)uvs[2].y)-((double)uv.x-(double)uvs[2].x)*((double)uvs[0].y-(double)uvs[2].y);
	}
#endif
	//判断是否相交
	if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0)) {
		return false;
	}

	Float det = e0 + e1 + e2;
	if (det == 0) {
		return false;
	}

	//走到这就说明相交了
	//求质心坐标以及参数t
	Float invDet = 1.0 / det;
	Float b0 = e0 * invDet;
	Float b1 = e1 * invDet;
	Float b2 = e2 * invDet;
	//获得三个世界坐标系的顶点
	const Point3f& v1 = _mesh->vertices[_vertexIndices[0]];
	const Point3f& v2 = _mesh->vertices[_vertexIndices[1]];
	const Point3f& v3 = _mesh->vertices[_vertexIndices[2]];
	
	Point3f p = v1 * b0 + v2 * b1 + v3 * b2;
	Vector3f dp02 = v1 - v3, dp12 = v2 - v3;

	std::vector<Point3f> set;
	set.push_back(p);
	if(insect){
		Normal3f n=Normal3f(Normalize(Cross(dp02, dp12)));
		(*insect) = UVInteraction(p, n);
		Normal3f ns;
		if (_mesh->normals) {
			ns = (b0 * _mesh->normals[_vertexIndices[0]]
					+ b1 * _mesh->normals[_vertexIndices[1]]
					+ b2 * _mesh->normals[_vertexIndices[2]]);
			//判断是否为合法网格
			if (ns.LengthSquared() > 0.0f) {
				ns = Normalize(ns);
				insect->n=ns;
			} 
		}
	}
	return  true;
	}
};


std::vector<std::shared_ptr<Shape>> CreateTriangleMesh(
	const Transform *o2w, const Transform *w2o, bool reverseOrientation,
	int nTriangles, const int *vertexIndices, int nVertices, const Point3f *p,
	const Vector3f *s, const Normal3f *n, const Point2f *uv);
std::vector<std::shared_ptr<Triangle>> CreateTriangleMesh2(
	const Transform *o2w, const Transform *w2o, bool reverseOrientation,
	int nTriangles, const int *vertexIndices, int nVertices, const Point3f *p,
	const Vector3f *s, const Normal3f *n, const Point2f *uv);
std::vector<std::shared_ptr<Shape>> CreateTriangleMeshShape(
	const Transform *o2w, const Transform *w2o, bool reverseOrientation,
	const ParamSet &params);

#endif /* SRC_SHAPES_TRIANGLEMESH_H_ */

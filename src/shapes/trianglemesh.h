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
public:
	Triangle(const Transform* ObjectToWorld, const Transform* WorldToObject,
			bool reverseOrientation, std::shared_ptr<TriangleMesh>& mesh,
			int triangleNumber) :
			Shape(ObjectToWorld, WorldToObject, reverseOrientation), _mesh(mesh) {
		_vertexIndices = &(_mesh->vertexIndices[triangleNumber * 3]);
	}
	Bound3f ObjectBound() const override {
		const Point3f& p0 = (*worldToObject)(_mesh->vertices[_vertexIndices[0]]);
		const Point3f& p1 = (*worldToObject)(_mesh->vertices[_vertexIndices[1]]);
		const Point3f& p2 = (*worldToObject)(_mesh->vertices[_vertexIndices[2]]);
		return Union(Bound3f(p0, p1), p2);
	}
	Bound3f WorldBound() const override {
		const Point3f& p0 = _mesh->vertices[_vertexIndices[0]];
		const Point3f& p1 = _mesh->vertices[_vertexIndices[1]];
		const Point3f& p2 = _mesh->vertices[_vertexIndices[2]];
		return Union(Bound3f(p0, p1), p2);
	}
	//三角形的面积
	Float Area() const override{
		Vector3f l1=_mesh->vertices[_vertexIndices[1]]-_mesh->vertices[_vertexIndices[0]];
		Vector3f l2=_mesh->vertices[_vertexIndices[2]]-_mesh->vertices[_vertexIndices[0]];
		return Cross(l1,l2).Length()*0.5f;
	}

	bool Intersect(const Ray& ray,Float* tHit,SurfaceInteraction* surfaceIsect,bool testAlpha=true) const override;
	bool IntersectP(const Ray& ray,bool testAlpha=true) const override;
};

#endif /* SRC_SHAPES_TRIANGLEMESH_H_ */

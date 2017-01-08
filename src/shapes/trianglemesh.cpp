/*
 * trianglemesh.cpp
 *
 *  Created on: 2017年1月6日
 *      Author: zhuqian
 */
#include "trianglemesh.h"

TriangleMesh::TriangleMesh(const Transform& ObjectToWorld, int nTriangles,
		const int* vertexIndices, int nVertices, const Point3f* P,
		const Vector3f *S, const Normal3f *N, const Point2f *UV) :
		numTriangles(nTriangles), numVertices(nVertices), vertexIndices(
				vertexIndices, vertexIndices + 3 * numTriangles) {

	vertices.reset(new Point3f[numVertices]);
	//转换到世界坐标系下
	for (int i = 0; i < numVertices; ++i) {
		vertices[i] = ObjectToWorld(P[i]);
	}

	//法线
	if (N) {
		normals.reset(new Normal3f[numVertices]);
		for (int i = 0; i < numVertices; ++i) {
			normals[i] = ObjectToWorld(N[i]);
		}
	}

	//切线
	if (S) {
		tangents.reset(new Vector3f[numVertices]);
		for (int i = 0; i < numVertices; i++) {
			tangents[i] = ObjectToWorld(S[i]);
		}
	}

	if (UV) {
		uv.reset(new Point2f[numVertices]);
		memcpy(uv.get(), UV, numVertices * sizeof(Point2f));
	}
}

bool Triangle::Intersect(const Ray& ray, Float* tHit,
		SurfaceInteraction* surfaceIsect, bool testAlpha) const {

	const Point3f& v1 = _mesh->vertices[_vertexIndices[0]];
	const Point3f& v2 = _mesh->vertices[_vertexIndices[1]];
	const Point3f& v3 = _mesh->vertices[_vertexIndices[2]];
	//变换Ray的起始点到(0,0,0)
	Point3f v1t = v1 - Vector3f(ray.o);
	Point3f v2t = v2 - Vector3f(ray.o);
	Point3f v3t = v3 - Vector3f(ray.o);

	//重新排列ray的方向的x,y,z分量，保证z分量的绝对值最大(不为0)
	//这里没用取余是为了避免浮点数的除法运算
	int zk = MaxDimension(Abs(ray.d));
	int xk = zk + 1;
	if (xk == 3) {
		xk = 0;
	}
	int yk = xk + 1;
	if (yk == 3) {
		yk = 0;
	}
	v1t = Permute(v1t, xk, yk, zk);
	v2t = Permute(v2t, xk, yk, zk);
	v3t = Permute(v3t, xk, yk, zk);
	//使ray的方向指向局部坐标的(0,0,1)
	Float sx = -ray.d[xk] / ray.d[zk];
	Float sy = -ray.d[yk] / ray.d[zk];
	Float sz = 1 / ray.d[zk];
	//只计算x,y的切变换,z的变换在确定和三角相交的时候再执行
	//验证一句话，好程序要抠门>_<
	v1t.x += v1t.z * sx;
	v1t.y += v1t.z * sy;
	v2t.x += v2t.z * sx;
	v2t.y += v2t.z * sy;
	v3t.x += v3t.z * sx;
	v3t.y += v3t.z * sy;

//下面要开始计算edge function了
}

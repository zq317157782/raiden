/*
 * trianglemesh.cpp
 *
 *  Created on: 2017年1月6日
 *      Author: zhuqian
 */
#include "trianglemesh.h"

TriangleMesh::TriangleMesh(const Transform& ObjectToWorld, int nTriangles, const int* vertexIndices,
	int nVertices, const Point3f* P, const Vector3f *S, const Normal3f *N, const Point2f *UV):numTriangles(nTriangles), numVertices(nVertices),vertexIndices(vertexIndices, vertexIndices+3*numTriangles){
	
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
		for (int i = 0; i < numVertices; i++)
		{
			tangents[i] = ObjectToWorld(S[i]);
		}
	}

	if (UV) {
		uv.reset(new Point2f[numVertices]);
		memcpy(uv.get(), UV, numVertices * sizeof(Point2f));
	}
}


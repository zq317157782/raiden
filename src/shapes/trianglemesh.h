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
	const int numTriangles;//三角面片的个数
	const int numVertices; //顶点个数
	std::vector<int> vertexIndices;//索引
	std::unique_ptr<Point3f[]> vertices;//顶点数据
	std::unique_ptr<Normal3f[]> normals; //逐顶点法线
	std::unique_ptr<Vector3f[]> tangents; //切线
	std::unique_ptr<Point2f[]> uv; //纹理坐标

	TriangleMesh(const Transform& ObjectToWorld, int nTriangles/*三角面片个数*/,const int* vertexIndices/*顶点索引*/, 
		int nVertices/*顶点个数*/, const Point3f* P/*顶点数组*/, const Vector3f *S/*切线数组*/, const Normal3f *N/*法线数组*/, const Point2f *uv/*纹理坐标数组*/);
};



#endif /* SRC_SHAPES_TRIANGLEMESH_H_ */

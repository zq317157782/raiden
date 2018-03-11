/*
 * trianglemesh.cpp
 *
 *  Created on: 2017年1月6日
 *      Author: zhuqian
 */
#include "trianglemesh.h"
#include "interaction.h"
#include "paramset.h"
#include <map>
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
	Vector3f origin = Vector3f(ray.o);
	Point3f v1t = v1 - origin;
	Point3f v2t = v2 - origin;
	Point3f v3t = v3 - origin;

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
	Vector3f d = Permute(ray.d, xk, yk, zk);
	//使ray的方向指向局部坐标的(0,0,1)
	Float sx = -d.x / d.z;
	Float sy = -d.y / d.z;
	Float sz = 1 /d.z;
	//只计算x,y的切变换,z的变换在确定和三角相交的时候再执行
	//验证一句话，好程序要抠门>_<
	v1t.x += v1t.z * sx;
	v1t.y += v1t.z * sy;
	v2t.x += v2t.z * sx;
	v2t.y += v2t.z * sy;
	v3t.x += v3t.z * sx;
	v3t.y += v3t.z * sy;

	//下面要开始计算edge function了
	Float e0 = v2t.x * v3t.y - v2t.y * v3t.x;
	Float e1 = v3t.x * v1t.y - v3t.y * v1t.x;
	Float e2 = v1t.x * v2t.y - v1t.y * v2t.x;
#ifdef FLOAT_IS_DOUBLE
#else
	if (e0 == 0 || e1 == 0 || e2 == 0) {
		e0 = (double) v2t.x * (double) v3t.y - (double) v2t.y * (double) v3t.x;
		e1 = (double) v3t.x * (double) v1t.y - (double) v3t.y * (double) v1t.x;
		e2 = (double) v1t.x * (double) v2t.y - (double) v1t.y * (double) v2t.x;
	}
#endif
	//判断是否相交
	if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0)) {
		return false;
	}
	Float det = e0 + e1 + e2;
	//两条向量共边了,那就不是三角形了
	if (det == 0) {
		return false;
	}
	//补上z的缩放
	v1t.z *= sz;
	v2t.z *= sz;
	v3t.z *= sz;

	//判断是否是有效的t值
	Float s = e0 * v1t.z + e1 * v2t.z + e2 * v3t.z;
	if (det < 0 && (s >= 0 || s < det * ray.tMax)) {
		return false;
	} else if (det > 0 && (s <= 0 || s > det * ray.tMax)) {
		return false;
	}
	//走到这就说明相交了
	//求质心坐标以及参数t
	Float invDet = 1.0 / det;
	Float b0 = e0 * invDet;
	Float b1 = e1 * invDet;
	Float b2 = e2 * invDet;
	Float t = s * invDet;

	//计算质心坐标的时候产生的误差
	//todo 这里还没有看PBRT的书的解释
	Float maxZt = MaxComponent(Abs(Vector3f(v1t.z, v2t.z, v3t.z)));
	Float deltaZ = gamma(3) * maxZt;
	Float maxXt = MaxComponent(Abs(Vector3f(v1t.x, v2t.x, v3t.x)));
	Float maxYt = MaxComponent(Abs(Vector3f(v1t.y, v2t.y, v3t.y)));
	Float deltaX = gamma(5) * (maxXt + maxZt);
	Float deltaY = gamma(5) * (maxYt + maxZt);
	Float deltaE = 2
			* (gamma(2) * maxXt * maxYt + deltaY * maxXt + deltaX * maxYt);
	Float maxE = MaxComponent(Abs(Vector3f(e0, e1, e2)));
	Float deltaT = 3
			* (gamma(3) * maxE * maxZt + deltaE * maxZt + deltaZ * maxE)
			* std::abs(invDet);
	if (t <= deltaT) {
		return false;
	}

	Point2f uv[3];
	GetUVs(uv);
	Vector3f dp02 = v1 - v3, dp12 = v2 - v3;
#ifdef TRIANGLE_MESH_PRECOMPUTE_DATA_IF_CAN
	
#else
	Vector3f dpdu, dpdv;
	//计算dpdu和dpdv
	Vector2f duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];
	
	//行列式
	Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
	//是否是退化的UV坐标
	//这里不判断是否为0是为了避免浮点数产生的误差 (determinant==0)
	bool degenerateUV = std::abs(determinant) < 1e-8;
	if (!degenerateUV) {
		Float invdet = 1 / determinant;
		dpdu = (duv12[1] * dp02 - duv02[1] * dp12) * invdet;
		dpdv = (-duv12[0] * dp02 + duv02[0] * dp12) * invdet;
	}
	if (degenerateUV || Cross(dpdu, dpdv).LengthSquared() == 0) {
		CoordinateSystem(Normalize(Cross(v3 - v1, v2 - v1)), &dpdu, &dpdv);
	}
#endif //TRIANGLE_MESH_PRECOMPUTE_DATA_IF_CAN

	//计算误差
	Float xAbsSum = (std::abs(b0 * v1.x) + std::abs(b1 * v2.x)
			+ std::abs(b2 * v3.x));
	Float yAbsSum = (std::abs(b0 * v1.y) + std::abs(b1 * v2.y)
			+ std::abs(b2 * v3.y));
	Float zAbsSum = (std::abs(b0 * v1.z) + std::abs(b1 * v2.z)
			+ std::abs(b2 * v3.z));
	Vector3f pErr = gamma(7) * Vector3f(xAbsSum, yAbsSum, zAbsSum);

	//使用质心坐标计算相交点以及相交点UV坐标
	Point3f pHit = v1 * b0 + v2 * b1 + v3 * b2;
	Point2f uvHit = uv[0] * b0 + uv[1] * b1 + uv[2] * b2;


	//填充SurfaceInteraction
#ifdef TRIANGLE_MESH_PRECOMPUTE_DATA_IF_CAN
	*surfaceIsect = SurfaceInteraction(pHit, pErr, uvHit, -ray.d, _dpdu, _dpdv,
		Normal3f(0, 0, 0), Normal3f(0, 0, 0), ray.time, this);

	surfaceIsect->n = surfaceIsect->shading.n = _normal;
#else
	*surfaceIsect = SurfaceInteraction(pHit, pErr, uvHit, -ray.d, dpdu, dpdv,
			Normal3f(0, 0, 0), Normal3f(0, 0, 0), ray.time, this);

	//使用两条边来计算法线
	surfaceIsect->n = surfaceIsect->shading.n = Normal3f(
		Normalize(Cross(dp02, dp12)));
#endif //TRIANGLE_MESH_PRECOMPUTE_DATA_IF_CAN

	

	//判断网格是否包含逐顶点法线和切线
	//这里是根据mesh提供的数据，进行相应的法线，切线，次切线计算，并且在数据不合法的时候，进行一定的处理
	if (_mesh->normals || _mesh->tangents) {
		Normal3f ns;
		//网格包含自定义法线
		if (_mesh->normals) {
			ns = (b0 * _mesh->normals[_vertexIndices[0]]
					+ b1 * _mesh->normals[_vertexIndices[1]]
					+ b2 * _mesh->normals[_vertexIndices[2]]);
			//判断是否为合法网格
			if (ns.LengthSquared() > 0.0f) {
				ns = Normalize(ns);
			} else {
				ns = surfaceIsect->n;
			}
		} else {
			ns = surfaceIsect->n;
		}

		//计算切线
		Vector3f ss;
		if (_mesh->tangents) {
			ss = (b0 * _mesh->tangents[_vertexIndices[0]]
					+ b1 * _mesh->tangents[_vertexIndices[1]]
					+ b2 * _mesh->tangents[_vertexIndices[2]]);
			if (ss.LengthSquared() > 0.0f)
				ss = Normalize(ss);
			else {
				ss = Normalize(surfaceIsect->dpdu);
			}
		} else {
			ss = Normalize(surfaceIsect->dpdu);
		}

		//计算次切线
		Vector3f ts = Cross(ss, ns);
		if (ts.LengthSquared() > 0.0f) {
			ts = Normalize(ts);
			ss = Cross(ts, ns);
		} else {
			CoordinateSystem((Vector3f) ns, &ss, &ts);
		}

		//计算dndu和dndv
		Normal3f dndu, dndv;
		if (_mesh->normals) {
			Vector2f duv02 = uv[0] - uv[2];
			Vector2f duv12 = uv[1] - uv[2];
			Normal3f dn1 = _mesh->normals[_vertexIndices[0]]
					- _mesh->normals[_vertexIndices[2]];
			Normal3f dn2 = _mesh->normals[_vertexIndices[1]]
					- _mesh->normals[_vertexIndices[2]];
			Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
			bool degenerateUV = std::abs(determinant) < 1e-8;
			if (degenerateUV) {
				dndu = dndv = Normal3f(0, 0, 0);
			} else {
				Float invDet = 1 / determinant;
				dndu = (duv12[1] * dn1 - duv02[1] * dn2) * invDet;
				dndv = (-duv12[0] * dn1 + duv02[0] * dn2) * invDet;
			}
		} else {
			dndu = dndv = Normal3f(0, 0, 0);
		}
		//设置着色信息
		surfaceIsect->SetShadingGeometry(ss, ts, dndu, dndv, true);
	}

	//如果包含mesh法线，以mesh的法线的朝向为主朝向
	if (_mesh->normals) {
		surfaceIsect->n = Faceforward(surfaceIsect->n, surfaceIsect->shading.n);
	} else if (reverseOrientation ^ transformSwapsHandedness) {
		surfaceIsect->n = surfaceIsect->shading.n = -surfaceIsect->n;
	}
	//设置参数
	*tHit = t;
	return true;
}

bool Triangle::IntersectP(const Ray& ray, bool testAlpha) const {
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
	Vector3f d = Permute(ray.d, xk, yk, zk);
//使ray的方向指向局部坐标的(0,0,1)
	Float sx = -d.x / d.z;
	Float sy = -d.y / d.z;
	Float sz = 1.0f / d.z;
//只计算x,y的切变换,z的变换在确定和三角相交的时候再执行
//验证一句话，好程序要抠门>_<
	v1t.x += v1t.z * sx;
	v1t.y += v1t.z * sy;
	v2t.x += v2t.z * sx;
	v2t.y += v2t.z * sy;
	v3t.x += v3t.z * sx;
	v3t.y += v3t.z * sy;

//下面要开始计算edge function了
	Float e0 = v2t.x * v3t.y - v2t.y * v3t.x;
	Float e1 = v3t.x * v1t.y - v3t.y * v1t.x;
	Float e2 = v1t.x * v2t.y - v1t.y * v2t.x;
#ifdef FLOAT_IS_DOUBLE
#else
	if (e0 == 0 || e1 == 0 || e2 == 0) {
		e0 = (double)v2t.x * (double)v3t.y - (double)v2t.y * (double)v3t.x;
		e1 = (double)v3t.x * (double)v1t.y - (double)v3t.y * (double)v1t.x;
		e2 = (double)v1t.x * (double)v2t.y - (double)v1t.y * (double)v2t.x;
	}
#endif
//判断是否相交
	if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0)) {
		return false;
	}
	Float det = e0 + e1 + e2;
//两条向量共边了,那就不是三角形了
	if (det == 0) {
		return false;
	}
//补上z的缩放
	v1t.z *= sz;
	v2t.z *= sz;
	v3t.z *= sz;

//判断是否是有效的t值
	Float s = e0 * v1t.z + e1 * v2t.z + e2 * v3t.z;
	if (det < 0 && (s >= 0 || s < det * ray.tMax)) {
		return false;
	} else if (det > 0 && (s <= 0 || s > det * ray.tMax)) {
		return false;
	}
	Float invDet = 1.0 / det;
//		Float b0 = e0 * invDet;
//		Float b1 = e1 * invDet;
//		Float b2 = e2 * invDet;
	Float t = s * invDet;
	//计算质心坐标的时候产生的误差
	Float maxZt = MaxComponent(Abs(Vector3f(v1t.z, v2t.z, v3t.z)));
	Float deltaZ = gamma(3) * maxZt;
	Float maxXt = MaxComponent(Abs(Vector3f(v1t.x, v2t.x, v3t.x)));
	Float maxYt = MaxComponent(Abs(Vector3f(v1t.y, v2t.y, v3t.y)));
	Float deltaX = gamma(5) * (maxXt + maxZt);
	Float deltaY = gamma(5) * (maxYt + maxZt);
	Float deltaE = 2
			* (gamma(2) * maxXt * maxYt + deltaY * maxXt + deltaX * maxYt);
	Float maxE = MaxComponent(Abs(Vector3f(e0, e1, e2)));
	Float deltaT = 3
			* (gamma(3) * maxE * maxZt + deltaE * maxZt + deltaZ * maxE)
			* std::abs(invDet);
	if (t <= deltaT) {
		return false;
	}
//走到这就说明相交了
	return true;
}



std::vector<std::shared_ptr<Shape>> CreateTriangleMesh(
	const Transform *o2w, const Transform *w2o, bool reverseOrientation,
	int nTriangles, const int *vertexIndices, int nVertices, const Point3f *p,
	const Vector3f *s, const Normal3f *n, const Point2f *uv) {
	//创建trianglemesh
	std::shared_ptr<TriangleMesh> mesh = std::make_shared<TriangleMesh>(
		*o2w, nTriangles, vertexIndices, nVertices, p, s, n, uv);
	std::vector<std::shared_ptr<Shape>> tris;
	tris.reserve(nTriangles);
	for (int i = 0; i < nTriangles; ++i) {
		tris.push_back(std::make_shared<Triangle>(o2w, w2o,
			reverseOrientation, mesh, i));
	}
	return tris;
}

std::vector<std::shared_ptr<Shape>> CreateTriangleMeshShape(
	const Transform *o2w, const Transform *w2o, bool reverseOrientation,
	const ParamSet &params) {
	int nvi, npi, nuvi, nsi, nni;
	const int *vi = params.FindInt("indices", &nvi);
	const Point3f *P = params.FindPoint3f("P", &npi);
	const Point2f *uvs = params.FindPoint2f("uv", &nuvi);
	if (!uvs) uvs = params.FindPoint2f("st", &nuvi);
	std::vector<Point2f> tempUVs;
	if (!uvs) {
		const Float *fuv = params.FindFloat("uv", &nuvi);
		if (!fuv) fuv = params.FindFloat("st", &nuvi);
		if (fuv) {
			nuvi /= 2;
			tempUVs.reserve(nuvi);
			for (int i = 0; i < nuvi; ++i)
				tempUVs.push_back(Point2f(fuv[2 * i], fuv[2 * i + 1]));
			uvs = &tempUVs[0];
		}
	}
	if (uvs) {
		if (nuvi < npi) {
			Error(
				"Not enough of \"uv\"s for triangle mesh.  Expected "<< npi <<", "
				"found "<< nuvi <<".  Discarding.");
			uvs = nullptr;
		}
		else if (nuvi > npi) {
			Warning(
				"More \"uv\"s provided than will be used for triangle "
				"mesh.  (" << npi << " expcted, " << nuvi << " found)");
		}
	}
	if (!vi) {
		Error(
			"Vertex indices \"indices\" not provided with triangle mesh shape");
		return std::vector<std::shared_ptr<Shape>>();
	}
	if (!P) {
		Error("Vertex positions \"P\" not provided with triangle mesh shape");
		return std::vector<std::shared_ptr<Shape>>();
	}
	const Vector3f *S = params.FindVector3f("S", &nsi);
	if (S && nsi != npi) {
		Error("Number of \"S\"s for triangle mesh must match \"P\"s");
		S = nullptr;
	}
	const Normal3f *N = params.FindNormal3f("N", &nni);
	if (N && nni != npi) {
		Error("Number of \"N\"s for triangle mesh must match \"P\"s");
		N = nullptr;
	}
	for (int i = 0; i < nvi; ++i) {
		if (vi[i] >= npi) {
			Error(
				"trianglemesh has out of-bounds vertex index " << vi[i] << " (" << npi << " \"P\" "
				"values were given)");
			return std::vector<std::shared_ptr<Shape>>();
		}
	}
	Debug("[CreateTriangleMeshShape triangle num:"<< nvi / 3 <<"]");
	return CreateTriangleMesh(o2w, w2o, reverseOrientation, nvi / 3, vi, npi, P,
		S, N, uvs);
}
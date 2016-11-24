/*
 * transform.cpp
 *
 *  Created on: 2016年11月16日
 *      Author: zhuqian
 */

#include "transform.h"
#include "interaction.h"
Matrix4x4::Matrix4x4(Float mm[4][4]) {
	memcpy(m, mm, 16 * sizeof(Float));
	Assert(!HasNaNs());
}

Matrix4x4 Transpose(const Matrix4x4 &mm) {
	Assert(!mm.HasNaNs());
	return Matrix4x4(mm.m[0][0], mm.m[1][0], mm.m[2][0], mm.m[3][0], mm.m[0][1],
			mm.m[1][1], mm.m[2][1], mm.m[3][1], mm.m[0][2], mm.m[1][2],
			mm.m[2][2], mm.m[3][2], mm.m[0][3], mm.m[1][3], mm.m[2][3],
			mm.m[3][3]);
}

//求逆矩阵 使用高斯-约旦法
//代码来自PBRTV3
//自己不会实现。。。这个轮子我放弃
Matrix4x4 Inverse(const Matrix4x4 &m) {
	Assert(!m.HasNaNs());
	int indxc[4], indxr[4];
	int ipiv[4] = { 0, 0, 0, 0 };
	Float minv[4][4];
	memcpy(minv, m.m, 4 * 4 * sizeof(Float));
	for (int i = 0; i < 4; i++) {
		int irow = 0, icol = 0;
		Float big = 0.f;
		// Choose pivot
		for (int j = 0; j < 4; j++) {
			if (ipiv[j] != 1) {
				for (int k = 0; k < 4; k++) {
					if (ipiv[k] == 0) {
						if (std::abs(minv[j][k]) >= big) {
							big = Float(std::abs(minv[j][k]));
							irow = j;
							icol = k;
						}
					} else if (ipiv[k] > 1)
						Error("Singular matrix in MatrixInvert");
				}
			}
		}
		++ipiv[icol];
		// Swap rows _irow_ and _icol_ for pivot
		if (irow != icol) {
			for (int k = 0; k < 4; ++k)
				std::swap(minv[irow][k], minv[icol][k]);
		}
		indxr[i] = irow;
		indxc[i] = icol;
		if (minv[icol][icol] == 0.f)
			Error("Singular matrix in MatrixInvert");

		// Set $m[icol][icol]$ to one by scaling row _icol_ appropriately
		Float pivinv = 1. / minv[icol][icol];
		minv[icol][icol] = 1.;
		for (int j = 0; j < 4; j++)
			minv[icol][j] *= pivinv;

		// Subtract this row from others to zero out their columns
		for (int j = 0; j < 4; j++) {
			if (j != icol) {
				Float save = minv[j][icol];
				minv[j][icol] = 0;
				for (int k = 0; k < 4; k++)
					minv[j][k] -= minv[icol][k] * save;
			}
		}
	}
	// Swap columns to reflect permutation
	for (int j = 3; j >= 0; j--) {
		if (indxr[j] != indxc[j]) {
			for (int k = 0; k < 4; k++)
				std::swap(minv[k][indxr[j]], minv[k][indxc[j]]);
		}
	}
	return Matrix4x4(minv);
}

bool Transform::SwapsHandedness() const {
//计算行列式值
//这里只计算3x3矩阵的行列式就够了
	Float det = _m.m[0][0] * (_m.m[1][1] * _m.m[2][2] - _m.m[2][1] * _m.m[1][2])
			- _m.m[0][1] * (_m.m[1][0] * _m.m[2][2] - _m.m[2][0] * _m.m[1][2])
			+ _m.m[0][2] * (_m.m[1][0] * _m.m[2][1] - _m.m[2][0] * _m.m[1][1]);
	return det < 0;
}

//位移转换 参考PBRT
Transform Translate(const Vector3f &delta) {
	Matrix4x4 m(1, 0, 0, delta.x, 0, 1, 0, delta.y, 0, 0, 1, delta.z, 0, 0, 0,
			1);
	Matrix4x4 minv(1, 0, 0, -delta.x, 0, 1, 0, -delta.y, 0, 0, 1, -delta.z, 0,
			0, 0, 1);
	return Transform(m, minv);
}

//缩放
Transform Scale(Float x, Float y, Float z) {
	Matrix4x4 m(x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1);
	Matrix4x4 minv(1.0f / x, 0, 0, 0, 0, 1.0f / y, 0, 0, 0, 0, 1.0f / z, 0, 0,
			0, 0, 1);
	return Transform(m, minv);
}

//旋转矩阵的逆 等于旋转矩阵的转置  所以是正交矩阵
Transform RotateX(Float angle) {
	float sinR = std::sin(Radians(angle));
	float cosR = std::cos(Radians(angle));
	Matrix4x4 m(1, 0, 0, 0, 0, cosR, -sinR, 0, 0, sinR, cosR, 0, 0, 0, 0, 1);
	return Transform(m, Transpose(m));
}

Transform RotateY(float angle) {
	float sinR = std::sin(Radians(angle));
	float cosR = std::cos(Radians(angle));
	Matrix4x4 m(cosR, 0, sinR, 0, 0, 1, 0, 0, -sinR, 0, cosR, 0, 0, 0, 0, 1);
	return Transform(m, Transpose(m));
}

Transform RotateZ(float angle) {
	float sinR = std::sin(Radians(angle));
	float cosR = std::cos(Radians(angle));
	Matrix4x4 m(cosR, -sinR, 0, 0, sinR, cosR, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	return Transform(m, Transpose(m));
}

//计算 绕任意轴旋转某角度的方法
// v`=v_c+v_p*cos(theta)+v_2*sin(theta);
Transform Rotate(Float angle, const Vector3f &axis) {
	Vector3f a = Normalize(axis);
	float s = std::sin(Radians(angle));
	float c = std::cos(Radians(angle));
	float m[4][4];

	m[0][0] = a.x * a.x + (1.f - a.x * a.x) * c;
	m[0][1] = a.x * a.y * (1.f - c) - a.z * s;
	m[0][2] = a.x * a.z * (1.f - c) + a.y * s;
	m[0][3] = 0;

	m[1][0] = a.x * a.y * (1.f - c) + a.z * s;
	m[1][1] = a.y * a.y + (1.f - a.y * a.y) * c;
	m[1][2] = a.y * a.z * (1.f - c) - a.x * s;
	m[1][3] = 0;

	m[2][0] = a.x * a.z * (1.f - c) - a.y * s;
	m[2][1] = a.y * a.z * (1.f - c) + a.x * s;
	m[2][2] = a.z * a.z + (1.f - a.z * a.z) * c;
	m[2][3] = 0;

	m[3][0] = 0;
	m[3][1] = 0;
	m[3][2] = 0;
	m[3][3] = 1;

	Matrix4x4 mat = Matrix4x4(m);
	return Transform(mat, Transpose(mat));
}

Transform Orthographic(float znear, float zfar) {
	return Scale(1.f, 1.f, 1.f / (zfar - znear))
			* Translate(Vector3f(0.f, 0.f, -znear));
}

Transform Perspective(float fov, float n, float f) {
	Matrix4x4 persp = Matrix4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, f / (f - n),
			-f * n / (f - n), 0, 0, 1, 0);
	//使用fov来缩放到标准空间
	float invTanAng = 1.f / tanf(Radians(fov) / 2.f);
	return Scale(invTanAng, invTanAng, 1) * Transform(persp);
}

Transform Transform::operator*(const Transform& tran) const {
	Matrix4x4 m = this->_m * tran._m;
	Matrix4x4 mInv = tran._invM * this->_invM;
	return Transform(m, mInv);
}

//对SurfaceInteraction进行变换
SurfaceInteraction Transform::operator()(const SurfaceInteraction& si) const{
	SurfaceInteraction ret;
	const Transform& t=(*this);
	ret.p=t(si.p,si.pErr,&ret.pErr);
	ret.n=Normalize(t(si.n));//变换法线
	ret.wo=Normalize(t(si.wo));
	ret.uv=si.uv;
	ret.time=si.time;
	ret.mediumInterface=si.mediumInterface;
	ret.shape=si.shape;
	//todo赋值BRDF相关
	//todo赋值Primitive相关

	//空间点与参数之间的梯度
	ret.dpdu=t(si.dpdu);
	ret.dpdv=t(si.dpdv);
	//法线与参数之间的梯度
	ret.dndu=t(si.dndu);
	ret.dndv=t(si.dndv);
	//空间点与屏幕空间之间的梯度
	ret.dpdx=t(si.dpdx);
	ret.dpdy=t(si.dpdy);
	//参数与屏幕空间之间的梯度
	ret.dudx=si.dudx;
	ret.dvdx=si.dvdx;
	ret.dudy=si.dudy;
	ret.dvdy=si.dvdy;
	//变换着色相关变量
	ret.shading.n=Normalize(t(si.shading.n));
	ret.shading.dpdu=t(si.shading.dpdu);
	ret.shading.dpdv=t(si.shading.dpdv);
	ret.shading.dndu=t(si.shading.dndu);
	ret.shading.dndv=t(si.shading.dndv);

	//使着色法线和结构法线在同一个半球中
	ret.shading.n=Faceforward(ret.shading.n,ret.n);
	return ret;
}

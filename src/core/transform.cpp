/*
 * transform.cpp
 *
 *  Created on: 2016年11月16日
 *      Author: zhuqian
 */

#include "transform.h"

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


bool Transform::SwapsHandedness() const{
//计算行列式值
//这里只计算3x3矩阵的行列式就够了
	Float det=_m.m[0][0]*(_m.m[1][1]*_m.m[2][2]-_m.m[2][1]*_m.m[1][2])
			-_m.m[0][1]*(_m.m[1][0]*_m.m[2][2]-_m.m[2][0]*_m.m[1][2])
			+_m.m[0][2]*(_m.m[1][0]*_m.m[2][1]-_m.m[2][0]*_m.m[1][1]);
	return det<0;
}


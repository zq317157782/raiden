/*
 * transform.h
 *
 *  Created on: 2016年11月16日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_TRANSFORM_H_
#define SRC_CORE_TRANSFORM_H_

#include "raiden.h"
struct Matrix4x4 {
	Float m[4][4];

	Matrix4x4() {
		m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
		m[0][1] = m[0][2] = m[0][3] = 0.0f;
		m[1][0] = m[1][2] = m[1][3] = 0.0f;
		m[2][0] = m[2][1] = m[2][3] = 0.0f;
		m[3][0] = m[3][1] = m[3][2] = 0.0f;
	}

	Matrix4x4(Float mm[4][4]);

	Matrix4x4(Float t00, Float t01, Float t02, Float t03, Float t10, Float t11,
			Float t12, Float t13, Float t20, Float t21, Float t22, Float t23,
			Float t30, Float t31, Float t32, Float t33) {
		m[0][0] = t00;
		m[0][1] = t01;
		m[0][2] = t02;
		m[0][3] = t03;
		m[1][0] = t10;
		m[1][1] = t11;
		m[1][2] = t12;
		m[1][3] = t13;
		m[2][0] = t20;
		m[2][1] = t21;
		m[2][2] = t22;
		m[2][3] = t23;
		m[3][0] = t30;
		m[3][1] = t31;
		m[3][2] = t32;
		m[3][3] = t33;
		Assert(!HasNaNs());
	}

	bool operator==(const Matrix4x4& mat) const {
		Assert(!mat.HasNaNs());
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				if (m[i][j] != mat.m[i][j])
					return false;
		return true;
	}

	bool operator!=(const Matrix4x4& mat) const {
		Assert(!mat.HasNaNs());
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				if (m[i][j] != mat.m[i][j])
					return true;
		return false;
	}
	//求转置矩阵
	friend Matrix4x4 Transpose(const Matrix4x4 &mm);
	//求逆矩阵
	friend Matrix4x4 Inverse(const Matrix4x4 &mm);

	//矩阵相乘
	Matrix4x4 operator*(const Matrix4x4 &mm) const {
		Matrix4x4 mat;
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				mat.m[i][j] = m[i][0] * mm.m[0][j] + m[i][1] * mm.m[1][j]
						+ m[i][2] * mm.m[2][j] + m[i][3] * mm.m[3][j];
		return mat;
	}

	//判断变量中是否包含nan分量
	bool HasNaNs() const {
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				if (std::isnan(m[i][j]))
					return true;
		return false;
	}

//	//矩阵相乘
//	static Matrix4x4 Mul(const Matrix4x4 &mm1, const Matrix4x4 &mm2) {
//		Matrix4x4 mat;
//		for (int i = 0; i < 4; ++i)
//			for (int j = 0; j < 4; ++j)
//				mat.m[i][j] = mm1.m[i][0] * mm2.m[0][j]
//						+ mm1.m[i][1] * mm2.m[1][j] + mm1.m[i][2] * mm2.m[2][j]
//						+ mm1.m[i][3] * mm2.m[3][j];
//		return mat;
//	}
};

//变换
class Transform {
private:
	Matrix4x4 _m, _invM;
public:
	Transform() {

	}

	Transform(Float mm[4][4]) {
		_m = Matrix4x4(mm);
		_invM = Inverse(_m); //取逆
		Assert(!_m.HasNaNs());
	}

	Transform(const Matrix4x4& mm) :
			_m(mm), _invM(Inverse(mm)) {
		Assert(!_m.HasNaNs());
	}

	Transform(const Matrix4x4& mm, const Matrix4x4& invMm) :
			_m(mm), _invM(invMm) {
		Assert(!_m.HasNaNs());
		Assert(!_invM.HasNaNs());
	}
	friend Transform Inverse(const Transform &t) {
		return Transform(t._invM, t._m);
	}
	friend Transform Transpose(const Transform &t) {
		return Transform(Transpose(t._m), Transpose(t._invM));
	}
	bool operator==(const Transform &t) const {
		return t._m == _m && t._invM == _invM;
	}
	bool operator!=(const Transform &t) const {
		return t._m != _m || t._invM != _invM;
	}

	bool operator<(const Transform &t2) const {
		//从第一个元素往最后一个元素比较
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j) {
				if (_m.m[i][j] < t2._m.m[i][j])
					return true;
				if (_m.m[i][j] > t2._m.m[i][j])
					return false;
			}
		return false;
	}

	bool IsIdentity() const {
		return (_m.m[0][0] == 1.0f && _m.m[0][1] == 0.0f && _m.m[0][2] == 0.0f
				&& _m.m[0][3] == 0.f && _m.m[1][0] == 0.0f && _m.m[1][1] == 1.0f
				&& _m.m[1][2] == 0.0f && _m.m[1][3] == 0.0f
				&& _m.m[2][0] == 0.0f && _m.m[2][1] == 0.0f
				&& _m.m[2][2] == 1.0f && _m.m[2][3] == 0.0f
				&& _m.m[3][0] == 0.0f && _m.m[3][1] == 0.0f
				&& _m.m[3][2] == 0.0f && _m.m[3][3] == 1.0f);
	}

	inline const Matrix4x4 &GetMatrix() const {
		return _m;
	}
	inline const Matrix4x4 &GetInverseMatrix() const {
		return _invM;
	}
};

#endif /* SRC_CORE_TRANSFORM_H_ */

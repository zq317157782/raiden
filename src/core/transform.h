/*
 * transform.h
 *
 *  Created on: 2016年11月16日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_TRANSFORM_H_
#define SRC_CORE_TRANSFORM_H_

#include "raiden.h"
#include "geometry.h"
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

	friend std::ostream &operator<<(std::ostream &os, const Matrix4x4 &m) {
		os << "[[" << m.m[0][0] << " " << m.m[0][1] << " " << m.m[0][2] << " "
				<< m.m[0][3] << "]" << " [" << m.m[1][0] << " " << m.m[1][1]
				<< " " << m.m[1][2] << " " << m.m[1][3] << "]" << " ["
				<< m.m[2][0] << " " << m.m[2][1] << " " << m.m[2][2] << " "
				<< m.m[2][3] << "]" << " [" << m.m[3][0] << " " << m.m[3][1]
				<< " " << m.m[3][2] << " " << m.m[3][3] << "]]";
		return os;
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

	//以下转换操作基本都提供了两个版本，一个普通的变换以及另外一个计算浮点数误差的版本
	template<typename T>
	inline Point3<T> operator()(const Point3<T>& p) const;
	template<typename T>
	inline Point3<T> operator()(const Point3<T>& p, Vector3<T>* err) const;
	template<typename T>
	inline Point3<T> operator()(const Point3<T>& p, const Vector3<T>& cErr,
			Vector3<T>* err) const;
	template<typename T>
	inline Vector3<T> operator()(const Vector3<T>& v) const;
	template<typename T>
	inline Vector3<T> operator()(const Vector3<T>& v, Vector3<T>* err) const;
	template<typename T>
	inline Vector3<T> operator()(const Vector3<T>& v, const Vector3<T>& cErr,
			Vector3<T>* err) const;
	template<typename T>
	inline Normal3<T> operator()(const Normal3<T>& n) const;
	inline Ray operator()(const Ray& ray) const;
	inline Ray operator()(const Ray& ray, Vector3f *oErr,
			Vector3f *odErr) const;
	inline Ray operator()(const Ray& ray, const Vector3f& coErr,
			const Vector3f& cdErr, Vector3f *oErr, Vector3f *odErr) const;
	inline RayDifferential operator()(const RayDifferential& ray) const;
	inline Bound3f operator()(const Bound3f& bound) const;
	inline SurfaceInteraction operator()(const SurfaceInteraction&) const;//变换表面相交点

	bool SwapsHandedness() const; //判断当前是否变换了坐标系true:右手坐标系 false:左手坐标系

	friend std::ostream &operator<<(std::ostream &os, const Transform &t) {
		os << "t=" << t._m << ", inv=" << t._invM;
		return os;
	}

	//变换之间的相乘，不满足交换律
	Transform operator*(const Transform& tran) const;
};

//对Point3的变换操作
template<typename T>
inline Point3<T> Transform::operator()(const Point3<T>& p) const {
	T x = p.x, y = p.y, z = p.z;
	T xx = _m.m[0][0] * x + _m.m[0][1] * y + _m.m[0][2] * z + _m.m[0][3];	//*1
	T yy = _m.m[1][0] * x + _m.m[1][1] * y + _m.m[1][2] * z + _m.m[1][3];	//*1
	T zz = _m.m[2][0] * x + _m.m[2][1] * y + _m.m[2][2] * z + _m.m[2][3];	//*1
	T ww = _m.m[3][0] * x + _m.m[3][1] * y + _m.m[3][2] * z + _m.m[3][3];	//*1
	Assert(ww != 0.0f);
	if (ww == 1.0f) {
		return Point3<T>(xx, yy, zz);
	} else {
		return Point3<T>(xx, yy, zz) / ww;
	}
}

template<typename T>
inline Point3<T> Transform::operator()(const Point3<T>& p,
		Vector3<T>* err/*绝对误差*/) const {
	T x = p.x, y = p.y, z = p.z;
	T xx = _m.m[0][0] * x + _m.m[0][1] * y + _m.m[0][2] * z + _m.m[0][3];	//*1
	T yy = _m.m[1][0] * x + _m.m[1][1] * y + _m.m[1][2] * z + _m.m[1][3];	//*1
	T zz = _m.m[2][0] * x + _m.m[2][1] * y + _m.m[2][2] * z + _m.m[2][3];	//*1
	T ww = _m.m[3][0] * x + _m.m[3][1] * y + _m.m[3][2] * z + _m.m[3][3];	//*1

	//计算transform对于point的浮点数误差边界
	T absX = std::abs(_m.m[0][0] * x) + std::abs(_m.m[0][1] * y)
			+ std::abs(_m.m[0][2] * z) + std::abs(_m.m[0][3]);
	T absY = std::abs(_m.m[1][0] * x) + std::abs(_m.m[1][1] * y)
			+ std::abs(_m.m[1][2] * z) + std::abs(_m.m[1][3]);
	T absZ = std::abs(_m.m[2][0] * x) + std::abs(_m.m[2][1] * y)
			+ std::abs(_m.m[2][2] * z) + std::abs(_m.m[2][3]);
	*err = Vector3<T>(absX, absY, absZ) * gamma(3);

	Assert(ww != 0.0f);
	if (ww == 1.0f) {
		return Point3<T>(xx, yy, zz);
	} else {
		return Point3<T>(xx, yy, zz) / ww;
	}
}
template<typename T>
inline Point3<T> Transform::operator()(const Point3<T>& p,
		const Vector3<T>& cErr/*累积误差*/, Vector3<T>* err/*绝对误差*/) const {
	T x = p.x, y = p.y, z = p.z;
	T xx = _m.m[0][0] * x + _m.m[0][1] * y + _m.m[0][2] * z + _m.m[0][3];	//*1
	T yy = _m.m[1][0] * x + _m.m[1][1] * y + _m.m[1][2] * z + _m.m[1][3];	//*1
	T zz = _m.m[2][0] * x + _m.m[2][1] * y + _m.m[2][2] * z + _m.m[2][3];	//*1
	T ww = _m.m[3][0] * x + _m.m[3][1] * y + _m.m[3][2] * z + _m.m[3][3];	//*1

	//计算transform对于point的浮点数误差边界
	T absX = std::abs(_m.m[0][0] * x) + std::abs(_m.m[0][1] * y)
			+ std::abs(_m.m[0][2] * z) + std::abs(_m.m[0][3]);
	T absY = std::abs(_m.m[1][0] * x) + std::abs(_m.m[1][1] * y)
			+ std::abs(_m.m[1][2] * z) + std::abs(_m.m[1][3]);
	T absZ = std::abs(_m.m[2][0] * x) + std::abs(_m.m[2][1] * y)
			+ std::abs(_m.m[2][2] * z) + std::abs(_m.m[2][3]);
	*err = Vector3<T>(absX, absY, absZ) * gamma(3);

	//这里是把累积误差也考虑进去
	err->x += (gamma(3) + (T) 1)
			* (std::abs(_m.m[0][0]) * cErr.x + std::abs(_m.m[0][1]) * cErr.y
					+ std::abs(_m.m[0][2]) * cErr.z);
	err->y += (gamma(3) + (T) 1)
			* (std::abs(_m.m[1][0]) * cErr.x + std::abs(_m.m[1][1]) * cErr.y
					+ std::abs(_m.m[1][2]) * cErr.z);
	err->z += (gamma(3) + (T) 1)
			* (std::abs(_m.m[2][0]) * cErr.x + std::abs(_m.m[2][1]) * cErr.y
					+ std::abs(_m.m[2][2]) * cErr.z);

	Assert(ww != 0.0f);
	if (ww == 1.0f) {
		return Point3<T>(xx, yy, zz);
	} else {
		return Point3<T>(xx, yy, zz) / ww;
	}
}

//对Vector3的变换操作
template<typename T>
inline Vector3<T> Transform::operator()(const Vector3<T>& p) const {
	T x = p.x, y = p.y, z = p.z;
	T xx = _m.m[0][0] * x + _m.m[0][1] * y + _m.m[0][2] * z;	//+_m.m[0][3]*0
	T yy = _m.m[1][0] * x + _m.m[1][1] * y + _m.m[1][2] * z;	//+_m.m[1][3]*0
	T zz = _m.m[2][0] * x + _m.m[2][1] * y + _m.m[2][2] * z;	//+_m.m[2][3]*0
	//T ww=_m.m[3][0]*x+_m.m[3][1]*y+_m.m[3][2]*z;//+_m.m[3][3]*0
	return Vector3<T>(xx, yy, zz);
}

template<typename T>
inline Vector3<T> Transform::operator()(const Vector3<T>& v,
		Vector3<T>* err/*绝对误差*/) const {
	T x = v.x, y = v.y, z = v.z;
	T xx = _m.m[0][0] * x + _m.m[0][1] * y + _m.m[0][2] * z;	//+_m.m[0][3]*0
	T yy = _m.m[1][0] * x + _m.m[1][1] * y + _m.m[1][2] * z;	//+_m.m[1][3]*0
	T zz = _m.m[2][0] * x + _m.m[2][1] * y + _m.m[2][2] * z;	//+_m.m[2][3]*0
	/*其实这里应该是 gamma(3)*m*x+gamma(3)*m*y+gamma(2)*m*z; 但是多一个gamma(1)*m*z并不影响什么，还是保守误差边界*/
	err->x = (std::abs(_m.m[0][0] * x) + std::abs(_m.m[0][1] * y)
			+ std::abs(_m.m[0][2] * z)) * gamma(3);
	err->y = (std::abs(_m.m[1][0] * x) + std::abs(_m.m[1][1] * y)
			+ std::abs(_m.m[1][2] * z)) * gamma(3);
	err->z = (std::abs(_m.m[2][0] * x) + std::abs(_m.m[2][1] * y)
			+ std::abs(_m.m[2][2] * z)) * gamma(3);
	return Vector3<T>(xx, yy, zz);
}

template<typename T>
inline Vector3<T> Transform::operator()(const Vector3<T>& v,
		const Vector3<T>& cErr, Vector3<T>* err) const {
	T x = v.x, y = v.y, z = v.z;
	T xx = _m.m[0][0] * x + _m.m[0][1] * y + _m.m[0][2] * z;	//+_m.m[0][3]*0
	T yy = _m.m[1][0] * x + _m.m[1][1] * y + _m.m[1][2] * z;	//+_m.m[1][3]*0
	T zz = _m.m[2][0] * x + _m.m[2][1] * y + _m.m[2][2] * z;	//+_m.m[2][3]*0

	/*其实这里应该是 gamma(3)*m*x+gamma(3)*m*y+gamma(2)*m*z; 但是多一个gamma(1)*m*z并不影响什么，还是保守误差边界*/
	err->x = (std::abs(_m.m[0][0] * x) + std::abs(_m.m[0][1] * y)
			+ std::abs(_m.m[0][2] * z)) * gamma(3);
	err->y = (std::abs(_m.m[1][0] * x) + std::abs(_m.m[1][1] * y)
			+ std::abs(_m.m[1][2] * z)) * gamma(3);
	err->z = (std::abs(_m.m[2][0] * x) + std::abs(_m.m[2][1] * y)
			+ std::abs(_m.m[2][2] * z)) * gamma(3);
	//计算累积误差
	err->x += (gamma(3) + (T) 1)
			* (std::abs(_m.m[0][0]) * cErr.x + std::abs(_m.m[0][1]) * cErr.y
					+ std::abs(_m.m[0][2]) * cErr.z);
	err->y += (gamma(3) + (T) 1)
			* (std::abs(_m.m[1][0]) * cErr.x + std::abs(_m.m[1][1]) * cErr.y
					+ std::abs(_m.m[1][2]) * cErr.z);
	err->z += (gamma(3) + (T) 1)
			* (std::abs(_m.m[2][0]) * cErr.x + std::abs(_m.m[2][1]) * cErr.y
					+ std::abs(_m.m[2][2]) * cErr.z);

	return Vector3<T>(xx, yy, zz);
}

//对Normal3的变换操作
template<typename T>
inline Normal3<T> Transform::operator()(const Normal3<T>& n) const {
	T x = n.x, y = n.y, z = n.z;
	T xx = _m.m[0][0] * x + _m.m[0][1] * y + _m.m[0][2] * z;	//+_m.m[0][3]*0
	T yy = _m.m[1][0] * x + _m.m[1][1] * y + _m.m[1][2] * z;	//+_m.m[1][3]*0
	T zz = _m.m[2][0] * x + _m.m[2][1] * y + _m.m[2][2] * z;	//+_m.m[2][3]*0
	//T ww=_m.m[3][0]*x+_m.m[3][1]*y+_m.m[3][2]*z;//+_m.m[3][3]*0
	return Normal3<T>(xx, yy, zz);
}

//对射线的变换
inline Ray Transform::operator()(const Ray& r) const {
	Vector3f err;	//用来记录对原点进行transform后引入的err
	Point3f o = (*this)(r.o, &err);
	Vector3f d = (*this)(r.d);
	Float tMax = r.tMax;
	Float dLengthSquared = d.LengthSquared();
	if (dLengthSquared > 0) {
		Float offset = Dot(err, Abs(d)) / dLengthSquared;//全部用绝对值来计算就能获得偏离原点最小的合理的offset
		o += (d * offset);
		tMax -= offset;	//tMax需要缩减
	}
	return Ray(o, d, tMax, r.time, r.medium);
}
inline Ray Transform::operator()(const Ray& r, Vector3f *err/*射线起点的绝对误差*/,
		Vector3f *dErr/*射线方向的绝对误差*/) const {
	Point3f o = (*this)(r.o, err);
	Vector3f d = (*this)(r.d, dErr);
	Float tMax = r.tMax;
	Float dLengthSquared = d.LengthSquared();
	if (dLengthSquared > 0) {
		Float offset = Dot(*err, Abs(d)) / dLengthSquared;//全部用绝对值来计算就能获得偏离原点最小的合理的offset
		o += (d * offset);
		tMax -= offset;	//tMax需要缩减
	}
	return Ray(o, d, tMax, r.time, r.medium);
}

inline Ray Transform::operator()(const Ray& r, const Vector3f& coErr,
		const Vector3f& cdErr, Vector3f *err, Vector3f *dErr) const {
	Point3f o = (*this)(r.o, coErr, err);
	Vector3f d = (*this)(r.d, cdErr, dErr);
	Float tMax = r.tMax;
	Float dLengthSquared=d.LengthSquared();
		if(dLengthSquared>0){
			Float offset = Dot(*err, Abs(d))/dLengthSquared;	//全部用绝对值来计算就能获得偏离原点最小的合理的offset
			o += (d * offset);
			tMax -= offset;	//tMax需要缩减
	}
	return Ray(o, d, tMax, r.time, r.medium);
}

//对微分射线的变换
inline RayDifferential Transform::operator()(const RayDifferential& ray) const {
	Ray r = (*this)(Ray(ray));
	RayDifferential rayDifferential = RayDifferential(r.o, r.d, r.tMax, r.time);
	if (ray.hasDifferential) {
		rayDifferential.hasDifferential = ray.hasDifferential;
		rayDifferential.ox = (*this)(ray.ox);
		rayDifferential.oy = (*this)(ray.oy);
		rayDifferential.dx = (*this)(ray.dx);
		rayDifferential.dy = (*this)(ray.dy);
	}
	return rayDifferential;
}

//对AABB包围盒进行变换
inline Bound3f Transform::operator()(const Bound3f& b) const {
	const Transform& T = (*this);
	//对包围盒的8个顶点都进行变换，然后求合并
	Bound3f ret(T(Point3f(b.minPoint.x, b.minPoint.y, b.minPoint.z)));
	Union(ret, T(Point3f(b.maxPoint.x, b.minPoint.y, b.minPoint.z)));
	Union(ret, T(Point3f(b.maxPoint.x, b.maxPoint.y, b.minPoint.z)));
	Union(ret, T(Point3f(b.maxPoint.x, b.maxPoint.y, b.maxPoint.z)));
	Union(ret, T(Point3f(b.minPoint.x, b.maxPoint.y, b.maxPoint.z)));
	Union(ret, T(Point3f(b.minPoint.x, b.minPoint.y, b.maxPoint.z)));
	Union(ret, T(Point3f(b.minPoint.x, b.maxPoint.y, b.minPoint.z)));
	Union(ret, T(Point3f(b.maxPoint.x, b.minPoint.y, b.maxPoint.z)));
	return ret;
}


Transform Translate(const Vector3f &delta);
Transform Scale(Float x, Float y, Float z);

Transform RotateX(Float angle);
Transform RotateY(Float angle);
Transform RotateZ(Float angle);
Transform Rotate(Float angle, const Vector3f &axis);
Transform Orthographic(Float znear, Float zfar);	//正交变换
Transform Perspective(Float fov, Float znear, Float zfar);//透视变换 fov:field of view

#endif /* SRC_CORE_TRANSFORM_H_ */

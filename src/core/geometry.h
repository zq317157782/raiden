/*
 * geometry.h
 *
 *  Created on: 2016年11月9日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_GEOMETRY_H_
#define SRC_CORE_GEOMETRY_H_
#include "raiden.h"

//判断是否有NaN的值出现，只需要在DEBUG状态下才需要定义
#ifdef DEBUG_BUILD
template<typename T>
inline bool IsNaN(T n) {
	return std::isnan(n);
}
#endif

//三维向量
template<typename T>
class Vector3 {
public:
	Float x, y, z;
public:
	Vector3() {
		x = y = z = 0;
	}
	Vector3(T xx, T yy, T zz) :
			x(xx), y(yy), z(zz) {
		//只有为每个分量单独赋值的时候才需要下NaN的断言
		Assert(!HasNaNs());
	}

	Vector3(const Vector3<T>& v) {
		Assert(!v.HasNaNs());
		x = v.x;
		y = v.y;
		z = v.z;
	}

	explicit Vector3(const Point3<T>& p) {
		Assert(!p.HasNaNs());
		x = p.x;
		y = p.y;
		z = p.z;
	}

	Vector3<T>& operator=(const Vector3<T>& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	Vector3<T> operator+(const Vector3<T>& v) const {
		Assert(!v.HasNaNs());
		return Vector3<T>(x + v.x, y + v.y, z + v.z);
	}

	Vector3<T>& operator+=(const Vector3<T>& v) {
		Assert(!v.HasNaNs());
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3<T> operator-(const Vector3<T>& v) const {
		Assert(!v.HasNaNs());
		return Vector3<T>(x - v.x, y - v.y, z - v.z);
	}

	Vector3<T>& operator-=(const Vector3<T>& v) {
		Assert(!v.HasNaNs());
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	template<typename U>
	Vector3<T> operator*(U n) const {
		Assert(!IsNaN(n));
		return Vector3<T>(x * n, y * n, z * n);
	}

	template<typename U>
	Vector3<T>& operator*=(U n) {
		Assert(!IsNaN(n));
		x *= n;
		y *= n;
		z *= n;
		return *this;
	}

	template<typename U>
	Vector3<T> operator/(U n) const {
		Assert(!IsNaN(n));
		Assert(n != 0);
		T f = (Float) 1 / n;
		return Vector3<T>(f * x, f * y, f * z);
	}

	template<typename U>
	Vector3<T>& operator/=(U n) {
		Assert(!IsNaN(n));
		Assert(n != 0);
		T f = (Float) 1 / n;
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	Vector3<T> operator-() const {
		return Vector3<T>(-x, -y, -z);
	}

	bool operator==(const Vector3<T>& v) const {
		if (x == v.x && y == v.y && z == v.z)
			return true;
		return false;
	}

	bool operator!=(const Vector3<T>& v) const {
		if (x != v.x || y != v.y || z != v.z)
			return true;
		return false;
	}

	//返回向量的数量级的平方
	T MagnitudeSquared() const {
		return x * x + y * y + z * z;
	}

	T LengthSquared() const {
		return MagnitudeSquared();
	}

	//返回向量的数量级 有开根操作
	T Magnitude() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	T Length() const {
		return Magnitude();
	}

	T operator[](int index) const {
		Assert(index >= 0 && index < 3);
		return (&x)[index];
	}

	T& operator[](int index) {
		Assert(index >= 0 && index < 3);
		return (&x)[index];
	}

	//重构ostream方法
	friend std::ostream &operator<<(std::ostream &os, const Vector3<T> &v) {
		os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
		return os;
	}
#ifdef DEBUG_BUILD
	//判断三个分量中有没有NaN的变量
	bool HasNaNs() const {
		return ::IsNaN(x) || ::IsNaN(y) || ::IsNaN(z);
	}
#endif
};

typedef Vector3<Float> Vector3f;
typedef Vector3<int> Vector3i;

//二维向量
template<typename T>
class Vector2 {
public:
	Float x, y;
public:
	Vector2() {
		x = y = 0;
	}
	Vector2(T xx, T yy) :
			x(xx), y(yy) {
		//只有为每个分量单独赋值的时候才需要下NaN的断言
		Assert(!HasNaNs());
	}

	Vector2(const Vector2<T>& v) {
		Assert(!v.HasNaNs());
		x = v.x;
		y = v.y;
	}

	explicit Vector2(const Point2<T> p) {
		Assert(!p.HasNaNs());
		x = p.x;
		y = p.y;
	}
	Vector2<T>& operator=(const Vector2<T>& v) {
		Assert(v.HasNaNs());
		x = v.x;
		y = v.y;
		return *this;
	}

	Vector2<T> operator+(const Vector2<T>& v) const {
		Assert(!v.HasNaNs());
		return Vector2<T>(x + v.x, y + v.y);
	}

	Vector2<T>& operator+=(const Vector2<T>& v) {
		Assert(!v.HasNaNs());
		x += v.x;
		y += v.y;
		return *this;
	}

	Vector2<T> operator-(const Vector2<T>& v) const {
		Assert(!v.HasNaNs());
		return Vector2<T>(x - v.x, y - v.y);
	}

	Vector2<T>& operator-=(const Vector2<T>& v) {
		Assert(!v.HasNaNs());
		x -= v.x;
		y -= v.y;
		return *this;
	}

	template<typename U>
	Vector2<T> operator*(U n) const {
		Assert(!IsNaN(n));
		return Vector2<T>(x * n, y * n);
	}

	template<typename U>
	Vector2<T>& operator*=(U n) {
		Assert(!IsNaN(n));
		x *= n;
		y *= n;
		return *this;
	}

	template<typename U>
	Vector2<T> operator/(U n) const {
		Assert(!IsNaN(n));
		Assert(n != 0);
		T f = (Float) 1 / n;
		return Vector2<T>(f * x, f * y);
	}

	template<typename U>
	Vector2<T>& operator/=(U n) {
		Assert(!IsNaN(n));
		Assert(n != 0);
		T f = (Float) 1 / n;
		x *= f;
		y *= f;
		return *this;
	}

	Vector2<T> operator-() const {
		return Vector2<T>(-x, -y);
	}

	bool operator==(const Vector2<T>& v) const {
		if (x == v.x && y == v.y)
			return true;
		return false;
	}

	bool operator!=(const Vector2<T>& v) const {
		if (x != v.x || y != v.y)
			return true;
		return false;
	}

	//返回向量的数量级的平方
	T MagnitudeSquared() const {
		return x * x + y * y;
	}

	T LengthSquared() const {
		return MagnitudeSquared();
	}

	//返回向量的数量级 有开根操作
	T Magnitude() const {
		return std::sqrt(x * x + y * y);
	}

	T Length() const {
		return Magnitude();
	}

	T operator[](int index) const {
		Assert(index >= 0 && index < 2);
		return (&x)[index];
	}

	T& operator[](int index) {
		Assert(index >= 0 && index < 2);
		return (&x)[index];
	}

	//重构ostream方法
	friend std::ostream &operator<<(std::ostream &os, const Vector2<T> &v) {
		os << "[" << v.x << ", " << v.y << "]";
		return os;
	}
#ifdef DEBUG_BUILD
	//判断分量中有没有NaN的变量
	bool HasNaNs() const {
		return ::IsNaN(x) || ::IsNaN(y);
	}
#endif
};
typedef Vector2<Float> Vector2f;
typedef Vector2<int> Vector2i;

template<typename T>
inline Float Dot(const Vector3<T>& v1, const Vector3<T>& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template<typename T>
inline Float Dot(const Vector2<T>& v1, const Vector2<T>& v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

template<typename T>
inline Float AbsDot(const Vector3<T>& v1, const Vector3<T>& v2) {
	return std::fabsf(Dot(v1, v2));
}

template<typename T>
inline Float AbsDot(const Vector2<T>& v1, const Vector2<T>& v2) {
	return std::fabsf(Dot(v1, v2));
}

//基于左手坐标系
template<typename T>
inline Vector3<T> Cross(const const Vector3<T>& v1, const const Vector3<T>& v2) {
	return Vector3<T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x);
}

template<typename T>
inline Vector3<T> Normalize(const Vector3<T>& v) {
	return v / v.Magnitude();
}
template<typename T>
inline Vector2<T> Normalize(const Vector2<T>& v) {
	return v / v.Magnitude();
}

//标量乘以向量的操作，其实就是换个位置，使用向量乘以标量的方式
template<typename T,typename U>
inline Vector3<T> operator*(U n,const Vector3<T>& v){
	return v*n;
}

template<typename T>
class Point3 {
public:
	T x, y, z;
public:
	Point3() {
		x = y = z = 0;
	}

	Point3(T xx, T yy, T zz) :
			x(xx), y(yy), z(zz) {
		Assert(!HasNaNs());
	}

	Point3(const Point3<T>& p) {
		Assert(!p.HasNaNs());
		x = p.x;
		y = p.y;
		z = p.z;

	}

	Point3<T>& operator=(const Point3<T>& p) {
		Assert(!p.HasNaNs());
		x = p.x;
		y = p.y;
		z = p.z;
		return *this;
	}

	Point3<T> operator+(const Point3<T>& p) const {
		Assert(!p.HasNaNs());
		return Point3<T>(x + p.x, y + p.y, z + p.z);
	}

	Point3<T>& operator+=(const Point3<T>& p) {
		Assert(!p.HasNaNs());
		x += p.x;
		y += p.y;
		z += p.z;
		return *this;
	}

	//两个空间点相减 返回的是一个空间向量
	Vector3<T> operator-(const Point3<T>& p) const {
		Assert(!p.HasNaNs());
		return Vector3<T>(x - p.x, y - p.y, z - p.z);
	}

	template<typename U>
	Point3<T> operator*(U u) const {
		Assert(!IsNaN(u));
		return Point3<T>(x * u, y * u, z * u);
	}

	template<typename U>
	Point3<T>& operator*=(U u) {
		Assert(!IsNaN(u));
		x *= u;
		y *= u;
		z *= u;
		return *this;
	}

	template<typename U>
	Point3<T> operator/(U u) const {
		Assert(!IsNaN(u));
		Assert(u != 0);
		Float f = (Float) 1 / u;
		return Point3<T>(x * f, y * f, z * f);
	}

	template<typename U>
	Point3<T>& operator/=(U u) {
		Assert(!IsNaN(u));
		Assert(u != 0);
		Float f = (Float) 1 / u;
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	Point3<T> operator-() const {
		return Point3<T>(-x, -y, -z);
	}

	//和Vector相关的操作
	Point3<T> operator+(const Vector3<T>& v) const {
		Assert(!v.HasNaNs());
		return Point3<T>(x + v.x, y + v.y, z + v.z);
	}
	Point3<T>& operator+=(const Vector3<T>& v) {
		Assert(!v.HasNaNs());
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	bool operator==(const Point3<T>& p) const {
		if (p.x == x && p.y == y && p.z == z)
			return true;
		return false;
	}

	bool operator!=(const Point3<T>& p) const {
		if (p.x != x || p.y != y || p.z != z)
			return true;
		return false;
	}

	T operator[](int index) const {
		Assert(index >= 0 && index < 3);
		return (&x)[index];
	}
	T& operator[](int index) {
		Assert(index >= 0 && index < 3);
		return (&x)[index];
	}

	//重构ostream方法
	friend std::ostream &operator<<(std::ostream &os, const Point3<T> &v) {
		os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
		return os;
	}

#ifdef DEBUG_BUILD
	//判断分量中有没有NaN的变量
	bool HasNaNs() const {
		return ::IsNaN(x) || ::IsNaN(y);
	}
#endif
};

typedef Point3<Float> Point3f;
typedef Point3<int> Point3i;

template<typename T>
class Point2 {
public:
	T x, y;
public:
	Point2() {
		x = y = 0;
	}

	Point2(T xx, T yy, T zz) :
			x(xx), y(yy) {
		Assert(!HasNaNs());
	}

	Point2(const Point2<T>& p) {
		Assert(!p.HasNaNs());
		x = p.x;
		y = p.y;
	}

	Point2<T>& operator=(const Point2<T>& p) {
		Assert(!p.HasNaNs());
		x = p.x;
		y = p.y;
		return *this;
	}

	Point2<T> operator+(const Point2<T>& p) const {
		Assert(!p.HasNaNs());
		return Point2<T>(x + p.x, y + p.y);
	}

	Point2<T>& operator+=(const Point2<T>& p) {
		Assert(!p.HasNaNs());
		x += p.x;
		y += p.y;
		return *this;
	}

	Vector2<T> operator-(const Point2<T>& p) const {
		Assert(!p.HasNaNs());
		return Point2<T>(x - p.x, y - p.y);
	}

	template<typename U>
	Point2<T> operator*(U u) const {
		Assert(!IsNaN(u));
		return Point2<T>(x * u, y * u);
	}

	template<typename U>
	Point2<T>& operator*=(U u) {
		Assert(!IsNaN(u));
		x *= u;
		y *= u;
		return *this;
	}

	template<typename U>
	Point2<T> operator/(U u) const {
		Assert(!IsNaN(u));
		Assert(u != 0);
		Float f = (Float) 1 / u;
		return Point2<T>(x * f, y * f);
	}

	template<typename U>
	Point2<T>& operator/=(U u) {
		Assert(!IsNaN(u));
		Assert(u != 0);
		Float f = (Float) 1 / u;
		x *= f;
		y *= f;
		return *this;
	}

	Point2<T> operator-() const {
		return Point2<T>(-x, -y);
	}

	//和Vector相关的操作
	Point2<T> operator+(const Vector3<T>& v) const {
		Assert(!v.HasNaNs());
		return Point2<T>(x + v.x, y + v.y);
	}
	Point2<T>& operator+=(const Vector3<T>& v) {
		Assert(!v.HasNaNs());
		x += v.x;
		y += v.y;
		return *this;
	}

	bool operator==(const Point2<T>& p) const {
		if (p.x == x && p.y == y)
			return true;
		return false;
	}

	bool operator!=(const Point2<T>& p) const {
		if (p.x != x || p.y != y)
			return true;
		return false;
	}

	T operator[](int index) const {
		Assert(index >= 0 && index < 3);
		return (&x)[index];
	}
	T& operator[](int index) {
		Assert(index >= 0 && index < 3);
		return (&x)[index];
	}

	//重构ostream方法
	friend std::ostream &operator<<(std::ostream &os, const Point2<T> &v) {
		os << "[" << v.x << ", " << v.y << "]";
		return os;
	}

#ifdef DEBUG_BUILD
	//判断分量中有没有NaN的变量
	bool HasNaNs() const {
		return ::IsNaN(x) || ::IsNaN(y);
	}
#endif
};

typedef Point2<Float> Point2f;
typedef Point2<int> Point2i;

template<typename T>
inline Float DistanceSquared(const Point3<T>& p1, const Point3<T>& p2) {
	Assert(!p1.HasNaNs() && !p2.HasNaNs());
	return (p1 - p2).MagnitudeSquared();
}

template<typename T>
inline Float Distance(const Point3<T>& p1, const Point3<T>& p2) {
	return std::sqrt(DistanceSquared(p1, p2));
}

template<typename T>
inline Float DistanceSquared(const Point2<T>& p1, const Point2<T>& p2) {
	Assert(!p1.HasNaNs() && !p2.HasNaNs());
	return (p1 - p2).MagnitudeSquared();
}

template<typename T>
inline Float Distance(const Point2<T>& p1, const Point2<T>& p2) {
	return std::sqrt(DistanceSquared(p1, p2));
}

#endif /* SRC_CORE_GEOMETRY_H_ */

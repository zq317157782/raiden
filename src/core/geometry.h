/*
 * geometry.h
 *
 *  Created on: 2016年11月9日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_GEOMETRY_H_
#define SRC_CORE_GEOMETRY_H_
#include "raiden.h"

template<typename T>
inline bool IsNaN(T n) {
	return std::isnan(n);
}

//todo Vector3向量类
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
		x = v.x;
		y = v.y;
		z = v.z;
	}

	Vector3<T>& operator=(const Vector3<T>& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	Vector3<T> operator+(const Vector3<T>& v) {
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

	Vector3<T> operator-(const Vector3<T>& v) {
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


	Vector3<T> operator*(T n) {
		Assert(!IsNaN(n));
		return Vector3<T>(x * n, y * n, z * n);
	}

	Vector3<T>& operator*=(T n) {
		Assert(!IsNaN(n));
		x *= n;
		y *= n;
		z *= n;
		return *this;
	}

	Vector3<T> operator-() const {
		return Vector3<T>(-x, -y, -z);
	}



private:
	//判断三个分量中有没有NaN的变量
	bool HasNaNs() const{
		return ::IsNaN(x) || ::IsNaN(y) || ::IsNaN(z);
	}
};

#endif /* SRC_CORE_GEOMETRY_H_ */

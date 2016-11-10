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

	//todo explicit Vector3(const Point3<T>)
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
private:
	//判断三个分量中有没有NaN的变量
	bool HasNaNs() const {
		return ::IsNaN(x) || ::IsNaN(y) || ::IsNaN(z);
	}
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
		x = v.x;
		y = v.y;
	}

	Vector2<T>& operator=(const Vector3<T>& v) {
		x = v.x;
		y = v.y;
		return *this;
	}

	//todo explicit Vector2(const Point2<T>)
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
			return x * x + y * y ;
		}

		T LengthSquared() const {
			return MagnitudeSquared();
		}

		//返回向量的数量级 有开根操作
		T Magnitude() const {
			return std::sqrt(x * x + y * y );
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
private:
	//判断三个分量中有没有NaN的变量
	bool HasNaNs() const {
		return ::IsNaN(x) || ::IsNaN(y);
	}
};
typedef Vector2<Float> Vector2f;
typedef Vector2<int> Vector2i;

#endif /* SRC_CORE_GEOMETRY_H_ */

/*
 * geometry.h
 *
 *  Created on: 2016年11月9日
 *      Author: zhuqian
 *
 *
 *
 *
 */



#ifndef SRC_CORE_GEOMETRY_H_
#define SRC_CORE_GEOMETRY_H_
#include "raiden.h"

//判断是否有NaN的值出现
template<typename T>
inline bool IsNaN(T n) {
	//return !(n==n);//使用自己和自己比较的方式 如果一个数是NaN的话，它和任意数比较都为false,但是编译器可能会进行优化，导致失效
	return std::isnan(n); //这是使用标准库的判断方法
}

template<>
inline bool IsNaN(int n) {
	return false; 
}


/*
* 通用的空间三维向量类型,包含(x,y,z)三个分量
* 因为是通用的三维向量，所以这里的实现方式没有使用SSE2或者DoD，因为某些通用应用场合，并无法从SSE或者DoD获益
*/
template<typename T>
class Vector3 {
public:
	T x, y, z;
public:
	inline Vector3() {
		x = y = z = 0;
	}
	inline Vector3(T xx, T yy, T zz) :
			x(xx), y(yy), z(zz) {
		//只有为每个分量单独赋值的时候才需要下NaN的断言
		Assert(!HasNaNs());
	}
	inline Vector3(T v) :
			x(v), y(v), z(v) {
		//只有为每个分量单独赋值的时候才需要下NaN的断言
		Assert(!HasNaNs());
	}

	template<typename T1>
	inline explicit Vector3(const Point3<T1>& p):x(p.x),y(p.y),z(p.z){
		Assert(!HasNaNs());
	}

	template<typename T1>
	inline explicit Vector3(const Normal3<T1>& n):x(n.x),y(n.y),z(n.z) {
		Assert(!HasNaNs());
	}	

	template<typename T1> 
	inline explicit operator Vector3<T1>() const{
		return Vector3<T1>(x,y,z);
	}
//这里默认的赋值函数和复制函数都不错，所以只在DEBUG模式下才需要自己定义，并且下断言来调试
#ifdef RAIDEN_DEBUG
	inline Vector3(const Vector3<T>& v):x(v.x),y(v.y),z(v.z){
		Assert(!HasNaNs());
	}

	inline Vector3<T>& operator=(const Vector3<T>& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		Assert(!HasNaNs());
		return *this;
	}
#endif

	inline Vector3<T> operator+(const Vector3<T>& v) const {
		Assert(!v.HasNaNs());
		return Vector3<T>(x + v.x, y + v.y, z + v.z);
	}

	inline Vector3<T>& operator+=(const Vector3<T>& v) {
		Assert(!v.HasNaNs());
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	inline Vector3<T> operator-(const Vector3<T>& v) const {
		Assert(!v.HasNaNs());
		return Vector3<T>(x - v.x, y - v.y, z - v.z);
	}

	inline Vector3<T>& operator-=(const Vector3<T>& v) {
		Assert(!v.HasNaNs());
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	template<typename U>
	inline Vector3<T> operator*(U n) const {
		Assert(!IsNaN(n));
		return Vector3<T>(x * n, y * n, z * n);
	}

	template<typename U>
	inline Vector3<T>& operator*=(U n) {
		Assert(!IsNaN(n));
		x *= n;
		y *= n;
		z *= n;
		return *this;
	}

	template<typename U>
	inline Vector3<T> operator/(U n) const {
		Assert(!IsNaN(n));
		Assert(n != 0);
		T f = (Float) 1 / n;
		return Vector3<T>(f * x, f * y, f * z);
	}

	template<typename U>
	inline Vector3<T>& operator/=(U n) {
		Assert(!IsNaN(n));
		Assert(n != 0);
		T f = (Float) 1 / n;
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	inline Vector3<T> operator-() const {
		return Vector3<T>(-x, -y, -z);
	}

	inline bool operator==(const Vector3<T>& v) const {
		if (x == v.x && y == v.y && z == v.z){
			return true;
		}	
		return false;
	}

	inline bool operator!=(const Vector3<T>& v) const {
		if (x != v.x || y != v.y || z != v.z){
			return true;
		}
		return false;
	}

	inline T LengthSquared() const {
		return x * x + y * y + z * z;
	}


	inline T Length() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	inline T operator[](int index) const {
		Assert(index >= 0 && index < 3);
		return (&x)[index];
	}

	inline T& operator[](int index) {
		Assert(index >= 0 && index < 3);
		return (&x)[index];
	}

	//重构ostream方法
	inline friend std::ostream &operator<<(std::ostream &os, const Vector3<T> &v) {
		os << "[ " << v.x << " , " << v.y << " , " << v.z << " ]";
		return os;
	}
	//判断三个分量中有没有NaN的变量
	inline bool HasNaNs() const {
		return ::IsNaN(x) || ::IsNaN(y) || ::IsNaN(z);
	}
};

typedef Vector3<Float> Vector3f;
typedef Vector3<int> Vector3i;

//二维向量
template<typename T>
class  Vector2 {
public:
	T x, y;
public:
	inline Vector2():x(0),y(0){
	}
	inline Vector2(T xx, T yy):x(xx), y(yy) {
		//只有为每个分量单独赋值的时候才需要下NaN的断言
		Assert(!HasNaNs());
	}

	inline Vector2(T v):x(v),y(v){
		Assert(!HasNaNs());
	}

	template<typename T1>
	inline explicit Vector2(const Point2<T1>& p):x(p.x),y(p.y){
		Assert(!HasNaNs());
	}

	template<typename T1>
	inline explicit Vector2(const Vector3<T1>& v):x(v.x),y(v.y){
		Assert(!HasNaNs());
	}

	template<typename T1> 
	inline explicit operator Vector2<T1>() const{
		return Vector2<T1>(x,y);
	}
	
//这里默认的赋值函数和复制函数都不错，所以只在DEBUG模式下才需要自己定义，并且下断言来调试
#ifdef RAIDEN_DEBUG
	inline Vector2(const Vector2<T>& v):x(v.x),y(v.y){
		Assert(!HasNaNs());
	}
	inline Vector2<T>& operator=(const Vector2<T>& v) {
		x = v.x;
		y = v.y;
		Assert(!HasNaNs());
		return *this;
	}
#endif
	inline Vector2<T> operator+(const Vector2<T>& v) const {
		Assert(!v.HasNaNs());
		return Vector2<T>(x + v.x, y + v.y);
	}

	inline Vector2<T>& operator+=(const Vector2<T>& v) {
		Assert(!v.HasNaNs());
		x += v.x;
		y += v.y;
		return *this;
	}

	inline Vector2<T> operator-(const Vector2<T>& v) const {
		Assert(!v.HasNaNs());
		return Vector2<T>(x - v.x, y - v.y);
	}

	inline Vector2<T>& operator-=(const Vector2<T>& v) {
		Assert(!v.HasNaNs());
		x -= v.x;
		y -= v.y;
		return *this;
	}

	template<typename U>
	inline Vector2<T> operator*(U n) const {
		Assert(!IsNaN(n));
		return Vector2<T>(x * n, y * n);
	}

	template<typename U>
	inline Vector2<T>& operator*=(U n) {
		Assert(!IsNaN(n));
		x *= n;
		y *= n;
		return *this;
	}

	template<typename U>
	inline Vector2<T> operator/(U n) const {
		Assert(!IsNaN(n));
		Assert(n != 0);
		T f = (Float) 1 / n;
		return Vector2<T>(f * x, f * y);
	}

	template<typename U>
	inline Vector2<T>& operator/=(U n) {
		Assert(!IsNaN(n));
		Assert(n != 0);
		T f = (Float) 1 / n;
		x *= f;
		y *= f;
		return *this;
	}

	inline Vector2<T> operator-() const {
		return Vector2<T>(-x, -y);
	}

	inline bool operator==(const Vector2<T>& v) const {
		if (x == v.x && y == v.y)
			return true;
		return false;
	}

	inline bool operator!=(const Vector2<T>& v) const {
		if (x != v.x || y != v.y)
			return true;
		return false;
	}

	//返回向量的数量级的平方
	inline T LengthSquared() const {
		return x * x + y * y;
	}

	//返回向量的数量级 有开根操作
	inline T Length() const {
		return std::sqrt(x * x + y * y);
	}

	inline T operator[](int index) const {
		Assert(index >= 0 && index < 2);
		return (&x)[index];
	}

	inline T& operator[](int index) {
		Assert(index >= 0 && index < 2);
		return (&x)[index];
	}

	//重构ostream方法
	inline friend std::ostream &operator<<(std::ostream &os, const Vector2<T> &v) {
		os << "[" << v.x << ", " << v.y << "]";
		return os;
	}
	//判断分量中有没有NaN的变量
	inline bool HasNaNs() const {
		return ::IsNaN(x) || ::IsNaN(y);
	}
};
typedef Vector2<Float> Vector2f;
typedef Vector2<int> Vector2i;

template<typename T>
class  Point3 {
public:
	T x, y, z;
public:
	inline Point3():x(0),y(0),z(0){
	}
	inline Point3(T xx, T yy, T zz) :
			x(xx), y(yy), z(zz) {
		Assert(!HasNaNs());
	}
	inline Point3(T v):x(v),y(v),z(v){
		Assert(!HasNaNs());
	}

	template<typename T1>
    inline explicit Point3(const Vector3<T1>& v):x(v.x),y(v.y),z(v.z){
        Assert(!HasNaNs());
    }

	template<typename T1> 
	inline explicit operator Point3<T1>() const{
		return Point3<T1>(x,y,z);
	}

//这里默认的赋值函数和复制函数都不错，所以只在DEBUG模式下才需要自己定义，并且下断言来调试
#ifdef RAIDEN_DEBUG
	inline Point3(const Point3<T>& p):x(p.x),y(p.y),z(p.z){
		Assert(!HasNaNs());
	}
	inline Point3<T>& operator=(const Point3<T>& p) {
		x = p.x;
		y = p.y;
		z = p.z;
		Assert(!HasNaNs());
		return *this;
	}
#endif

	inline Point3<T> operator+(const Point3<T>& p) const {
		Assert(!p.HasNaNs());
		return Point3<T>(x + p.x, y + p.y, z + p.z);
	}

	inline Point3<T>& operator+=(const Point3<T>& p) {
		Assert(!p.HasNaNs());
		x += p.x;
		y += p.y;
		z += p.z;
		return *this;
	}

	//两个空间点相减 返回的是一个空间向量
	inline Vector3<T> operator-(const Point3<T>& p) const {
		Assert(!p.HasNaNs());
		return Vector3<T>(x - p.x, y - p.y, z - p.z);
	}

	template<typename U>
	inline Point3<T> operator*(U u) const {
		Assert(!IsNaN(u));
		return Point3<T>(x * u, y * u, z * u);
	}

	template<typename U>
	inline Point3<T>& operator*=(U u) {
		Assert(!IsNaN(u));
		x *= u;
		y *= u;
		z *= u;
		return *this;
	}

	template<typename U>
	inline Point3<T> operator/(U u) const {
		Assert(!IsNaN(u));
		Assert(u != 0);
		Float f = (Float) 1 / u;
		return Point3<T>(x * f, y * f, z * f);
	}

	template<typename U>
	inline Point3<T>& operator/=(U u) {
		Assert(!IsNaN(u));
		Assert(u != 0);
		Float f = (Float) 1 / u;
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	inline Point3<T> operator-() const {
		return Point3<T>(-x, -y, -z);
	}

	//和Vector相关的操作
	inline Point3<T> operator+(const Vector3<T>& v) const {
		Assert(!v.HasNaNs());
		return Point3<T>(x + v.x, y + v.y, z + v.z);
	}
	inline Point3<T> operator-(const Vector3<T>& v) const {
		Assert(!v.HasNaNs());
		return Point3<T>(x - v.x, y - v.y, z - v.z);
	}

	inline Point3<T>& operator+=(const Vector3<T>& v) {
		Assert(!v.HasNaNs());
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	inline bool operator==(const Point3<T>& p) const {
		if (p.x == x && p.y == y && p.z == z)
			return true;
		return false;
	}

	inline bool operator!=(const Point3<T>& p) const {
		if (p.x != x || p.y != y || p.z != z)
			return true;
		return false;
	}

	inline T operator[](int index) const {
		Assert(index >= 0 && index < 3);
		return (&x)[index];
	}
	inline T& operator[](int index) {
		Assert(index >= 0 && index < 3);
		return (&x)[index];
	}

	//重构ostream方法
	inline friend std::ostream &operator<<(std::ostream &os, const Point3<T> &v) {
		os << "[ " << v.x << " , " << v.y << " , " << v.z << " ]";
		return os;
	}

	//判断分量中有没有NaN的变量
	inline bool HasNaNs() const {
		return ::IsNaN(x) || ::IsNaN(y);
	}
};

typedef Point3<Float> Point3f;
typedef Point3<int> Point3i;

template<typename T>
class  Point2 {
public:
	T x, y;
public:
	inline Point2() :x(0),y(0){
	}
	inline Point2(T xx, T yy) :
			x(xx), y(yy) {
		Assert(!HasNaNs());
	}
	inline Point2(T v):x(v),y(v){
		Assert(!HasNaNs());
	}
	
	template<typename T1>
	inline explicit Point2(const Vector2<T1>& v):x(v.x),y(v.y){
		Assert(!HasNaNs());
	}

	template<typename T1>
	inline explicit Point2(const Point3<T1>& v):x(v.x),y(v.y){
		Assert(!HasNaNs());
	}

	template <typename T1>
    inline explicit operator Point2<T1>() const {
        return Point2<T1>(x, y);
    }

	#ifdef RAIDEN_DEBUG
	//这里默认的赋值函数和复制函数都不错，所以只在DEBUG模式下才需要自己定义，并且下断言来调试
	inline Point2(const Point2<T>& p):x(p.x),y(p.y){
		Assert(!HasNaNs());
	}

	inline Point2<T>& operator=(const Point2<T>& p) {
		x = p.x;
		y = p.y;
		Assert(!HasNaNs());
		return *this;
	}
	#endif

	inline Point2<T> operator+(const Point2<T>& p) const {
		Assert(!p.HasNaNs());
		return Point2<T>(x + p.x, y + p.y);
	}

	inline Point2<T> operator+(const Vector2<T>& p) const {
		Assert(!p.HasNaNs());
		return Point2<T>(x + p.x, y + p.y);
	}

	inline Point2<T>& operator+=(const Point2<T>& p) {
		Assert(!p.HasNaNs());
		x += p.x;
		y += p.y;
		return *this;
	}

	inline Vector2<T> operator-(const Point2<T>& p) const {
		Assert(!p.HasNaNs());
		return Vector2<T>(x - p.x, y - p.y);
	}

	inline Point2<T> operator-(const Vector2<T>& p) const {
		Assert(!p.HasNaNs());
		return Point2<T>(x - p.x, y - p.y);
	}

	template<typename U>
	inline Point2<T> operator*(U u) const {
		Assert(!IsNaN(u));
		return Point2<T>(x * u, y * u);
	}

	template<typename U>
	inline Point2<T>& operator*=(U u) {
		Assert(!IsNaN(u));
		x *= u;
		y *= u;
		return *this;
	}

	template<typename U>
	inline Point2<T> operator/(U u) const {
		Assert(!IsNaN(u));
		Assert(u != 0);
		Float f = (Float) 1 / u;
		return Point2<T>(x * f, y * f);
	}

	template<typename U>
	inline Point2<T>& operator/=(U u) {
		Assert(!IsNaN(u));
		Assert(u != 0);
		Float f = (Float) 1 / u;
		x *= f;
		y *= f;
		return *this;
	}

	inline Point2<T> operator-() const {
		return Point2<T>(-x, -y);
	}

	//和Vector相关的操作
	inline Point2<T> operator+(const Vector3<T>& v) const {
		Assert(!v.HasNaNs());
		return Point2<T>(x + v.x, y + v.y);
	}
	inline Point2<T>& operator+=(const Vector3<T>& v) {
		Assert(!v.HasNaNs());
		x += v.x;
		y += v.y;
		return *this;
	}

	inline bool operator==(const Point2<T>& p) const {
		if (p.x == x && p.y == y)
			return true;
		return false;
	}

	inline bool operator!=(const Point2<T>& p) const {
		if (p.x != x || p.y != y)
			return true;
		return false;
	}

	inline T operator[](int index) const {
		Assert(index >= 0 && index < 3);
		return (&x)[index];
	}
	inline T& operator[](int index) {
		Assert(index >= 0 && index < 3);
		return (&x)[index];
	}

	//重构ostream方法
	inline friend std::ostream &operator<<(std::ostream &os, const Point2<T> &v) {
		os << "[ " << v.x << " , " << v.y << " ]";
		return os;
	}
	//判断分量中有没有NaN的变量
	inline bool HasNaNs() const {
		return ::IsNaN(x) || ::IsNaN(y);
	}
};

typedef Point2<Float> Point2f;
typedef Point2<int> Point2i;

//三分量法线
template<typename T>
class  Normal3 {
public:
	T x, y, z;
public:
	inline Normal3():x(0),y(0),z(0) {
	}

	inline Normal3(T xx, T yy, T zz):x(xx), y(yy), z(zz) {
		Assert(!HasNaNs());
	}

	template<typename T1>
	inline explicit Normal3<T>(const Vector3<T1>& v) :
			x(v.x), y(v.y), z(v.z) {
		Assert(!HasNaNs());
	}

#ifdef RAIDEN_DEBUG
	inline Normal3(const Normal3& nl) {
		x = nl.x;
		y = nl.y;
		z = nl.z;
		Assert(!HasNaNs());
	}
	inline Normal3<T>& operator=(const Normal3& nl) {
		x = nl.x;
		y = nl.y;
		z = nl.z;
		Assert(!HasNaNs());
		return *this;
	}
#endif
	inline Normal3<T> operator-() const {
		return Normal3<T>(-x, -y, -z);
	}
	inline Normal3<T> operator+(const Normal3<T>& nl) const {
		Assert(!nl.HasNaNs());
		return Normal3<T>(x + nl.x, y + nl.y, z + nl.z);
	}

	inline Normal3<T>& operator+=(const Normal3<T>& nl) {
		x += nl.x;
		y += nl.y;
		z += nl.z;
		Assert(!HasNaNs());
		return *this;
	}
	inline Normal3<T> operator-(const Normal3<T>& n) const {
		Assert(!n.HasNaNs());
		return Normal3<T>(x - n.x, y - n.y, z - n.z);
	}

	inline Normal3<T> operator-=(const Normal3<T>& n) {
		x -= n.x;
		y -= n.y;
		z -= n.z;
		Assert(!HasNaNs());
		return *this;
	}

	template<typename U>
	inline Normal3<T> operator*(U n) const {
		Assert(!IsNaN(n));
		return Normal3<T>(x * n, y * n, z * n);
	}

	template<typename U>
	inline Normal3<T>& operator*=(U f) {
		Assert(!IsNaN(f));
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}
	template<typename U>
	inline Normal3<T> operator/(U f) const {
		Assert(!IsNaN(f));
		Assert(f != 0);
		Float reciprocal = 1.0 / f;
		return Normal3<T>(x * reciprocal, y * reciprocal, z * reciprocal);
	}

	template<typename U>
	inline Normal3<T>& operator/=(U f) {
		Assert(!IsNaN(f));
		Assert(f != 0);
		Float reciprocal = 1.0 / f;
		x *= reciprocal;
		y *= reciprocal;
		z *= reciprocal;
		return *this;
	}

	inline T operator[](int i) const {
		Assert(i >= 0 && i <= 2);
		return (&x)[i];
	}

	inline T& operator[](int i) {
		Assert(i >= 0 && i <= 2);
		return (&x)[i];
	}

	inline bool operator==(const Normal3<T>& n) const {
		Assert(!n.HasNaNs());
		if (x == n.x && y == n.y && z == n.z) {
			return true;
		}
		return false;
	}

	inline bool operator!=(const Normal3<T>& n) const {
		Assert(!n.HasNaNs());
		if (x != n.x || y != n.y || z != n.z) {
			return true;
		}
		return false;
	}
	inline T LengthSquared() const {
		return x * x + y * y + z * z;
	}
	inline T Length() const {
		return std::sqrt(x * x + y * y + z * z);
	}
	//判断分量中有没有NaN的变量
	inline bool HasNaNs() const {
		return ::IsNaN(x) || ::IsNaN(y) || ::IsNaN(z);
	}

	//重构ostream方法
	inline friend std::ostream &operator<<(std::ostream &os, const Normal3<T> &n) {
		os << "[ " << n.x << " , " << n.y << " , " << n.z << " ]";
		return os;
	}
};

typedef Normal3<Float> Normal3f;

//AABB盒
template<typename T>
class  Bound3 {
public:
//AABB盒的最小顶点和最大顶点
	Point3<T> minPoint, maxPoint;
public:
	inline Bound3() {
		//默认构造函数最小点取最大值，最大点取最小值
		//PBRT_V2中是取了float的两个无限值
		T minValue = std::numeric_limits<T>::lowest();//lowest是带符号最小的浮点数 min是不带符号最小的浮点数，不包括0
		T maxValue = std::numeric_limits<T>::max();
		minPoint = Point3<T>(maxValue, maxValue, maxValue);
		maxPoint = Point3<T>(minValue, minValue, minValue);
	}
	inline Bound3(const Point3<T>& p) :
			minPoint(p), maxPoint(p) {
	}
	inline Bound3(const Point3<T>& p1, const Point3<T>& p2) :
			minPoint(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
					std::min(p1.z, p2.z)), maxPoint(std::max(p1.x, p2.x),
					std::max(p1.y, p2.y), std::max(p1.z, p2.z)) {
	}

	//强制转换到其他类型的Bound3
	template<typename T1>
	inline explicit operator Bound3<T1>() const {
		return Bound3<T1>((Point3<T1>) minPoint, (Point3<T1>) maxPoint);
	}

	//这里通过索引来访问minPoint和maxPoint属性
	//这里用const ref提高访问class对象的速度，但是同时不能修改值，因为是引用
	inline const Point3<T>& operator[](int index) const {
		Assert(index >= 0 && index < 2);
		return (&minPoint)[index];
	}

	inline Point3<T>& operator[](int index) {
		Assert(index >= 0 && index < 2);
		return (&minPoint)[index];
	}

	//顺序0~7: 前：左下，右下，左上，右上，后：左下，右下，左上，右上
	inline Point3<T> Corner(int index) const {
		Assert(index >= 0 && index < 8);
		T x = (*this)[index & 1].x;	//偶数取minPoint.x 奇数取maxPoint.x;
		T y = (*this)[index & 2 ? 1 : 0].y;	//index第二位是0取minPoint.y,否则取maxPoint.y;
		T z = (*this)[index & 4 ? 1 : 0].z;	//类推z
		return Point3<T>(x, y, z);
	}

	//返回对角线向量
	inline Vector3<T> Diagonal() const {
		return (maxPoint - minPoint);
	}

	//求面积
	inline T SurfaceArea() const {
		Vector3<T> d = Diagonal();
		return (d.x * d.y + d.x * d.z + d.y * d.z) * 2.0;
	}

	//求体积
	inline T Volume() const {
		Vector3<T> d = Diagonal();
		return d.x * d.y * d.z;
	}

	//获取最大的边界
	inline int MaximumExtent() const {
		Vector3<T> diag = Diagonal();
		if (diag.x > diag.y && diag.x > diag.z){
			return 0;
		}
		else if (diag.y > diag.z){
			return 1;
		}
		else{
			return 2;
		}	
	}

	inline bool operator==(const Bound3<T>& b) const {
		if (minPoint == b.minPoint && maxPoint == b.maxPoint) {
			return true;
		}
		return false;
	}

	bool operator!=(const Bound3<T>& b) const {
		if (minPoint != b.minPoint || maxPoint != b.maxPoint) {
			return true;
		}
		return false;
	}

	//获得外接包围球
	inline void BoundingSphere(Point3<T>*c, Float* r) const {
		*c = (minPoint + maxPoint) / 2;
		if (Inside(*c, *this)) {
			*r = Distance(*c, maxPoint);
		} else {
			*r = 0;
		}
	}
	//计算点在碰撞盒中的偏移量
	inline Vector3<T> Offset(const Point3<T> &p) const {
		Vector3<T> o = p - minPoint;
		if (maxPoint.x > minPoint.x){
			o.x /= (maxPoint.x - minPoint.x);
		}
		if (maxPoint.y > minPoint.y){
			o.y /= (maxPoint.y - minPoint.y);
		}
		if (maxPoint.z > minPoint.z){
			o.z /= (maxPoint.z - minPoint.z);
		}
		return o;
	}

	//射线求角
	inline bool IntersectP(const Ray& ray,Float* tHit1=nullptr,Float *tHit2=nullptr) const;

	inline bool IntersectP(const Ray& ray, const Vector3f& reciprocalDir, const int isNeg[3]) const;
	//重构ostream方法
	inline friend std::ostream &operator<<(std::ostream &os, const Bound3<T> &n) {
		os << "< " << n.minPoint << " , " << n.maxPoint << " >";
		return os;
	}

};
typedef Bound3<Float> Bound3f;
typedef Bound3<int> Bound3i;

//平面Rect
template<typename T>
class Bound2 {
public:
	Point2<T> minPoint, maxPoint;
public:
	inline Bound2() {
		//默认构造函数最小点取最大值，最大点取最小值
		//PBRT_V2中是取了float的两个无限值
		T minValue = std::numeric_limits<T>::lowest();//lowest是带符号最小的浮点数 min是不带符号最小的浮点数，不包括0
		T maxValue = std::numeric_limits<T>::max();
		minPoint = Point2<T>(maxValue, maxValue);
		maxPoint = Point2<T>(minValue, minValue);
	}
	inline Bound2(const Point2<T>& p) :
			minPoint(p), maxPoint(p) {
	}
	inline Bound2(const Point2<T>& p1, const Point2<T>& p2) :
			minPoint(std::min(p1.x, p2.x), std::min(p1.y, p2.y)), maxPoint(
					std::max(p1.x, p2.x), std::max(p1.y, p2.y)) {
	}

	template<typename T1>
	inline explicit operator Bound2<T1>() const {
		return Bound2<T1>((Point2<T1>) minPoint, (Point2<T1>) maxPoint);
	}

	//这里通过索引来访问minPoint和maxPoint属性
	//这里用const ref提高访问class对象的速度，但是同时不能修改值，因为是引用
	inline const Point2<T>& operator[](int index) const {
		Assert(index >= 0 && index < 2);
		return (&minPoint)[index];
	}

	inline Point2<T>& operator[](int index) {
		Assert(index >= 0 && index < 2);
		return (&minPoint)[index];
	}

	inline Point2<T> Corner(int index) const {
		Assert(index >= 0 && index < 4);
		T x = (*this)[index & 1].x;	//偶数取minPoint.x 奇数取maxPoint.x;
		T y = (*this)[index & 2 ? 1 : 0].y;	//index第二位是0取minPoint.y,否则取maxPoint.y;
		return Point2<T>(x, y);
	}

	//返回对角线向量
	inline Vector2<T> Diagonal() const {
		return (maxPoint - minPoint);
	}

	//求面积
	inline T Area() const {
		Vector2<T> d = Diagonal();
		return d.x * d.y;
	}

	//获取最大的边界
	inline int MaximumExtent() const {
		Vector2<T> diag = Diagonal();
		if (diag.x > diag.y)
			return 0;
		else
			return 1;
	}

	inline bool operator==(const Bound2<T>& b) const {
		if (minPoint == b.minPoint && maxPoint == b.maxPoint) {
			return true;
		}
		return false;
	}

	inline bool operator!=(const Bound2<T>& b) const {
		if (minPoint != b.minPoint || maxPoint != b.maxPoint) {
			return true;
		}
		return false;
	}

	//获得包围圆
	inline void BoundingSphere(Point2<T>*c, Float* r) const {
		*c = (minPoint + maxPoint) / 2;
		if (Inside(*c, *this)) {
			*r = Distance(*c, maxPoint);
		} else {
			*r = 0;
		}
	}

	//重构ostream方法
	inline friend std::ostream &operator<<(std::ostream &os, const Bound2<T> &n) {
		os << "< " << n.minPoint << " , " << n.maxPoint << " >";
		return os;
	}

};
typedef Bound2<Float> Bound2f;
typedef Bound2<int> Bound2i;

//Bound2i的向前迭代器
#include <iterator>
class Bound2iIterator: public std::forward_iterator_tag {
public:
	inline Bound2iIterator(const Bound2i &b, const Point2i &pt) :
			_p(pt), _bounds(&b) {
	}
	inline Bound2iIterator operator++() {
		advance();
		return *this;
	}
	inline Bound2iIterator operator++(int) {
		Bound2iIterator old = *this;
		advance();
		return old;
	}
	inline bool operator==(const Bound2iIterator &bi) const {
		return _p == bi._p && _bounds == bi._bounds;
	}
	inline bool operator!=(const Bound2iIterator &bi) const {
		return _p != bi._p || _bounds != bi._bounds;
	}

	inline Point2i operator*() const {
		return _p;
	}

private:
	inline void advance() {
		++_p.x;
		if (_p.x == _bounds->maxPoint.x) {
			_p.x = _bounds->minPoint.x;
			++_p.y;
		}
	}
	Point2i _p;
	const Bound2i *_bounds;
};

inline Bound2iIterator begin(const Bound2i &b) {
	return Bound2iIterator(b, b.minPoint);
}

inline Bound2iIterator end(const Bound2i &b) {
	Point2i pEnd(b.minPoint.x, b.maxPoint.y);
	if (b.minPoint.x >= b.maxPoint.x || b.minPoint.y >= b.maxPoint.y)
		pEnd = b.minPoint;
	return Bound2iIterator(b, pEnd);
}

//AABB和point之间的合并
template<typename T>
inline Bound3<T> Union(const Bound3<T>& b, const Point3<T> p) {
	// return Bound3<T>(
	// 		Point3<T>(std::min(b.minPoint.x, p.x), std::min(b.minPoint.y, p.y),
	// 				std::min(b.minPoint.z, p.z)),
	// 		Point3<T>(std::max(b.maxPoint.x, p.x), std::max(b.maxPoint.y, p.y),
	// 				std::max(b.maxPoint.z, p.z)));
	Bound3<T> ret;
	ret.minPoint=Min(b.minPoint,p);
	ret.maxPoint=Max(b.maxPoint,p);
	return ret;
}
//AABB和AABB之间的合并
template<typename T>
inline Bound3<T> Union(const Bound3<T>& b, const Bound3<T>& b2) {
	// return Bound3<T>(
	// 		Point3<T>(std::min(b.minPoint.x, b2.minPoint.x),
	// 				std::min(b.minPoint.y, b2.minPoint.y),
	// 				std::min(b.minPoint.z, b2.minPoint.z)),
	// 		Point3<T>(std::max(b.maxPoint.x, b2.maxPoint.x),
	// 				std::max(b.maxPoint.y, b2.maxPoint.y),
	// 				std::max(b.maxPoint.z, b2.maxPoint.z)));
	Bound3<T> ret;
	ret.minPoint=Min(b.minPoint,b2.minPoint);
	ret.maxPoint=Max(b.maxPoint,b2.maxPoint);
	return ret;
}

//2D版本
template<typename T>
inline Bound2<T> Union(const Bound2<T>& b, const Point2<T> p) {
	Bound2<T> ret;
	ret.minPoint=Min(b.minPoint,p);
	ret.maxPoint=Max(b.maxPoint,p);
	return ret;
}

//AABB盒之间的交集
template<typename T>
inline Bound3<T> Intersect(const Bound3<T>& b, const Bound3<T>& b2) {
	// return Bound3<T>(
	// 		Point3<T>(std::max(b.minPoint.x, b2.minPoint.x),
	// 				std::max(b.minPoint.y, b2.minPoint.y),
	// 				std::max(b.minPoint.z, b2.minPoint.z)),
	// 		Point3<T>(std::min(b.maxPoint.x, b2.maxPoint.x),
	// 				std::min(b.maxPoint.y, b2.maxPoint.y),
	// 				std::min(b.maxPoint.z, b2.maxPoint.z)));
	//这里要修正一个BUG，不能直接传到构造器中,因为构造器会自动判断两个参数的最大最小分量，并以此构造正确的Bound
	//而我们这里需要"非法"的Bound
	//具体细节查看Pbrt的git
	Bound3<T> ret;
	ret.maxPoint=Min(b.maxPoint,b2.maxPoint);
	ret.minPoint=Max(b.minPoint,b2.minPoint);
	return ret;
}

template<typename T>
inline Bound2<T> Intersect(const Bound2<T>& b, const Bound2<T>& b2) {
	// return Bound2<T>(
	// 		Point2<T>(std::max(b.minPoint.x, b2.minPoint.x),
	// 				std::max(b.minPoint.y, b2.minPoint.y)),
	// 		Point2<T>(std::min(b.maxPoint.x, b2.maxPoint.x),
	// 				std::min(b.maxPoint.y, b2.maxPoint.y)));
	//这里要修正一个BUG，不能直接传到构造器中,因为构造器会自动判断两个参数的最大最小分量，并以此构造正确的Bound
	//而我们这里需要"非法"的Bound
	//具体细节查看Pbrt的git
	Bound2<T> ret;
	ret.maxPoint=Min(b.maxPoint,b2.maxPoint);
	ret.minPoint=Max(b.minPoint,b2.minPoint);
	return ret;
}

//判断两个AABB盒是否重叠
template<typename T>
inline bool Overlap(const Bound3<T> &b1, const Bound3<T> &b2) {
	bool x = (b1.maxPoint.x >= b2.minPoint.x)
			&& (b1.minPoint.x <= b2.maxPoint.x);
	bool y = (b1.maxPoint.y >= b2.minPoint.y)
			&& (b1.minPoint.y <= b2.maxPoint.y);
	bool z = (b1.maxPoint.z >= b2.minPoint.z)
			&& (b1.minPoint.z <= b2.maxPoint.z);
	return (x && y && z);
}

template<typename T>
inline bool Overlap(const Bound2<T> &b1, const Bound2<T> &b2){
	bool x = (b1.maxPoint.x >= b2.minPoint.x)
			&& (b1.minPoint.x <= b2.maxPoint.x);
	bool y = (b1.maxPoint.y >= b2.minPoint.y)
			&& (b1.minPoint.y <= b2.maxPoint.y);
	return (x && y);
}

//判断一个点是否在AABB中
template<typename T>
inline bool Inside(const Point3<T>& p, const Bound3<T> &b) {
	bool x = p.x >= b.minPoint.x && p.x <= b.maxPoint.x;
	bool y = p.y >= b.minPoint.y && p.y <= b.maxPoint.y;
	bool z = p.z >= b.minPoint.z && p.z <= b.maxPoint.z;
	return (x && y && z);
}

//判断一个点是否在AABB中,不包括上边界
template<typename T>
inline bool InsideExclusive(const Point3<T>& p, const Bound3<T> &b) {
	bool x = p.x >= b.minPoint.x && p.x < b.maxPoint.x;
	bool y = p.y >= b.minPoint.y && p.x < b.maxPoint.y;
	bool z = p.z >= b.minPoint.z && p.z < b.maxPoint.z;
	return (x && y && z);
}

//判断一个点是否在RECT中
template<typename T>
inline bool Inside(const Point2<T>& p, const Bound2<T> &b) {
	bool x = p.x >= b.minPoint.x && p.x <= b.maxPoint.x;
	bool y = p.y >= b.minPoint.y && p.y <= b.maxPoint.y;
	return (x && y);
}

//判断一个点是否在RECT中,不包括上边界
template<typename T>
inline bool InsideExclusive(const Point2<T>& p, const Bound2<T> &b) {
	bool x = p.x >= b.minPoint.x && p.x < b.maxPoint.x;
	bool y = p.y >= b.minPoint.y && p.y < b.maxPoint.y;
	return (x && y);
}

//扩充AABB,各个维都扩充delta分量
template<typename T>
inline Bound3<T> Expand(const Bound3<T>& b, T delta) {
	Bound3<T> result;
	result.minPoint = b.minPoint + Vector3<T>(-delta, -delta, -delta);
	result.maxPoint = b.maxPoint + Vector3<T>(delta, delta, delta);
	return result;
}

//基础射线
class Ray {
public:
	Point3f o;	//射线原点
	Vector3f d;	//射线的方向
	mutable Float tMax; //最大参数值
	Float time; //曝光时间相关
	const Medium* medium;//射线所在的介质
public:
	Ray(const Point3f& oo, const Vector3f& dd, Float tmax = Infinity, Float t =
			0.0,const Medium* medium=nullptr) :
			o(oo), d(dd), tMax(tmax), time(t), medium(medium){
		Assert(!HasNaNs());
	}
	Ray() :
			tMax(Infinity), time(0), medium(nullptr){
	}
	Point3f operator()(Float t) const {
		return o + d * t;
	}

	//判断射线是否包含NaN变量
	bool HasNaNs() const {
		return o.HasNaNs() || d.HasNaNs() || IsNaN(time) || IsNaN(tMax);
	}

	//重构ostream方法
	friend std::ostream &operator<<(std::ostream &os, const Ray &r) {
		os << "< o:" << r.o << " ,d:" << r.d << " ,tMax:" << r.tMax << " ,time:"
				<< r.time << ">";
		return os;
	}
};

class RayDifferential: public Ray {
public:
	bool hasDifferential;	//判断是否包含微分信息
	Point3f ox, oy;
	Vector3f dx, dy;
public:
	RayDifferential() :
			Ray() {
		hasDifferential = false;	//默认没有微分信息
	}
	RayDifferential(const Point3f& oo, const Vector3f& dd,
			Float tmax = Infinity, Float t = 0.0, const Medium* medium = nullptr) :
			Ray(oo, dd, tmax, t, medium) {
		Assert(!HasNaNs());
		hasDifferential = false;
	}
	RayDifferential(const Ray& r) :
			Ray(r) {
		Assert(!r.HasNaNs());
		hasDifferential = false;
	}

	//判断射线是否包含NaN变量
	bool HasNaNs() const {
		return Ray::HasNaNs() || ox.HasNaNs() || oy.HasNaNs() || dx.HasNaNs()
				|| dy.HasNaNs();
	}

	//缩放微分信息，默认的差分是一个像素的信息(1 dx ==1 pixel)
	void ScaleRayDifferential(Float s) {
		ox = o + (ox - o) * s;
		oy = o + (oy - o) * s;
		dx = d + (dx - d) * s;
		dy = d + (dy - d) * s;
	}

	//重构ostream方法
	friend std::ostream &operator<<(std::ostream &os,
			const RayDifferential &r) {
		os << "< o:" << r.o << " ,d:" << r.d << " ,tMax:" << r.tMax << " ,time:"
				<< r.time << " ,hasDifferential:" << r.hasDifferential
				<< " ,ox:" << r.ox << " ,dx:" << r.dx << " ,oy:" << r.oy
				<< " ,dy:" << r.dy << ">";
		return os;
	}
};

//Dot运算
template<typename T>
inline Float Dot(const Vector3<T>& v1, const Vector3<T>& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
template<typename T>
inline Float Dot(const Vector3<T>& v1, const Normal3<T>& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
template<typename T>
inline Float Dot(const Normal3<T>& v1, const Vector3<T>& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
template<typename T>
inline Float Dot(const Normal3<T>& v1, const Normal3<T>& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
template<typename T>
inline Float Dot(const Vector2<T>& v1, const Vector2<T>& v2) {
	return v1.x * v2.x + v1.y * v2.y;
}
template<typename T>
inline Float AbsDot(const Vector3<T>& v1, const Vector3<T>& v2) {
	return std::abs(Dot(v1, v2));
}
template<typename T>
inline Float AbsDot(const Normal3<T>& v1, const Vector3<T>& v2) {
	return std::abs(Dot(v1, v2));
}
template<typename T>
inline Float AbsDot(const Vector3<T>& v1, const Normal3<T>& v2) {
	return std::abs(Dot(v1, v2));
}
template<typename T>
inline Float AbsDot(const Normal3<T>& v1, const Normal3<T>& v2) {
	return std::abs(Dot(v1, v2));
}
template<typename T>
inline Float AbsDot(const Vector2<T>& v1, const Vector2<T>& v2) {
	return std::abs(Dot(v1, v2));
}
//叉乘
//基于左手坐标系
template<typename T>
inline Vector3<T> Cross(const Vector3<T>& v1, const Vector3<T>& v2) {
	return Vector3<T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x);
}
template<typename T>
inline Vector3<T> Cross(const Normal3<T>& v1, const Vector3<T>& v2) {
	return Vector3<T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x);
}
template<typename T>
inline Vector3<T> Cross(const Normal3<T>& v1, const Normal3<T>& v2) {
	return Vector3<T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x);
}
template<typename T>
inline Vector3<T> Cross(const Vector3<T>& v1, const Normal3<T>& v2) {
	return Vector3<T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x);
}

//标准化
template<typename T>
inline Vector3<T> Normalize(const Vector3<T>& v) {
	return v / v.Length();
}
template<typename T>
inline Vector2<T> Normalize(const Vector2<T>& v) {
	return v / v.Length();
}
template<typename T>
inline Normal3<T> Normalize(const Normal3<T>& n) {
	Assert(!n.HasNaNs());
	return n / n.Length();
}

//标量乘以向量的操作，其实就是换个位置，使用向量乘以标量的方式
template<typename T, typename U>
inline Vector3<T> operator*(U n, const Vector3<T>& v) {
	return v * n;
}

template<typename T, typename U>
inline Normal3<T> operator*(U n, const Normal3<T>& v) {
	return v * n;
}

//把第一个参数转换到和第二个参数相同的空间半球中
//4系列
template<typename T>
inline Normal3<T> Faceforward(const Normal3<T> &n, const Vector3<T> &v) {
	return (Dot(n, v) < 0.0) ? -n : n;
}

template<typename T>
inline Normal3<T> Faceforward(const Normal3<T> &n, const Normal3<T> &v) {
	return (Dot(n, v) < 0.0) ? -n : n;
}

template<typename T>
inline Vector3<T> Faceforward(const Vector3<T> &n, const Vector3<T> &v) {
	return (Dot(n, v) < 0.0) ? -n : n;
}

template<typename T>
inline Vector3<T> Faceforward(const Vector3<T> &n, const Normal3<T> &v) {
	return (Dot(n, v) < 0.0) ? -n : n;
}

//距离相关
template<typename T>
inline Float DistanceSquared(const Point3<T>& p1, const Point3<T>& p2) {
	Assert(!p1.HasNaNs() && !p2.HasNaNs());
	return (p1 - p2).LengthSquared();
}
template<typename T>
inline Float Distance(const Point3<T>& p1, const Point3<T>& p2) {
	return std::sqrt(DistanceSquared(p1, p2));
}
template<typename T>
inline Float DistanceSquared(const Point2<T>& p1, const Point2<T>& p2) {
	Assert(!p1.HasNaNs() && !p2.HasNaNs());
	return (p1 - p2).LengthSquared();
}
template<typename T>
inline Float Distance(const Point2<T>& p1, const Point2<T>& p2) {
	return std::sqrt(DistanceSquared(p1, p2));
}

//根据一个向量V(x,y,z)生成一个新的坐标系
//1.首先生成一个和V正交的向量VT(-z,0,x),并且标准化
//2.叉乘生成第三个变量VB
//3.为了保证当x,z都为0的时候发生错误，所以这里判断当x>y:VT(-z,0,x)不然VT(0,-z,y)
template<typename T>
inline void CoordinateSystem(const Vector3<T>& V, Vector3<T>* VT,
		Vector3<T>* VB) {
	if (std::abs(V.x) > std::abs(V.y)) {
		Float reciprocal = 1.0 / std::sqrt(V.x * V.x + V.z * V.z);	//用来标准化的参数
		(*VT) = Vector3<T>(-V.z * reciprocal, 0, V.x * reciprocal);
	} else {
		Float reciprocal = 1.0 / std::sqrt(V.y * V.y + V.z * V.z);	//用来标准化的参数
		(*VT) = Vector3<T>(0, -V.z * reciprocal, V.y * reciprocal);
	}
	(*VB) = Cross(V, *VT);
}

//球坐标到向量的变换
//theta是和Z轴之间的角度
//phi是从x轴往y轴旋转的角度
//1.costheta就是Z坐标
//2.1-Z=x+y=>x+y=sintheta
//3.不考虑Z轴的情况下,x==cosphi,y==sinphi

inline Vector3f SphericalDirection(Float sintheta, Float costheta, Float phi) {
	return Vector3f(sintheta * std::cos(phi), sintheta * std::sin(phi),
			costheta);
}
//任意坐标系下的球面坐标转向量
inline Vector3f SphericalDirection(Float sinTheta, Float cosTheta, Float phi,
		const Vector3f &x, const Vector3f &y, const Vector3f &z) {
	return sinTheta * std::cos(phi) * x + sinTheta * std::sin(phi) * y
			+ cosTheta * z;
}

//通过向量返回球面坐标theta
inline Float SphericalTheta(const Vector3f& v) {
	Float z = Clamp(v.z, -1, 1);
	return std::acos(z);
}

//通过向量返回球面坐标phi
inline Float SphericalPhi(const Vector3f &v) {
	Float p = std::atan2(v.y, v.x);	//因为phi是0~2PI 所以不能直接考虑cosphi
	return (p < 0) ? (p + 2 * Pi) : p;
}

//返回Vector3<T>的绝对值
template<typename T>
inline Vector3<T> Abs(const Vector3<T>& v) {
	return Vector3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

//返回Vector3<T>的绝对值
template<typename T>
inline Normal3<T> Abs(const Normal3<T>& v) {
	return Normal3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

//取大于原始值最小的整数
template<typename T>
inline Vector2<T> Ceil(const Vector2<T>& p) {
	return Vector2<T>(std::ceil(p.x), std::ceil(p.y));
}
template<typename T>
inline Point2<T> Ceil(const Point2<T>& p) {
	return Point2<T>(std::ceil(p.x), std::ceil(p.y));
}
template<typename T>
inline Vector3<T> Ceil(const Vector3<T>& p) {
	return Vector3<T>(std::ceil(p.x), std::ceil(p.y), std::ceil(p.z));
}
template<typename T>
inline Point3<T> Ceil(const Point3<T>& p) {
	return Point3<T>(std::ceil(p.x), std::ceil(p.y), std::ceil(p.z));
}

//取小于原始值最大的整数
template<typename T>
inline Vector2<T> Floor(const Vector2<T>& p) {
	return Vector2<T>(std::floor(p.x), std::floor(p.y));
}
template<typename T>
inline Point2<T> Floor(const Point2<T>& p) {
	return Point2<T>(std::floor(p.x), std::floor(p.y));
}
template<typename T>
inline Vector3<T> Floor(const Vector3<T>& p) {
	return Vector3<T>(std::floor(p.x), std::floor(p.y), std::floor(p.z));
}
template<typename T>
inline Point3<T> Floor(const Point3<T>& p) {
	return Point3<T>(std::floor(p.x), std::floor(p.y), std::floor(p.z));
}

//Max and Min
template<typename T>
inline Point3<T> Max(const Point3<T>& p1, const Point3<T>& p2) {
	return Point3<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
			std::max(p1.z, p2.z));
}
template<typename T>
inline Point2<T> Max(const Point2<T>& p1, const Point2<T>& p2) {
	return Point2<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
}
template<typename T>
inline Vector3<T> Max(const Vector3<T>& p1, const Vector3<T>& p2) {
	return Vector3<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
			std::max(p1.z, p2.z));
}
template<typename T>
inline Vector2<T> Max(const Vector2<T>& p1, const Vector2<T>& p2) {
	return Vector2<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
}
template<typename T>
inline Normal3<T> Max(const Normal3<T>& p1, const Normal3<T>& p2) {
	return Normal3<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
			std::max(p1.z, p2.z));
}

template<typename T>
inline Point3<T> Min(const Point3<T>& p1, const Point3<T>& p2) {
	return Point3<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
			std::min(p1.z, p2.z));
}
template<typename T>
inline Point2<T> Min(const Point2<T>& p1, const Point2<T>& p2) {
	return Point2<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
}
template<typename T>
inline Vector3<T> Min(const Vector3<T>& p1, const Vector3<T>& p2) {
	return Vector3<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
			std::min(p1.z, p2.z));
}
template<typename T>
inline Vector2<T> Min(const Vector2<T>& p1, const Vector2<T>& p2) {
	return Vector2<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
}
template<typename T>
inline Normal3<T> Min(const Normal3<T>& p1, const Normal3<T>& p2) {
	return Normal3<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
			std::min(p1.z, p2.z));
}

//获取最大的维度
template<typename T>
int MaxDimension(const Vector3<T>& v) {
	if (v.x > v.y) {
		if (v.x > v.z) {
			return 0;
		} else {
			return 2;
		}
	} else {
		if (v.y > v.z) {
			return 1;
		} else {
			return 2;
		}
	}
}

//重新排列
template<typename T>
Vector3<T> Permute(const Vector3<T>& v, int xi, int yi, int zi) {
	return Vector3<T>(v[xi], v[yi], v[zi]);
}

template<typename T>
Point3<T> Permute(const Point3<T>& v, int xi, int yi, int zi) {
	return Point3<T>(v[xi], v[yi], v[zi]);
}

//获取最大组件
template<typename T>
T MaxComponent(const Vector3<T> &v) {
	return std::max(v.x, std::max(v.y, v.z));
}

inline Point3f OffsetRayOrigin(const Point3f& p/*需要被偏移的原点*/,
		const Vector3f& pErr/*误差*/, const Normal3f& n/*原点所在表面法线*/,
		const Vector3f& w/*射线的方向*/) {
	//1.首先计算原点所在切平面需要偏移的量
	Float d = Dot(Abs(n), pErr);
#ifdef FLOAT_AS_DOUBLE
	//PBRT的解释如下:
	// We have tons of precision; for now bump up the offset a bunch just
	// to be extra sure that we start on the right side of the surface
	// (In case of any bugs in the epsilons code...)
	d *= 1024;
#endif
	//先判断射线是向外还是向内
	if (Dot(w, n) < 0) {
		d = -d;
	}
	Vector3f offset = d * Vector3f(n);

	Point3f po = p + offset;
	//再做保险的误差边界设置
	for (int i = 0; i < 3; ++i) {
		if (offset[i] > 0) {
			po[i] = NextFloatUp(po[i]);
		} else if (offset[i] < 0) {
			po[i] = NextFloatDown(po[i]);
		}
	}
	return po;
}

template<typename T>
inline bool Bound3<T>::IntersectP(const Ray& ray,Float* tHit1,Float *tHit2) const{
	//通过和3个夹板进行求交,求参数
	Float t0=0;
	Float t1=ray.tMax;
	for(int i=0;i<3;++i){
		//通过简化求交方程式，可以得到t=(x-ox)/dx
		Float reciprocalD=1.0/ray.d[i];
		Float tNear=(minPoint[i]-ray.o[i])*reciprocalD;
		Float tFar=(maxPoint[i]-ray.o[i])*reciprocalD;
		if(tNear>tFar){
			std::swap(tNear,tFar);
		}

		//进行浮点数误差处理
		//保证tFar在浮点数误差范围内呈现最大值
		tFar = tFar*(1 + 2 * gamma(3));

		if(t0<tNear){
			t0=tNear;
		}
		if(t1>tFar){
			t1=tFar;
		}
		//判断参数是否还合理，不合理说明相交失败
		if(t0>t1){
			return false;
		}
	}

	if(tHit1){
		*tHit1=t0;
	}
	if(tHit2){
		*tHit2=t1;
	}
	return true;
}


template<typename T>
inline bool Bound3<T>::IntersectP(const Ray& ray,const Vector3f& reciprocalDir,const int isNeg[3]) const {
	const Bound3<T>& b = *this;
	//求与yz平面的交点参数
	Float tMinX= (b[isNeg[0]].x - ray.o.x)*reciprocalDir.x;
	Float tMaxX= (b[1-isNeg[0]].x - ray.o.x)*reciprocalDir.x;
	//求与xz平面的交点参数
	Float tMinY = (b[isNeg[1]].y - ray.o.y)*reciprocalDir.y;
	Float tMaxY = (b[1 - isNeg[1]].y - ray.o.y)*reciprocalDir.y;

	//误差
	tMaxX = tMaxX*(1 + 2 * gamma(3));
	tMaxY = tMaxY*(1 + 2 * gamma(3));

	if (tMinX > tMaxY || tMinY > tMaxX) {
		return false;
	}

	if (tMinY > tMinX) {
		tMinX = tMinY;
	}
	if (tMaxY < tMaxX) {
		tMaxX = tMaxY;
	}

	//求与xy平面的交点参数
	Float tMinZ = (b[isNeg[2]].z - ray.o.z)*reciprocalDir.z;
	Float tMaxZ = (b[1 - isNeg[2]].z - ray.o.z)*reciprocalDir.z;

	//误差
	tMaxZ = tMaxZ*(1 + 2 * gamma(3));
	if (tMinX > tMaxZ || tMinZ > tMaxX) {
		return false;
	}

	if (tMinZ > tMinX) {
		tMinX = tMinZ;
	}
	if (tMaxZ < tMaxX) {
		tMaxX = tMaxZ;
	}

	//因为没有返回两个t,所以需要判断是否在射线的有效范围内
	return (tMinX < ray.tMax) && (tMaxX > 0);
}

#endif /* SRC_CORE_GEOMETRY_H_ */

#ifndef SRC_CORE_QUATERNION_H_
#define SRC_CORE_QUATERNION_H_
#pragma once

#include "raiden.h"
#include "geometry.h"

//4元数
class Quaternion{
public:
	Vector3f v;
	Float    w;
public:
	Quaternion(Float i, Float j, Float k, Float ww):v(i,j,k),w(ww){

	}

	Quaternion(const Vector3f& vv, Float ww) :v(vv), w(ww) {

	}

	inline Quaternion& operator+=(const Quaternion& q) {
		v += q.v;
		w += q.w;
		return (*this);
	}

	inline Quaternion operator+(const Quaternion& q) const{
		Quaternion p=(*this);
		p+=q;
		return p;
	}

	inline Quaternion& operator-=(const Quaternion& q) {
		v -= q.v;
		w -= q.w;
		return (*this);
	}

	inline Quaternion operator-( const Quaternion& q) const{
		Quaternion p=(*this);
		p-=q;
		return p;
	}

	inline Quaternion& operator*=(Float s) {
		v *= s;
		w *= s;
		return (*this);
	}

	inline Quaternion operator*(Float s) const{
		Quaternion p=(*this);
		p*=s;
		return p;
	}

	inline Quaternion& operator/=(Float div) {
		Assert(div!=0);
		Float reciprocal = 1.0 / div; 
		v *= reciprocal;
		w *= reciprocal;
		return (*this);
	}
	inline Quaternion operator/(Float div) const{
		Quaternion p=(*this);
		p/=div;
		return p;
	}

	inline Quaternion operator-() const{
		Quaternion p=(*this);
		p.v=-p.v;
		p.w=-p.w;
		return p;
	}

	inline bool operator==(const Quaternion& q ) const{
		return (v==q.v)&&(w==q.w);
	}
	inline bool operator!=(const Quaternion& q ) const{
		return (v!=q.v)||(w!=q.w);
	}

	//重构ostream方法
	inline friend std::ostream &operator<<(std::ostream &os, const Quaternion &q) {
		os << "[ " << q.v.x << " , " << q.v.y << " , " << q.v.z <<" , " <<q.w<<" ]";
		return os;
	}
};

inline Quaternion operator*(Float s,const Quaternion& q){
	return q*s;
}

//点乘
inline Float Dot(const Quaternion& p,const Quaternion& q){
	return Dot(p.v,q.v)+(p.w*q.w);
}

inline Quaternion Normalize(const Quaternion& p){
	return p/std::sqrt(Dot(p,p));
}

#endif//SRC_CORE_QUATERNION_H_
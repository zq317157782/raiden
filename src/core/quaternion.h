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
	}

	inline friend  Quaternion operator+(const Quaternion& p, const Quaternion& q) {
		return Quaternion(p.v + q.v, p.w + q.w);
	}

	inline Quaternion& operator-=(const Quaternion& q) {
		v -= q.v;
		w -= q.w;
	}

	inline friend  Quaternion operator-(const Quaternion& p, const Quaternion& q) {
		return Quaternion(p.v - q.v, p.w - q.w);
	}

};


#endif//SRC_CORE_QUATERNION_H_
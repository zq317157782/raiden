#ifndef SRC_CORE_QUATERNION_H_
#define SRC_CORE_QUATERNION_H_
#pragma once

#include "raiden.h"
#include "geometry.h"
#include "transform.h"
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

	//从四元数到Transform的转换
	Transform ToTransform() const{
		Float m[4][4];
		
		m[0][0] = 1 - 2 * (v.y*v.y+ v.z*v.z);
		m[0][1] = 2 * (v.x*v.y + v.z*w);
		m[0][2] = 2 * (v.x*v.z - v.y*w);
		m[0][3] = 0;

		m[1][0] = 2 * (v.x*v.y - v.z*w);
		m[1][1] = 1 - 2 * (v.x*v.x + v.z*v.z);
		m[1][2] = 2 * (v.y*v.z + v.x*w);
		m[1][3] = 0;

		m[2][0] = 2 * (v.x*v.z + v.y*w);
		m[2][1] = 2 * (v.y*v.z + v.x*w);
		m[2][2] = 1 - 2 * (v.x*v.x + v.y*v.y);
		m[2][3] = 1;

		m[3][0] = 0;
		m[3][1] = 0;
		m[3][2] = 0;
		m[3][3] = 1;

		Transform tran(m);
		return m;

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

//Spherical Linear Interpolate
Quaternion Slerp(Float t,const Quaternion& q1,const Quaternion& q2){
	//1.首先求两个四元数之间的cos值
	Float cosTheta=Clamp(Dot(q1,q2),-1,1);//保证数值在[-1~1]之间
	//2.如果两个四元数之间几乎平行，则采用线性插值
	if(cosTheta>0.9995f){
		return (1-t)*q1+t*q2;
	}else{
		//3.不然得话，使用Slerp
		Float theta=std::acos(cosTheta);
		theta=t*theta;
		Quaternion qperp=Normalize(q2-cosTheta*q1);//求q2的垂直分量向量
		return q1*std::cos(theta)+qperp*std::sin(theta);
	}
}
#endif//SRC_CORE_QUATERNION_H_
#pragma once
#include "raiden.h"
#include "geometry.h"

//Cubic Bezier曲线
class CubicBezier{
public:
	Point3f p[4];
public:
	CubicBezier(const Point3f& p0,const Point3f& p1,const Point3f& p2,const Point3f& p3){
		p[0]=p0;
		p[1]=p1;
		p[2]=p2;
		p[3]=p3;
	}
	
	CubicBezier(const Point3f cps[4]){
		p[0]=cps[0];
		p[1]=cps[1];
		p[2]=cps[2];
		p[3]=cps[3];
	}
};


//Cubic Bezier曲线的Blossom操作
inline Point3f Blossom(const CubicBezier& bezier,Float u0,Float u1,Float u2){
	Point3f a0=Lerp(u0,bezier.p[0],bezier.p[1]);
	Point3f a1=Lerp(u0,bezier.p[1],bezier.p[2]);
	Point3f a2=Lerp(u0,bezier.p[2],bezier.p[3]);

	Point3f b0=Lerp(u1,a0,a1);
	Point3f b1=Lerp(u1,a1,a2);

	Point3f c=Lerp(u2,b0,b1);
	return c;
}
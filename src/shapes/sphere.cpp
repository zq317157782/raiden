/*
 * sphere.cpp
 *
 *  Created on: 2016年11月23日
 *      Author: zhuqian
 */
#include "sphere.h"
#include "errfloat.h"
#include "paramset.h"
Bound3f Sphere::ObjectBound() const {
	return Bound3f(Point3f(-_radius, -_radius, _zMin),
			Point3f(_radius, _radius, _zMax));
}

//这里使用的是微积分内的一个标准的求绕某轴旋转的曲面的面积公式
Float Sphere::Area() const {
	return _phiMax * _radius * (_zMax - _zMin);
}

bool Sphere::Intersect(const Ray& ray, Float* tHit,
		SurfaceInteraction* surfaceIsect, bool testAlpha) const {
	Vector3f oErr, dErr;
	Ray oRay = (*worldToObject)(ray, &oErr, &dErr); //把射线转换到球体局部空间,并且计算转换过程中的浮点数误差
	//带误差的Ray原点
	ErrFloat ox(oRay.o.x, oErr.x);
	ErrFloat oy(oRay.o.y, oErr.y);
	ErrFloat oz(oRay.o.z, oErr.z);
	//带误差的Ray方向
	ErrFloat dx(oRay.d.x, dErr.x);
	ErrFloat dy(oRay.d.y, dErr.y);
	ErrFloat dz(oRay.d.z, dErr.z);
	//通过把射线的参数方程代入到球体的隐式解析方程中，得到二次多项式的A,B,C参数
	ErrFloat A = dx * dx + dy * dy + dz * dz;
	ErrFloat B = 2 * (dx * ox + dy * oy + dz * oz);
	ErrFloat C = ox * ox + oy * oy + oz * oz
			- ErrFloat(_radius) * ErrFloat(_radius);
	//求解二次多项式
	ErrFloat t0, t1;
	if (!Quadratic(A, B, C, &t0, &t1)) {
		//无法求解的话，说明没有相交,直接返回
		return false;
	}
	//tMax和t0,t1的比较，最保守的比较
	if (oRay.tMax < t0.UpperBound() || 0 >= t1.LowerBound()) {
		return false;
	}

	//开始比较是取t0还是t1
	ErrFloat tShapeHit = t0;

	if (tShapeHit.LowerBound() <= 0) {	//误差边界已经小于等于0了，不能用
		tShapeHit = t1;
		if (tShapeHit.UpperBound() > oRay.tMax)
			return false;	//两个变量都不满足射线的有效范围，返回fasle
	}

	Point3f pHit = oRay((Float) tShapeHit);	//获得相交点空间位置数据
	pHit = pHit * (_radius / Distance(pHit, Point3f(0, 0, 0)));	//重新refine 缩小ulp
	if (pHit.x == 0 && pHit.y == 0) {
		pHit.x = 1e-5f * _radius;	//防止0/0产生NaN这样悲催的事情发生
	}
	Float phi = std::atan2(pHit.y, pHit.x);
	if (phi < 0)
		phi += 2 * Pi;	//保证phi在0~2Pi范围之内

	//下面是比较交点和球体参数之间的关系
	if ((_zMin > -_radius && _zMin > pHit.z)
			|| (_zMax < _radius && _zMax < pHit.z) || (phi > _phiMax)) {
		if (tShapeHit == t1) {
			//如果已经是比较远的那个点，就不需要比较了
			return false;
		}
		//走到这说明是t0,比较t1的合法性
		if (t1.UpperBound() > oRay.tMax) {
			return false;
		}
		tShapeHit = t1;	//更新为t1
		pHit = oRay((Float) tShapeHit);	//重新计算空间交点
		pHit = pHit * (_radius / Distance(pHit, Point3f(0, 0, 0)));
		//重新计算参数
		if (pHit.x == 0 && pHit.y == 0) {
			pHit.x = 1e-5f * _radius;	//防止0/0产生NaN这样悲催的事情发生
		}
		phi = std::atan2(pHit.y, pHit.x);
		if (phi < 0)
			phi += 2 * Pi;	//保证phi在0~2Pi范围之内
		//然后以t1的身份再比较一次
		if ((_zMin > -_radius && _zMin > pHit.z)
				|| (_zMax < _radius && _zMax < pHit.z) || (phi > _phiMax)) {
			return false;		//失败
		}
	}
	//开始计算球体的参数表达式
	Float u = (phi / _phiMax);
	Float theta = std::acos(Clamp(pHit.z / _radius, -1, 1));
	Float v = (theta - _thetaMin) / (_thetaMax - _thetaMin);//这里也可以(_thetaMax-theta)/(_thetaMax-_thetaMin),其实方向反了
	Vector3f dpdu(-_phiMax * pHit.y, _phiMax * pHit.x, 0);
	//三角函数关系计算cosPhi和sinPhi
	Float phiEdge = std::sqrt(pHit.x * pHit.x + pHit.y * pHit.y);
	Float invPhiEdge = 1.0f / phiEdge;
	Float cosPhi = pHit.x * invPhiEdge;
	Float sinPhi = pHit.y * invPhiEdge;
	Vector3f dpdv = Vector3f(pHit.z * cosPhi, pHit.z * sinPhi,
			-_radius * std::sin(theta)) * (_thetaMax - _thetaMin);

	//以下代码计算法线的偏导，公式来自PBRT以及RayDiffereinal论文，我还没有仔细研究为啥是这样子
	Vector3f d2Pduu = -_phiMax * _phiMax * Vector3f(pHit.x, pHit.y, 0);
	Vector3f d2Pduv = (_thetaMax - _thetaMin) * pHit.z * _phiMax
			* Vector3f(-sinPhi, cosPhi, 0.);
	Vector3f d2Pdvv = -(_thetaMax - _thetaMin) * (_thetaMax - _thetaMin)
			* Vector3f(pHit.x, pHit.y, pHit.z);
	Float E = Dot(dpdu, dpdu);
	Float F = Dot(dpdu, dpdv);
	Float G = Dot(dpdv, dpdv);
	Vector3f N = Normalize(Cross(dpdu, dpdv));
	Float e = Dot(N, d2Pduu);
	Float f = Dot(N, d2Pduv);
	Float g = Dot(N, d2Pdvv);
	Float invEGF2 = 1 / (E * G - F * F);
	Normal3f dndu = Normal3f(
			(f * F - e * G) * invEGF2 * dpdu
					+ (e * F - f * E) * invEGF2 * dpdv);
	Normal3f dndv = Normal3f(
			(g * F - f * G) * invEGF2 * dpdu
					+ (f * F - g * E) * invEGF2 * dpdv);
	//Refine交点产生的绝对误差
	Vector3f pError = gamma(5) * Abs((Vector3f)pHit);
	*surfaceIsect=(*objectToWorld)(SurfaceInteraction(pHit,pError,Point2f(u,v),-oRay.d,dpdu,dpdv,dndu,dndv,oRay.time,this));
	//这里的tShapeHit并没有考虑refine,所以误差可能有点高
	 *tHit = (Float)tShapeHit;
	return true;
}

bool Sphere::IntersectP(const Ray& ray, bool testAlpha) const {
	Vector3f oErr, dErr;
	Ray oRay = (*worldToObject)(ray, &oErr, &dErr); //把射线转换到球体局部空间,并且计算转换过程中的浮点数误差
	//带误差的Ray原点
	ErrFloat ox(oRay.o.x, oErr.x);
	ErrFloat oy(oRay.o.y, oErr.y);
	ErrFloat oz(oRay.o.z, oErr.z);
	//带误差的Ray方向
	ErrFloat dx(oRay.d.x, dErr.x);
	ErrFloat dy(oRay.d.y, dErr.y);
	ErrFloat dz(oRay.d.z, dErr.z);
	//通过把射线的参数方程代入到球体的隐式解析方程中，得到二次多项式的A,B,C参数
	ErrFloat A = dx * dx + dy * dy + dz * dz;
	ErrFloat B = 2 * (dx * ox + dy * oy + dz * oz);
	ErrFloat C = ox * ox + oy * oy + oz * oz
			- ErrFloat(_radius) * ErrFloat(_radius);
	//求解二次多项式
	ErrFloat t0, t1;
	if (!Quadratic(A, B, C, &t0, &t1)) {
		//无法求解的话，说明没有相交,直接返回
		return false;
	}
	//tMax和t0,t1的比较，最保守的比较
	if (oRay.tMax < t0.UpperBound() || 0 >= t1.LowerBound()) {
		return false;
	}
	//开始比较是取t0还是t1
	ErrFloat tShapeHit = t0;

	if (tShapeHit.LowerBound() <= 0) {	//误差边界已经小于等于0了，不能用
		tShapeHit = t1;
		if (tShapeHit.UpperBound() > oRay.tMax)
			return false;	//两个变量都不满足射线的有效范围，返回fasle
	}
	Point3f pHit = oRay((Float) tShapeHit);	//获得相交点空间位置数据
	pHit = pHit * (_radius / Distance(pHit, Point3f(0, 0, 0)));	//refine?为啥?这么保守?
	if (pHit.x == 0 && pHit.y == 0) {
		pHit.x = 1e-5f * _radius;	//防止0/0产生NaN这样悲催的事情发生
	}
	Float phi = std::atan2(pHit.y, pHit.x);
	if (phi < 0)
		phi += 2 * Pi;	//保证phi在0~2Pi范围之内

	//下面是比较交点和球体参数之间的关系
	if ((_zMin > -_radius && _zMin > pHit.z)
			|| (_zMax < _radius && _zMax < pHit.z) || (phi > _phiMax)) {
		if (tShapeHit == t1) {
			//如果已经是比较远的那个点，就不需要比较了
			return false;
		}
		//走到这说明是t0,比较t1的合法性
		if (t1.UpperBound() > oRay.tMax) {
			return false;
		}
		tShapeHit = t1;	//更新为t1
		pHit = oRay((Float) tShapeHit);	//重新计算空间交点
		pHit = pHit * (_radius / Distance(pHit, Point3f(0, 0, 0)));
		//重新计算参数
		if (pHit.x == 0 && pHit.y == 0) {
			pHit.x = 1e-5f * _radius;	//防止0/0产生NaN这样悲催的事情发生
		}
		phi = std::atan2(pHit.y, pHit.x);
		if (phi < 0)
			phi += 2 * Pi;	//保证phi在0~2Pi范围之内
		//然后以t1的身份再比较一次
		if ((_zMin > -_radius && _zMin > pHit.z)
				|| (_zMax < _radius && _zMax < pHit.z) || (phi > _phiMax)) {
			return false;			//失败
		}
	}
	return true;
}


std::shared_ptr<Shape> CreateSphereShape(const Transform *o2w,
                                         const Transform *w2o,
                                         bool reverseOrientation,
                                         const ParamSet &params) {
    Float radius = params.FindOneFloat("radius", 1.0f);
    Float zmin = params.FindOneFloat("zmin", -radius);
    Float zmax = params.FindOneFloat("zmax", radius);
    Float phimax = params.FindOneFloat("phimax", 360.0f);
    Debug("[make sphere ,radius:"<<radius<<" ,zmin:"<<zmin<<" ,zmax:" <<zmax<<" ,phimax:"<<phimax<<"]=>");
	Debug("=>[ o2w:" << *o2w << "]");
    return std::make_shared<Sphere>(o2w, w2o, reverseOrientation, radius, zmin,
                                    zmax, phimax);
}

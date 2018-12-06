/*
 * sphere.cpp
 *
 *  Created on: 2016年11月23日
 *      Author: zhuqian
 */
#include "sphere.h"
#include "errfloat.h"
#include "paramset.h"
#include "sampling.h"
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
	EFloat ox(oRay.o.x, oErr.x);
	EFloat oy(oRay.o.y, oErr.y);
	EFloat oz(oRay.o.z, oErr.z);
	//带误差的Ray方向
	EFloat dx(oRay.d.x, dErr.x);
	EFloat dy(oRay.d.y, dErr.y);
	EFloat dz(oRay.d.z, dErr.z);
	//通过把射线的参数方程代入到球体的隐式解析方程中，得到二次多项式的A,B,C参数
	EFloat A = dx * dx + dy * dy + dz * dz;
	EFloat B = 2 * (dx * ox + dy * oy + dz * oz);
	EFloat C = ox * ox + oy * oy + oz * oz
			- EFloat(_radius) * EFloat(_radius);
	//求解二次多项式
	EFloat t0, t1;
	if (!Quadratic(A, B, C, &t0, &t1)) {
		//无法求解的话，说明没有相交,直接返回
		return false;
	}
	//tMax和t0,t1的比较，最保守的比较
	if (oRay.tMax < t0.UpperBound() || 0 >= t1.LowerBound()) {
		return false;
	}

	//开始比较是取t0还是t1
	EFloat tShapeHit = t0;

	if (tShapeHit.LowerBound() <= 0) {	//误差边界已经小于等于0了，不能用
		tShapeHit = t1;
		if (tShapeHit.UpperBound() > oRay.tMax){
			//射线在t0和t1之间，意味着整个射线都在圆内
			return false;	//两个变量都不满足射线的有效范围，返回fasle
		}
			
	}

	Point3f pHit = oRay((Float) tShapeHit);	//获得相交点空间位置数据
	pHit = pHit * (_radius / Distance(pHit, Point3f(0, 0, 0)));	//重新refine 缩小ulp
	if (pHit.x == 0 && pHit.y == 0) {
		pHit.x = 1e-5f * _radius;	//防止0/0产生NaN这样悲催的事情发生
	}
	Float phi = std::atan2(pHit.y, pHit.x);
	if (phi < 0){
		phi += 2 * Pi;	//保证phi在0~2Pi范围之内
	}
		

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
		if (phi < 0){
			phi += 2 * Pi;	//保证phi在0~2Pi范围之内
		}
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
	Float invPhiEdge = 1.0 / phiEdge;
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
	EFloat ox(oRay.o.x, oErr.x);
	EFloat oy(oRay.o.y, oErr.y);
	EFloat oz(oRay.o.z, oErr.z);
	//带误差的Ray方向
	EFloat dx(oRay.d.x, dErr.x);
	EFloat dy(oRay.d.y, dErr.y);
	EFloat dz(oRay.d.z, dErr.z);
	//通过把射线的参数方程代入到球体的隐式解析方程中，得到二次多项式的A,B,C参数
	EFloat A = dx * dx + dy * dy + dz * dz;
	EFloat B = 2 * (dx * ox + dy * oy + dz * oz);
	EFloat C = ox * ox + oy * oy + oz * oz
			- EFloat(_radius) * EFloat(_radius);
	//求解二次多项式
	EFloat t0, t1;
	if (!Quadratic(A, B, C, &t0, &t1)) {
		//无法求解的话，说明没有相交,直接返回
		return false;
	}
	//tMax和t0,t1的比较，最保守的比较
	if (oRay.tMax < t0.UpperBound() || 0 >= t1.LowerBound()) {
		return false;
	}
	//开始比较是取t0还是t1
	EFloat tShapeHit = t0;

	if (tShapeHit.LowerBound() <= 0) {	//误差边界已经小于等于0了，不能用
		tShapeHit = t1;
		if (tShapeHit.UpperBound() > oRay.tMax){
			return false;	//两个变量都不满足射线的有效范围，返回fasle
		}
	}
	Point3f pHit = oRay((Float) tShapeHit);	//获得相交点空间位置数据
	pHit = pHit * (_radius / Distance(pHit, Point3f(0, 0, 0)));	//refine?为啥?这么保守?
	if (pHit.x == 0 && pHit.y == 0) {
		pHit.x = 1e-5f * _radius;	//防止0/0产生NaN这样悲催的事情发生
	}
	Float phi = std::atan2(pHit.y, pHit.x);
	if (phi < 0){
		phi += 2 * Pi;	//保证phi在0~2Pi范围之内
	}

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
		if (phi < 0){
			phi += 2 * Pi;	//保证phi在0~2Pi范围之内
		}
		//然后以t1的身份再比较一次
		if ((_zMin > -_radius && _zMin > pHit.z)
				|| (_zMax < _radius && _zMax < pHit.z) || (phi > _phiMax)) {
			return false;			//失败
		}
	}
	return true;
}


Interaction Sphere::Sample(const Point2f& uv, Float *pdf) const {
	Interaction ref;
	//计算得到局部空间下交点
	Point3f pointL = Point3f(0, 0, 0) + _radius*UniformSampleSphere(uv);
	//计算世界坐标下的交点
	ref.n = Normalize((*objectToWorld)(Normal3f(pointL.x, pointL.y, pointL.z)));
	//反转法线
	if (reverseOrientation) {
		ref.n *= -1.0;
	}

	pointL = pointL*(_radius / (Distance(pointL, Point3f(0, 0, 0))));
	//计算误差
	Vector3f pointLErr = gamma(5) * Abs((Vector3f)pointL);
	ref.p = (*objectToWorld)(pointL, pointLErr, &ref.pErr);
	*pdf = 1.0 / Area();
	return ref;
}

Interaction Sphere::Sample(const Interaction& ref, const Point2f& u, Float *pdf) const {
	//获取球心的世界坐标
	Point3f centerW = (*objectToWorld)(Point3f(0, 0, 0));
	Point3f pOrigin = OffsetRayOrigin(ref.p, ref.pErr, ref.n, centerW - ref.p);
	//交点在球体内部的情况
	if (DistanceSquared(pOrigin,centerW) <= _radius*_radius) {
		//采样得到球体相交点
		Interaction inst = Sample(u, pdf);
		//得到指向Light上一点的方向
		Vector3f wi = inst.p - ref.p;
		if (wi.LengthSquared() == 0) {
			*pdf = 0;
		}
		else {
			//转换area measure的pdf到立体角
			wi = Normalize(wi);
			*pdf = (*pdf)*DistanceSquared(ref.p, inst.p) / (AbsDot(inst.n, -wi));
		}
		if (std::isinf(*pdf)) {
			*pdf = 0;
		}
		return inst;
	}

	//建立局部坐标系,Z轴指向圆心
	Vector3f wZ = Normalize(centerW-ref.p);
	Vector3f wX, wY;
	CoordinateSystem(wZ,&wX,&wY);

	//计算在局部空间中的球体参数坐标
	Float sinThetaMax2 = _radius * _radius / DistanceSquared(ref.p, centerW);
	Float cosThetaMax = std::sqrt(std::max((Float)0, 1 - sinThetaMax2));
	Float cosTheta = (1 - u[0]) + u[0] * cosThetaMax;
	Float sinTheta = std::sqrt(std::max((Float)0, 1 - cosTheta * cosTheta));
	Float phi = u[1] * 2 * Pi;

	//下面的代码用了非常神奇的算法计算了球体交点,详细见PBRT第三版蒙特卡洛采样章节
	Float dc = Distance(ref.p, centerW);
	Float ds = dc * cosTheta -
		std::sqrt(std::max(
		(Float)0, _radius * _radius - dc * dc * sinTheta * sinTheta));
	Float cosAlpha = (dc * dc + _radius * _radius - ds * ds) / (2 * dc * _radius);
	Float sinAlpha = std::sqrt(std::max((Float)0, 1 - cosAlpha * cosAlpha));

	Vector3f nWorld =
		SphericalDirection(sinAlpha, cosAlpha, phi, -wX, -wY, -wZ);
	Point3f pWorld = centerW + Point3f(nWorld.x, nWorld.y, nWorld.z)*_radius;
	
	Interaction it;
	it.p = pWorld;
	it.pErr = gamma(5) * Abs((Vector3f)pWorld);
	it.n = Normal3f(nWorld);
	if (reverseOrientation) {
		it.n *= -1; 
	}
	
	//均匀采样cone
	*pdf = 1 / (2 * Pi * (1 - cosThetaMax));
	if (std::isinf(*pdf)) {
		*pdf = 0;
	}
	return it;

}

std::shared_ptr<Shape> CreateSphereShape(const Transform *o2w,
                                         const Transform *w2o,
                                         bool reverseOrientation,
                                         const ParamSet &params) {
    Float radius = params.FindOneFloat("radius", 1.0f);
    Float zmin = params.FindOneFloat("zmin", -radius);
    Float zmax = params.FindOneFloat("zmax", radius);
    Float phimax = params.FindOneFloat("phimax", 360.0f);
    Debug("[CreateSphereShape][radius:"<<radius<<" ,zmin:"<<zmin<<" ,zmax:" <<zmax<<" ,phimax:"<<phimax<<"]");
	Debug("[CreateSphereShape][ o2w:" << *o2w << "]");
    return std::make_shared<Sphere>(o2w, w2o, reverseOrientation, radius, zmin,
                                    zmax, phimax);
}

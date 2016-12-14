/*
 * reflection.h
 *
 *  Created on: 2016年12月6日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_REFLECTION_H_
#define SRC_CORE_REFLECTION_H_

//BxDF坐标系下
//法线(0,0,1)与W(x,y,z)点乘等于W.z
inline Float CosTheta(const Vector3f &w) {
	return w.z;
}

inline Float CosTheta2(const Vector3f &w) {
	return w.z * w.z;
}

inline Float AbsCosTheta(const Vector3f &w) {
	return std::abs(w.z);
}

//三角函数公式:sin^2+cos^2=1
inline Float SinTheta2(const Vector3f& w) {
	return std::max(0.0f, 1.0f - CosTheta2(w)); //这里进行max操作防止Float误差导致值小于0
}

inline Float SinTheta(const Vector3f& w) {
	return std::sqrt(SinTheta2(w));
}

inline Float TanTheta(const Vector3f& w) {
	return SinTheta(w) / CosTheta(w);
}

inline Float TanTheta2(const Vector3f& w) {
	return SinTheta2(w) / CosTheta2(w);
}

inline Float CosPhi(const Vector3f& w) {
	Float sint = SinTheta(w);
	if (sint == 0.0f) {
		return 1.0f;
	}
	//sint等于是w在x-y平面上投影的长度，斜边
	//w.x/sint 等于对边比斜边,等于角度的cos值
	//一如既往的加入float误差的保护
	return Clamp(w.x / sint, -1.f, 1.f);
}

inline Float CosPhi2(const Vector3f& w){
	Float cosphi=CosPhi(w);
	return cosphi*cosphi;
}

inline Float SinPhi(const Vector3f& w) {
	Float sint = SinTheta(w);
	if (sint == 0.0f) {
		return 0.0f;
	}
	//sint等于是w在x-y平面上投影的长度，斜边
	//w.y/sint 等于邻边比斜边,等于角度的sin值
	//一如既往的加入float误差的保护
	return Clamp(w.y / sint, -1.f, 1.f);
}

inline Float SinPhi2(const Vector3f& w){
	Float sinphi=SinPhi(w);
	return sinphi*sinphi;
}

//计算反射方向
//默认wo和n在同一半球
Vector3f Reflect(const Vector3f& wo,const Normal3f& n){
	return Vector3f(2*Dot(wo,n)*n)-wo;
}
//计算折射方向
//默认wt和n在同一半球
//eta是材质的折射率
bool Refract(const Vector3f& wi,const Normal3f& n,Float oeta/*这里是两个折射率之比(i/t)*/,Vector3f* wt){
	//1.先求cosThetaT
	//2.然后使用推导的折射方向公式
	Float cosThetaI=Dot(wi,n);
	Float sinThetaI2=std::max(0.0f,1.0f-cosThetaI*cosThetaI);
	//这里运用snell law
	Float sinThetaT2=oeta*sinThetaI2;
	if(sinThetaT2>=1){
		return false;//完全反射情况
	}
	Float cosThetaT=std::sqrt(1.0f-sinThetaT2);
	//代入公式
	*wt=oeta*(-wi)+(oeta*Dot(wi,n)-cosThetaT)*Vector3f(n);
	return true;
}

//这里把BxDF抽象成3个类型 Specular/Diffuse/Glossy
//两个行为 Reflection/Transmission
enum BxDFType {
	BSDF_REFLECTION = 1 << 0,  //反射
	BSDF_TRANSMISSION = 1 << 1,  //折射
	BSDF_DIFFUSE = 1 << 2,  //漫反射
	BSDF_GLOSSY = 1 << 3,
	BSDF_SPECULAR = 1 << 4,  //镜面反射
	BSDF_ALL_TYPES = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR,
	BSDF_ALL_REFLECTION = BSDF_REFLECTION | BSDF_ALL_TYPES,
	BSDF_ALL_TRANSMISSION = BSDF_TRANSMISSION | BSDF_ALL_TYPES,
	BSDF_ALL = BSDF_ALL_REFLECTION | BSDF_ALL_TRANSMISSION
};

//判断两个向量是否在同一半球
//BxDF坐标系下
inline bool SameHemisphere(const Vector3f &w, const Vector3f &wp) {
	return w.z * wp.z > 0.f;
}

#endif /* SRC_CORE_REFLECTION_H_ */
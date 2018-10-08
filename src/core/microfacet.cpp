#include "microfacet.h"
#include "reflection.h"


//直接copy自PBRT的GGX粗糙度到alpha的工具函数
Float GGXRoughnessToAlpha(Float roughness) {
	roughness = std::max(roughness, (Float)1e-3);
	Float x = std::log(roughness);
	return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x +
		0.000640711f * x * x * x * x;
}

Float IsotropyBeckmannDistribution::D(const Vector3f &wh) const{
	//1.原始公式
	////计算tan(theta)^2
	//Float tanTheta2=TanTheta2(wh);
	////graze angle
	//if (std::isinf(tanTheta2)) {
	//	return 0;
	//}
	////代入公式
	//Float alpha2 = _alpha*_alpha;
	//Float cosTheta2 = CosTheta2(wh);
	//return std::exp(-tanTheta2 / alpha2) / (Pi*alpha2*cosTheta2*cosTheta2);

	//2.简化公式
	Float cosTheta2 = CosTheta2(wh);
	if (cosTheta2 == 0) {
		return 0;
	}
	Float alpha2 = _alpha*_alpha;
	return std::exp((cosTheta2 - 1) / (alpha2*cosTheta2)) / (Pi*alpha2*cosTheta2*cosTheta2);
}

Float IsotropyBeckmannDistribution::Lambda(const Vector3f &w) const{
	//这种东西真的是要靠科学家来推导出来，厉害厉害
    Float absTanTheta=std::abs(TanTheta(w));
    if(std::isinf(absTanTheta)){
        return 0;
    }
    Float a=1/(_alpha*absTanTheta);
    if(a>1.6){
        return 0;
    }
    else{
        return (1-1.259*a+0.396*a*a)/(3.535*a+2.181*a*a);
    }
}

Vector3f IsotropyBeckmannDistribution::Sample_wh(const Vector3f &wo, const Point2f &u) const {
    
	Float logSample=std::log(u[0]);
	if (std::isinf(logSample)) {
		logSample = 0;
	}
	Float tanTheta2 = -(_alpha*_alpha)*logSample;
	Float cosTheta = 1 / (std::sqrt(tanTheta2 + 1));
	Float sinTheta = std::sqrt(std::max((Float)0.0, 1 - cosTheta*cosTheta));

    Float phi=2*Pi*u[1];
	Vector3f wh= SphericalDirection(sinTheta,cosTheta,phi);
	//保证在同一个半球
	if (!SameHemisphere(wo, wh)) {
		wh = -wh;
	}
    return wh;
}

Float AnisotropyGGXDistribution::D(const Vector3f &wh) const {
    
    Float cosTheta=AbsCosTheta(wh);
    if(cosTheta==0){
        return 0;
    }
    Float cosTheta4=(cosTheta*cosTheta)*(cosTheta*cosTheta);
    Float tanTheta2=TanTheta2(wh);

    Float sinPhi2=SinPhi2(wh);
    Float cosPhi2=CosPhi2(wh);

    Float alphaX2=_alphaX*_alphaX;
    Float alphaY2=_alphaY*_alphaY;

    Float term1=1+(cosPhi2/alphaX2+sinPhi2/alphaY2)*tanTheta2;
    Float term2=term1*term1;
    return 1.0/(Pi*_alphaX*_alphaY*cosTheta4*term2);
}

Float AnisotropyGGXDistribution::Lambda(const Vector3f &w) const{
    Float tanTheta2=TanTheta2(w);
    if(std::isinf(tanTheta2)){
        return 0;
    }
    Float alpha = std::sqrt(CosPhi2(w) * _alphaX * _alphaX +SinPhi2(w) * _alphaY * _alphaY);

    return (-1.0+std::sqrt(1.0+alpha*alpha*tanTheta2))/2.0;
}




Float IsotropyGGXDistribution::D(const Vector3f &wh) const {
	//各向同性版本和各向异性版本是有区别的
	//気をつけて
	Float cosTheta = AbsCosTheta(wh);
	if (cosTheta == 0) {
		return 0;
	}

	//这里使用简化成只包含一个cos(theta)成分的公式 GGX=a^2/[PI*(cos(theta)^2x(a^2-1)+1)^2]
	Float alpha2 = _alpha*_alpha;
	Float cosTheta2 = cosTheta*cosTheta;
	Float term = cosTheta2*(alpha2 - 1) + 1;
	return alpha2 / (Pi*term*term);
}

Float IsotropyGGXDistribution::Lambda(const Vector3f &w) const {
	//各向同性版本和各向异性版本是有区别的
	//気をつけて
	Float tanTheta2 = TanTheta2(w);
	if (std::isinf(tanTheta2)) {
		return 0;
	}
	
	return (-1.0 + std::sqrt(1.0 + _alpha*_alpha*tanTheta2))/2.0;
}


//这个是各项同性版本
 Vector3f IsotropyGGXDistribution::Sample_wh(const Vector3f &wo, const Point2f &u) const{
     //Pdf(theta)和Pdf(phi)是两个独立分布，因此可以独立处理
     Float phi=u.x*2*Pi;
     Float cosTheta=0;
     
	 cosTheta=std::sqrt((1-u.y)/(1+u.y*(_alpha*_alpha-1)));
     //sin2+cos2=1
     Float sinTheta=std::max(0.0,std::sqrt(1.0-(cosTheta*cosTheta)));
     Float x=sinTheta*std::cos(phi);
     Float y=sinTheta*std::sin(phi);
     Float z=cosTheta;
     Vector3f wh=Vector3f(x,y,z);
     //到底需不需要让wo和wh在同一个半球内，我并没有完全理解，暂时注释掉
	 //这里貌似应该要让wo和wh保持在同一个半球内,对D和pdf函数都没有影响
	 if(CosTheta(wo)<0){
         wh=-wh;
     }
     return wh;
 }

 //这个是各项同性版本
 //PDF = D * COS(wh)
 //返回的是半角向量空间的
 Float MicrofacetDistribution::Pdf(const Vector3f &wo, const Vector3f &wh) const {
	 return D(wh)*AbsCosTheta(wh);
 }
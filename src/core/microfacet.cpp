#include "microfacet.h"
#include "reflection.h"

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


//这个是各项同性版本
// Vector3f GGXDistribution::Sample_wh(const Vector3f &wo, const Point2f &u) const{
//     //Pdf(theta)和Pdf(phi)是两个独立分布，因此可以独立处理
//     Float phi=u.x*2*Pi;
//     Float cosTheta=0;
//     //各向同性
//     if(_alphaX==_alphaY){
//         cosTheta=std::sqrt((1-u.y)/(1+u.y*(_alphaX*_alphaY-1)));
//     }
//     //sin2+cos2=1
//     Float sinTheta=std::max(0.0,std::sqrt(1.0-(cosTheta*cosTheta)));
//     Float x=sinTheta*std::cos(phi);
//     Float y=sinTheta*std::sin(phi);
//     Float z=cosTheta;
//     Vector3f wh=Vector3f(x,y,z);
//     if(wo.z<0){
//         wh=-wh;
//     }
//     return wh;
// }
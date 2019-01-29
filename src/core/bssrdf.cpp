#include "bssrdf.h"
#include "parallel.h"
Float FresnelMoment1(Float eta) {
    Float eta2 = eta * eta, eta3 = eta2 * eta, eta4 = eta3 * eta,
          eta5 = eta4 * eta;
    if (eta < 1)
        return 0.45966f - 1.73965f * eta + 3.37668f * eta2 - 3.904945 * eta3 +
               2.49277f * eta4 - 0.68441f * eta5;
    else
        return -4.61686f + 11.1136f * eta - 10.4646f * eta2 + 5.11455f * eta3 -
               1.27198f * eta4 + 0.12746f * eta5;
}

Float FresnelMoment2(Float eta) {
    Float eta2 = eta * eta, eta3 = eta2 * eta, eta4 = eta3 * eta,
          eta5 = eta4 * eta;
    if (eta < 1) {
        return 0.27614f - 0.87350f * eta + 1.12077f * eta2 - 0.65095f * eta3 +
               0.07883f * eta4 + 0.04860f * eta5;
    } else {
        Float r_eta = 1 / eta, r_eta2 = r_eta * r_eta, r_eta3 = r_eta2 * r_eta;
        return -547.033f + 45.3087f * r_eta3 - 218.725f * r_eta2 +
               458.843f * r_eta + 404.557f * eta - 189.519f * eta2 +
               54.9327f * eta3 - 9.00603f * eta4 + 0.63942f * eta5;
    }
}


Spectrum SeparableBSSRDF::S(const Point3f& pi,const Vector3f& wi) const{
    //计算三个成分，并且相乘
    Float oneMinusFr=1.0f-FrDielectric(Dot(_po.wo,_po.shading.n),1,_eta);
    return oneMinusFr*Sp(pi)*Sw(wi);
}

Float BeamDiffusionMS(Float sigmaS,Float sigmaA,Float g,Float eta,Float r){
    //根据对称理论，计算sigmaS和sigmaT
    Float r_sigmaS=(1-g)*sigmaS;
    Float r_sigmaT=r_sigmaS+sigmaA;
    Float t_albedo=r_sigmaS/r_sigmaT;

    //Grosjean’s non-classical diffusion coefficient
    //考虑的是非典型情况下的
    //典型情况是指材质是高albedo,容易发生scattering,无限体积的材质
    Float D_G=(2*sigmaA+r_sigmaS)/(3*r_sigmaT*r_sigmaT);

    //effective transport coefficient
    //在monopole情况下，Fluence有解析解，其中有个成分就是这个
    Float sigma_tr=std::sqrt(sigmaA/D_G);

    //计算不依赖于monopole的位置的数据
    //比如，分割两个dipole光源的平面的位置
    Float fm1=FresnelMoment1(eta);
    Float fm2=FresnelMoment2(eta);
    Float ze = -2*D_G*(1+3*fm2)/(1-2*fm2);
    //再比如计算Radiant Exitance的Fluence成分的系数，以及Radiant Exitance的irradiance vector成分的系数
    //为简化版本:
    //Float co_phi=(0.25f-0.5f*fm1);
    //Float co_E=(0.5f-1.5f*fm2);
    //简化版本:
    Float co_phi=0.25f*(1-2*fm1);
    Float co_E=0.5f*(1-3*fm2);

    //
    const int nSamples=100;//样本数
    Float ED=0;
    for(int i=0;i<nSamples;++i){
        //计算样本值
        Float u=(i+0.5f)/nSamples;
        //MIS采样zr
        //其实可以写成:Float zr=std::log(u)/r_sigmaT;
        Float zr=std::log(1-u)/r_sigmaT;
        //计算虚拟光源的位置
        Float zv=-zr+2*ze;
        //计算表面上的出射点到两个光源的距离
        //基本的三角函数运用
        Float dr=std::sqrt(r*r+zr*zr);
        Float dv=std::sqrt(r*r+zv*zv);

        //计算Radiant Exitance的Fluence成分
        Float phiD=(std::exp(-sigma_tr*dr)/dr-std::exp(-sigma_tr*dv)/dv)*Inv4Pi/D_G;
        //计算Radiant Exitance的irradiance vector dot n 成分
        Float negNDotE=Inv4Pi*( zr*(1+dr*sigma_tr)*std::exp(-sigma_tr*dr)/(dr*dr*dr) - zv*(1+dv*sigma_tr)*std::exp(-sigma_tr*dv)/(dv*dv*dv) );
        //计算Radiant Exitance
        Float E=phiD*co_phi+negNDotE*co_E;

        //经验缩放数
        Float kappa = 1 - std::exp(-2 * r_sigmaT * (dr + zr));
        //第一个t_albedo来自r_sigmaS和MIS的pdf的比值
        //第二个来自Grosjean’s non-classical monopole 
        ED+=kappa*t_albedo*t_albedo*E;
    }

    return ED;
}

void ComputeBeamDiffusionBSSRDF(Float g,Float eta,BSSRDFTable* t){
    Assert(t!=nullptr);
    //首先生成指数分布的radius样本
    t->radiusSamples[0]=0;
    t->radiusSamples[1]=2.5e-3;//PBRT为何使用这个指数分布呢？？？
    for(int i=2;i<t->numRadiusSample;++i){
        t->radiusSamples[i]=t->radiusSamples[i-1]*1.2f;
    }

    //计算非线性的albedo样本
    //使用的公式为albedo=(1-exp(-8i/(N-1)))/(1-exp(-8))
    for(int i=0;i<t->numAlbedoSample;++i){
        t->albedoSamples[i]=(1-std::exp(-8*i/(Float)(t->numAlbedoSample-1)))/(1-std::exp(-8));
    }

    //并行的计算profile
    ParallelFor([&](int64_t i){
        for(int j=0;j<t->numRadiusSample;++j){
            Float albedo=t->albedoSamples[i];
            Float r=t->radiusSamples[j];
            //计算边缘profile
            t->profile[i*t->numRadiusSample+j]=2*Pi*r*BeamDiffusionMS(albedo,1-albedo,g,eta,r);//TODO Single-Scattering Event还没有考虑
        }
    },t->numAlbedoSample);
    
    
}
#include "bssrdf.h"
#include "parallel.h"
#include "interpolation.h"
#include "memory.h"
#include "scene.h"
#include "primitive.h"
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

 Spectrum TabulatedBSSRDF::Sr(Float r) const{
       Spectrum Sr(0);

       for(int i=0;i<Spectrum::numSample;++i){
           //计算无单位的光学半径
           Float opticalR=_sigmaT[i]*r;
           //single-scattering albedo
           Float albedo=_albedo[i];
           
           //样条插值获得权重
           int rOffset=0;
           Float rWeights[4];

           int albedoOffset=0;
           Float albedoWeights[4]; 

           bool b0=CatmullRomWeights(_table.numRadiusSample,_table.radiusSamples.get(),opticalR,&rOffset,rWeights);
           bool b1=CatmullRomWeights(_table.numAlbedoSample,_table.albedoSamples.get(),albedo,&albedoOffset,albedoWeights);
           
           if((!b0)||(!b1)){
               continue;
           }

           Float sr=0;
           //遍历weights,计算函数值
           for(int j=0;j<4;++j){
               for(int k=0;k<4;++k){
                   Float w=albedoWeights[j]*rWeights[k];
                   sr+=w*_table.EvalProfile(albedoOffset+j,rOffset+k);
               }
           }
           //从边缘PDF 转换到 disjoint PDF
           if(opticalR!=0){
               sr/=(2*Pi*opticalR);
           }

           Sr[i]=sr;
       }

       //乘上因为空间变换而带来的因子
       Sr=Sr*_sigmaT*_sigmaT;

       return Clamp(Sr);
    }

Float BeamDiffusionSS(Float sigmaS,Float sigmaA,Float g,Float eta,Float r){
    Float sigmaT=sigmaS+sigmaA;
    Float albedo=sigmaS/sigmaT;

    //计算全反射的临界角度
    //theta_c=arcsin(1/eta)
    //因此cosTheta_c=sqrt(1-1/eta^2)
    
    //根据临界角度来算最小的t值
    Float tc=r*std::sqrt(eta*eta-1);

    const int numSample=100;
    Float E=0;

    //MC计算积分
    for(int i=0;i<numSample;++i){
        //采样t值
         Float ti = tc - std::log(1 - (i + 0.5f) / numSample) / sigmaT;
         //计算到po点的距离
         Float d=std::sqrt(r*r+ti*ti);
         //计算ti和d之间余弦值
         Float cosThetaS=ti/d;
         //累计贡献
         //公式推导详见书,这里已经是f(x)/p(x)了
         E += albedo * std::exp(-sigmaT * (d + tc)) / (d * d) *
                  PhaseHG(cosThetaS, g) * (1 - FrDielectric(-cosThetaS, 1, eta)) *
                  std::abs(cosThetaS);
    }
    //最后不要忘记MC最后一步，除以样本数
    E=E/numSample;
    return E;
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
    Float ze = -2*D_G*(1+3*fm2)/(1-2*fm1);
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
        Float zr=-std::log(1-u)/r_sigmaT;
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
        
        //这里已经考虑了mis，只不过PDF部分和 Source部分相互抵消了
        // float pdf=r_sigmaT*std::exp(-r_sigmaT*zr);
        // float source=t_albedo*r_sigmaT*std::exp(-r_sigmaT*zr);

        ED+=kappa*t_albedo*t_albedo*E;

        //ED+=kappa*t_albedo*E/pdf;
    }
    
    return ED/nSamples;
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
        t->albedoSamples[i]=(1-std::exp(-8*i/(Float)(t->numAlbedoSample-1)))/(1-std::exp(-8.0f));
    }

    //并行的计算profile
    ParallelFor([&](size_t i){
        for(int j=0;j<t->numRadiusSample;++j){
            Float albedo=t->albedoSamples[i];
            Float r=t->radiusSamples[j];
            //计算边缘profile
            t->profile[i*t->numRadiusSample+j]=2*Pi*r*(BeamDiffusionMS(albedo,1-albedo,g,eta,r)+BeamDiffusionSS(albedo,1-albedo,g,eta,r));
        }
        //计算eff albedo
        t->albedoEff[i]=IntegrateCatmullRom(t->numRadiusSample,t->radiusSamples.get(),&t->profile[i*t->numRadiusSample],&t->profileCDF[i*t->numRadiusSample]);
    },t->numAlbedoSample);
    
}


Spectrum SeparableBSSRDF::Sample_S(const Scene &scene, Float u1, const Point2f &u2, MemoryArena &arena, SurfaceInteraction *si, Float *pdf) const{
    Spectrum sp=Sample_Sp(scene,u1,u2,arena,si,pdf);
    if(!sp.IsBlack()){
        si->bsdf=ARENA_ALLOC(arena,BSDF)(*si);
        si->bsdf->Add(ARENA_ALLOC(arena, SeparableBSSRDFAdapter)(this));
        si->wo = Vector3f(si->shading.n);
    }
    return sp;
}

class IntersectionChain{
public:
    SurfaceInteraction si;
    IntersectionChain* next=nullptr;
};

Spectrum SeparableBSSRDF::Sample_Sp(const Scene &scene, Float u1, const Point2f &u2, MemoryArena &arena, SurfaceInteraction *si, Float *pdf) const{
    //首先根据样本值，从三个轴里面，选出一个轴作为投影平面的法线方向
    //PBRT是给了50%的概率给表面垂直方向
    //另外50%的概率给两个切线方向
    //还需要重新缩放u1,使其再映射到0~1范围
    Vector3f sv,tv,nv;
    if(u1<0.5f){
        sv=_ss;
        tv=_ts;
        nv=Vector3f(_ns);
        u1=u1*2;
    }else if(u1<0.75f){
        sv=_ts;
        tv=Vector3f(_ns);
        nv=_ss;
        u1=(u1-0.5f)*4;
    }else {
        sv=Vector3f(_ns);
        tv=_ss;
        nv=_ts;
        u1=(u1-0.75f)*4;
    }

    //然后选择采样profile的哪个通道，并且重新缩放u1
    int ch=Clamp((int)u1*Spectrum::numSample,0,Spectrum::numSample-1);
    u1=u1*Spectrum::numSample-ch;//>_< 真是省样本啊
    //采样r和phi
    //然后根据通道的索引，采样Sr成分
    Float r=Sample_Sr(ch,u2[0]);
    if(r<0){
        return Spectrum(0);
    }
    //均匀分布
    Float phi=2*Pi*u2[1];
    //计算最大半径,大于这个半径的能量可以忽略不计(这个假设在光源非常强的情况下，可能会失败)
    //rMax定义的球体包含99.9%的能量
    Float rMax=Sample_Sr(ch,0.999f);
    if(r>=rMax){
         return Spectrum(0);
    }
    //根据勾股定律计算probe射线的长度
    //(l/2)^2+r^2=rMax^2
    Float l = 2 * std::sqrt(rMax * rMax - r * r);

    //计算probe射线的起点和目标点
    Interaction base;
    base.p=_po.p+(sv*std::cos(phi)+tv*std::sin(phi))*r-nv*l*0.5f;
    base.time=_po.time;
    Point3f target=base.p+nv*l;

    //把probe射线相交的所有的交点形成链表
    IntersectionChain *chain=ARENA_ALLOC(arena, IntersectionChain)();
    int numFound=0;
    IntersectionChain* ptr=chain;
    while(scene.Intersect(base.SpawnRayTo(target),&ptr->si)){
        base=ptr->si;//更新base,是得下次生成新射线是在最新的交点开始生成
        if(ptr->si.primitive->GetMaterial()==_material){
            IntersectionChain *next=ARENA_ALLOC(arena, IntersectionChain)();
            ptr->next=next;
            ptr=next;
            numFound++;
        }
    }
    //链表完成
    if(numFound==0){
        return Spectrum(0);
    }
    //从链表中随机选取一个交点

    int selected=Clamp(u1*numFound,0,numFound-1);

    while((selected--)>0){
        chain=chain->next;
    }
    (*si)=chain->si;
    //这里是两个pdf的积
    (*pdf)=Pdf_Sp(*si)/numFound;
    return Sp(*si);
}


Float SeparableBSSRDF::Pdf_Sp(const SurfaceInteraction& pi) const {
    //从世界坐标系转换到局部坐标系
    Normal3f nLocal=Normal3f(Dot(_ss,pi.n),Dot(_ts,pi.n),Dot(_ns,pi.n));
    Vector3f d=_po.p-pi.p;
    Vector3f dLocal=Vector3f(Dot(_ss,d),Dot(_ts,d),Dot(_ns,d));

    //计算映射到三个平面后的r的值
    Float rProj[3]={
        std::sqrt(dLocal.y*dLocal.y+dLocal.z*dLocal.z),
        std::sqrt(dLocal.x*dLocal.x+dLocal.z*dLocal.z),
        std::sqrt(dLocal.y*dLocal.y+dLocal.x*dLocal.x),
    };
    //下面是我困惑的地方，到底是不是MIS呢
    Float pdf=0;
    Float axisPdf[3]={0.25f,0.25f,0.5f};
    Float chPdf=1/(Float)Spectrum::numSample;
    for(int axis=0;axis<3;++axis){
        for(int ch=0;ch<Spectrum::numSample;++ch){
            pdf+=Pdf_Sr(ch,rProj[axis])*axisPdf[axis]*chPdf*std::abs(nLocal[axis]);//最后为啥还要乘以一个std::abs(nLocal[axis])???
        }
    }
    return pdf;
}
 
 Float TabulatedBSSRDF::Sample_Sr(int ch,Float u) const{
     if(_sigmaT[ch]==0){
         return -1;
     }
     return SampleCatmullRom2D(_table.numAlbedoSample,_table.numRadiusSample,_table.albedoSamples.get(),_table.radiusSamples.get(),_table.profile.get(),_table.profileCDF.get(),_albedo[ch],u)/_sigmaT[ch];
 }

 Float TabulatedBSSRDF::Pdf_Sr(int ch,Float r) const{
     //计算无单位的光学半径
           Float opticalR=_sigmaT[ch]*r;
           //single-scattering albedo
           Float albedo=_albedo[ch];
           
           //样条插值获得权重
           int rOffset=0;
           Float rWeights[4];

           int albedoOffset=0;
           Float albedoWeights[4]; 

           bool b0=CatmullRomWeights(_table.numRadiusSample,_table.radiusSamples.get(),opticalR,&rOffset,rWeights);
           bool b1=CatmullRomWeights(_table.numAlbedoSample,_table.albedoSamples.get(),albedo,&albedoOffset,albedoWeights);
           
           if((!b0)||(!b1)){
               return 0;
           }

           Float sr=0;
           Float effAlbedo=0;
           //遍历weights,计算函数值
           for(int j=0;j<4;++j){
               if(albedoWeights[j]==0){
                   continue;
               }
               effAlbedo+=_table.albedoEff[albedoOffset+j]*albedoWeights[j];
               for(int k=0;k<4;++k){
                   if(rWeights[k]==0){
                       continue;
                   }
                   Float w=albedoWeights[j]*rWeights[k];
                   sr+=w*_table.EvalProfile(albedoOffset+j,rOffset+k);
               }
           }
           //从边缘PDF 转换到 disjoint PDF
           if(opticalR!=0){
               sr/=(2*Pi*opticalR);
           }

           return std::max((Float)0,sr*_sigmaT[ch]*_sigmaT[ch]/effAlbedo);
 }

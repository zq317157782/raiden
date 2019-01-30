#pragma once
#include "raiden.h"
#include "geometry.h"
#include "reflection.h"

//从PBRT复制的两个工具函数
//使用多项式近似来近似菲涅尔的第一M和第二M
//参数使用的是折射系数的倒数(和PBRT的实现有关)
Float FresnelMoment1(Float invEta);
Float FresnelMoment2(Float invEta);

//双向次表面反射分布函数
class BSSRDF{
protected:
	const SurfaceInteraction &_po;//出射点和方向
	const Float _eta;//折射系数
public:
	BSSRDF(const SurfaceInteraction &po,Float eta):_po(po),_eta(eta){}
	//S(po,wo,pi,wi)
	//出射的微元radiance和入射的微元flux之比
	virtual Spectrum S(const Point3f& pi,const Vector3f& wi) const=0;
};


//可分离的BSSRDF模型
//BSSRDF模型太通用了，难度很大，所以一般学术会使用可分离的成分来近似BSSRDF
// S=(1-Fr)*Sp*Sw
class SeparableBSSRDF:public BSSRDF{
protected:
    //位置相关成分
    Spectrum Sp(const Point3f& pi) const{
        return Sr(Distance(_po.p,pi));
    }
     //角度相关成分
    Spectrum Sw(const Vector3f& wi) const{
        //Sw = (1-Fr(cosThetaI))/(c*Pi);
        //c = 1-2*first moment of Fresnel
        
        float c=1-2*FresnelMoment1(1/_eta);
        //wi是BSSRDF局部空间的
        return (1-FrDielectric(CosTheta(wi),1,_eta))/(c*Pi);
    }

	//距离相关成分
    virtual Spectrum Sr(Float d) const=0;

    //BSSRD的局部Frame 
    const Normal3f _ns;
    const Vector3f _ss;
    const Vector3f _ts;
    
public:
    SeparableBSSRDF(const SurfaceInteraction &po,Float eta):BSSRDF(po,eta),_ns(po.shading.n),_ss(Normalize(po.shading.dpdu)),_ts(Cross(_ns,_ss)){}

    virtual Spectrum S(const Point3f& pi,const Vector3f& wi) const override;
};


//记录bssrdf profile的表
class BSSRDFTable{
public:
    const int numAlbedoSample;//albedo样本个数
    const int numRadiusSample;//半径样本个数
    std::unique_ptr<Float[]> albedoSamples;
    std::unique_ptr<Float[]> radiusSamples;
    std::unique_ptr<Float[]> albedoEff;

    std::unique_ptr<Float[]> profile;//存的是极坐标空间下的边缘PDF(未标准化),多乘了2*Pi*r,r是光学半径
    std::unique_ptr<Float[]> profileCDF;
public:
    BSSRDFTable(int nAlbedoSample,int nRadiusSample):numAlbedoSample(nAlbedoSample),numRadiusSample(nRadiusSample){
       
        albedoSamples=std::unique_ptr<Float[]>(new Float[numAlbedoSample]);
        radiusSamples=std::unique_ptr<Float[]>(new Float[numRadiusSample]);

        albedoEff=std::unique_ptr<Float[]>(new Float[numAlbedoSample]);
       
        profile=std::unique_ptr<Float[]>(new Float[numAlbedoSample*numRadiusSample]);
        profileCDF=std::unique_ptr<Float[]>(new Float[numAlbedoSample*numRadiusSample]);
    }

    inline Float EvalProfile(int albedoIdx,int radiusIdx) const{
        return profile[albedoIdx*numRadiusSample+radiusIdx];
    }
};

//制表的BSSRDF
//使用一张二维表(albedo,radii)作为Sr的输出
//原来的Sr函数是个5维函数 Sr(r,albedo,sigma_t,g,eta)
class TabulatedBSSRDF: public SeparableBSSRDF{
private:
    //const Spectrum _sigmaS;//scattering coefficient
    Spectrum _sigmaT;
    Spectrum _albedo;
    const BSSRDFTable& _table;

     //计算profile
    virtual Spectrum Sr(Float d) const override;
public:
    TabulatedBSSRDF(const SurfaceInteraction &po,Float eta,const Spectrum& sigmaS,const Spectrum& sigmaA,const BSSRDFTable& table):SeparableBSSRDF(po,eta),_sigmaT(sigmaS+sigmaA),_table(table){
        //计算albedo
        for(int i=0;i<Spectrum::numSample;++i){
            //处理除数为0的情况
            _albedo[i]=(_sigmaT[i]==0)?0:(sigmaS[i]/_sigmaT[i]);
        }
    }

    
};

//使用PBD方法计算表面点处的Multi-Scattering造成的irradiance
Float BeamDiffusionMS(Float sigmaS,Float sigmaA,Float g,Float eta,Float r);

//填充BSSRDFTable
void ComputeBeamDiffusionBSSRDF(Float g,Float eta,BSSRDFTable* table);


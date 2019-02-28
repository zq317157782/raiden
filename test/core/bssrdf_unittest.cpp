#include "gtest.h"
#include "bssrdf.h"
#include <iostream>
#include <fstream>
#include "sampling.h"
#include "interpolation.h"

TEST(DuffusionEquation,ComputeBeamDiffusionBSSRDF){
    BSSRDFTable table(8,8);
    ComputeBeamDiffusionBSSRDF(0,1.5,&table);
    //非线性分布，集中在1
    //(1-std::exp(-8*i/(Float)(7)))/(1-std::exp(-8.0f))
    EXPECT_FLOAT_EQ(table.albedoSamples[0],(1-std::exp(-8*0/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[1],(1-std::exp(-8*1/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[2],(1-std::exp(-8*2/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[3],(1-std::exp(-8*3/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[4],(1-std::exp(-8*4/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[5],(1-std::exp(-8*5/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[6],(1-std::exp(-8*6/(Float)(7)))/(1-std::exp(-8.0f)));
    EXPECT_FLOAT_EQ(table.albedoSamples[7],1);
    //指数分布
    EXPECT_FLOAT_EQ(table.radiusSamples[0],0);
    EXPECT_FLOAT_EQ(table.radiusSamples[1],0.0024999999);
    EXPECT_FLOAT_EQ(table.radiusSamples[2],0.003);
    EXPECT_FLOAT_EQ(table.radiusSamples[3],0.0036000002);
    EXPECT_FLOAT_EQ(table.radiusSamples[4],0.0043200003);
    EXPECT_FLOAT_EQ(table.radiusSamples[5],0.0051840004);
    EXPECT_FLOAT_EQ(table.radiusSamples[6],0.0062208008);
    EXPECT_FLOAT_EQ(table.radiusSamples[7],0.0074649611);
}


TEST(DuffusionEquation,ProfileFile100X64){
    BSSRDFTable table(100,64);
    ComputeBeamDiffusionBSSRDF(0,1.3,&table);
    std::ofstream myfile;
    myfile.open ("profile_samples.data");
    myfile<<" #albedo radius profile_data\n";
    for(int i=0;i<100;++i){
        for(int j=0;j<64;++j){
            myfile<<table.albedoSamples[i]<<" "<<table.radiusSamples[j]<<" "<<table.profile[i*64+j]<<"\n";
        }
    }
    
    myfile.close();
}


TEST(DuffusionEquation,RadiuSamplesData){
    std::ofstream myfile;
    myfile.open ("sr_samples.data");
    myfile<<" #radius samples\n";
    myfile<<"\"raw samples\"\n";
    float radius[64];
    radius[0]=0;
    radius[1]=2.5e-3 ;
    for(int i=2;i<64;++i){
        radius[i]=radius[i-1]*1.2f;
    }
    for(int i=0;i<64;++i){
        myfile<<i<<" "<<radius[i]<<'\n';
    }
}




TEST(SeparableBSSRDFA,Sw){
    auto Sw=[](const Vector3f& wi,Float eta){
         float c=1-2*FresnelMoment1(1/eta);
        //wi是BSSRDF局部空间的
        return (1-FrDielectric(CosTheta(wi),1,eta))/(c*Pi);
    };

    RNG rng;
    Spectrum sum=0;
    const int numSample=30000;
    for(int i=0;i<numSample;++i){
        auto wi= UniformSampleHemisphere({rng.UniformFloat(),rng.UniformFloat()});
        auto f=Sw(wi,1.5);
        sum=sum+f*AbsCosTheta(wi);
    }
    sum=sum/numSample/UniformHemispherePdf();
    Float rgb[3];
    sum.ToRGB(rgb);
    EXPECT_LE(rgb[0],1.01);
    EXPECT_GE(rgb[0],0.99);

}


#include <ImfNamespace.h>
#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>
namespace IMF = OPENEXR_IMF_NAMESPACE;


TEST(BeamDiffusionMS,2DPreview){
    //输出皮肤的dipole preview

    /*
     t->radiusSamples[0]=0;
    t->radiusSamples[1]=2.5e-3;//PBRT为何使用这个指数分布呢？？？
    for(int i=2;i<t->numRadiusSample;++i){
        t->radiusSamples[i]=t->radiusSamples[i-1]*1.2f;
    }
    */
    float radius[64];
    radius[0]=0;
    radius[1]=2.5e-3 ;
    for(int i=2;i<64;++i){
        radius[i]=radius[i-1]*1.2f;
    }

    float eta=1.3f;
    float g=0;
    std::vector<IMF::Rgba> image;

    // for(int i=0;i<128;++i){
    //     float r=radius[i];
    //     float red=BeamDiffusionMS(0.740/(0.740+0.032),0.032/(0.740+0.032),g,eta,r);
    //     LInfo<<"r:"<<r<<" v:"<<red;
    // }   
    for(int i=0;i<128;++i){
        for(int j=0;j<128;++j){
            float r=0;

            float w=std::abs(i+0.5f-64);
            float h=std::abs(j+0.5f-64);
            int d=(int)std::sqrt(w*w+h*h);
            if(d>=64){
                d=63;
            }
            r=radius[d];
            

            

            float red=BeamDiffusionMS(0.740/(0.740+0.032),0.032/(0.740+0.032),g,eta,r);
            float green=BeamDiffusionMS(0.880/(0.880+0.170),0.170/(0.880+0.170),g,eta,r);
            float blue=BeamDiffusionMS(1.010/(1.010+0.480),0.480/(1.010+0.480),g,eta,r);
            image.push_back(IMF::Rgba{red, green, blue, (float)1});
        }
    }

    
     //WriteOpenEXR(, &image[0], 256, 256);

    IMF::RgbaOutputFile file("BeamDiffusionMS.exr", 128, 128, IMF::WRITE_RGBA);
    file.setFrameBuffer(&image[0], 1, 128);
    file.writePixels(128);
}

// TEST(SeparableBSSRDFAdapter,Sw){

//     SurfaceInteraction po;
//     po.p=Point3f(0,0,0);
//     BSSRDFTable table(10,10);
//     TabulatedBSSRDF bssrdf(po,nullptr,1.5,Spectrum(0),Spectrum(0),table);
//     SeparableBSSRDFAdapter adapter(&bssrdf);
    

//     RNG rng;
//     Spectrum sum=0;
//     const int numSample=30000;
//     for(int i=0;i<numSample;++i){
//         Vector3f wo(0,0,1);
//         Vector3f wi;
//         Float pdf;
//         auto f=adapter.Sample_f(wo,&wi,Point2f(rng.UniformFloat(),rng.UniformFloat()),&pdf);

//         sum=sum+f*AbsCosTheta(wi)/pdf;
//     }
//     sum=sum/numSample;
//     EXPECT_EQ(sum,1);

// }
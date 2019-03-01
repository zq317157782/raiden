#include "raiden.h"
#include "reflection.h"
//#include "transform.h"
#include "gtest.h"
#include "sampling.h"
#include "spectrum.h"

TEST(FrConductor,Au){
   Float air_ior[3]={1,1,1};
//    float au_ior[3]={0.15557,0.42415,1.3831};
//    float au_k[3] = {3.5024,2.4721,1.9155};
    Float au_ior[3]={0.18299,0.42108,1.3734};
   Float au_k[3] = {3.4242,2.3459,1.7704};
   
   Spectrum air=Spectrum::FromRGB(air_ior);
   Spectrum ior=Spectrum::FromRGB(au_ior);
   Spectrum k=Spectrum::FromRGB(au_k);

   Spectrum r=FrConductor(1,air,ior,k);
   Float au[3];
   r.ToRGB(au);
//    EXPECT_FLOAT_EQ(au[0],1.000);
//    EXPECT_FLOAT_EQ(au[1],0.766);
//    EXPECT_FLOAT_EQ(au[2],0.336);

   EXPECT_GE(au[0],0.9);
   EXPECT_GE(au[1],0.7);
   EXPECT_GE(au[2],0.3);
}

TEST(HairBSDF,WhiteFurnace){
   RNG rng;
   auto wo=UniformSampleSphere({rng.UniformFloat(),rng.UniformFloat()});
   for(Float betam=0.1;betam<=1;betam+=0.2){
      for(Float betan=0.1;betan<=1;betan+=0.2){
         int count=300000;
         Spectrum sigmaA(0);
         Spectrum sum(0);
         for(int i=0;i<count;++i){
            Float h=-1+2*rng.UniformFloat();
            HairBSDF bsdf(h,1.55f,sigmaA,betam,betan,0.0f);
            auto wi=UniformSampleSphere({rng.UniformFloat(),rng.UniformFloat()});
            sum+=bsdf.f(wo,wi)*AbsCosTheta(wi);
         }
         sum=sum/(count*UniformSpherePdf());
         EXPECT_GT(sum.y(),0.95);
         EXPECT_LT(sum.y(),1.05);
      }
   }
}

// TEST(HairBSDF,SampleWeights){
//    RNG rng;
//    auto wo=UniformSampleSphere({rng.UniformFloat(),rng.UniformFloat()});
//     for(Float betam=0.1;betam<=1;betam+=0.2){
//       for(Float betan=0.1;betan<=1;betan+=0.2){
//          int count=1000;
//          Spectrum sigmaA(0);
//          for(int i=0;i<count;++i){
//             Float h=-1+2*rng.UniformFloat();
//             HairBSDF bsdf(h,1.55f,sigmaA,betam,betan,0.0f);
//             Vector3f wi;
//             Float pdf;
//             auto f=bsdf.Sample_f(wo,&wi,{rng.UniformFloat(),rng.UniformFloat()},&pdf);
//             if(pdf>0){
//                EXPECT_EQ(f.y()*AbsCosTheta(wi),pdf);
//             }
//          }
//       }
//    }
// }

// TEST(FrDielectric,Glass){
//    for(int i=0;i<100;++i){
//       Float r = FrDielectric(i/(Float)99,1,1.5);
//       EXPECT_EQ(r,0)<<"cos: "<<i/(Float)99;
//    }
// }


TEST(Refract,Refract){
   Vector3f wi;
   Refract(Normalize(Vector3f(0.0,0,1)),Normal3f(0,0,1),1.5,&wi);
   EXPECT_EQ(wi,Vector3f(0,0,-1));
   EXPECT_EQ(wi.Length(),1);
}

TEST(FresnelDielectric,Compare){
   FresnelDielectric  fresnel(1.0,1.5);
   Spectrum one(1);
   SpecularReflection rf(one,&fresnel);
   SpecularTransmission rt(one,1,1.5,TransportMode::Radiance);

   Vector3f wo=Normalize(Vector3f(0.3,0,1));
   Vector3f wi;
   Float pdf;
   Spectrum A=rf.Sample_f(wo,&wi,Point2f(0,0),&pdf);
   Spectrum B=rt.Sample_f(wo,&wi,Point2f(0,0),&pdf);
   Spectrum C=A+B;
   Float rgb[3];
   C.ToRGB(rgb);
   EXPECT_EQ(rgb[0],1);
}
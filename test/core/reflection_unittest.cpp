#include "raiden.h"
#include "reflection.h"
//#include "transform.h"
#include "gtest.h"


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
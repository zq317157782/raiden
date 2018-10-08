#include "raiden.h"
#include "microfacet.h"
//#include "transform.h"
#include "gtest.h"


TEST(IsotropyBeckmann,D0){
   Float alpha = GGXRoughnessToAlpha(0);
   EXPECT_FLOAT_EQ(alpha,0.047269225);

   IsotropyBeckmannDistribution beckmann(GGXRoughnessToAlpha(0));
   Float d= beckmann.D(Vector3f(0,0,1));
   EXPECT_FLOAT_EQ(d, 142.46008);

   d = beckmann.D(Vector3f(1, 0, 0));
   EXPECT_FLOAT_EQ(d, 0);
}

TEST(IsotropyBeckmann, Sample) {
	Float alpha = GGXRoughnessToAlpha(0);
	EXPECT_FLOAT_EQ(alpha, 0.047269225);

	IsotropyBeckmannDistribution beckmann(GGXRoughnessToAlpha(0));
	Vector3f v=beckmann.Sample_wh(Vector3f(0,0,1),Point2f(0.5f,0));
	EXPECT_EQ(v.Length(), 1);
}
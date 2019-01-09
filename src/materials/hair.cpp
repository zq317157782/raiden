#include "hair.h"
#include "paramset.h"
HairMaterial* CreateHairMaterial(const TextureParams&mp){
    auto eta=mp.GetFloatTexture("eta",Float(1.55));
    auto betaM=mp.GetFloatTexture("betam",Float(1.0));
    auto betaN=mp.GetFloatTexture("betan",Float(1.0));
	return new HairMaterial(eta,betaM,betaN);
}
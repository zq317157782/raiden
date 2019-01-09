#include "hair.h"
#include "paramset.h"
HairMaterial* CreateHairMaterial(const TextureParams&mp){
    std::shared_ptr<Texture<Float>> betaM=mp.GetFloatTexture("betam",Float(0.0));
	return new HairMaterial(betaM);
}
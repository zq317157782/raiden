/*
 * grass.h
 *
 *  Created on: 2017年1月6日
 *      Author: zhuqian
 */

#ifndef SRC_MATERIALS_GRASS_H_
#define SRC_MATERIALS_GRASS_H_

#include "raiden.h"
#include "material.h"
#include "memory.h"
#include "interaction.h"
#include "reflection.h"
#include "texture.h"
class GrassMaterial : public Material {
private:
	std::shared_ptr<Texture<Spectrum>> _kr;
	std::shared_ptr<Texture<Spectrum>> _kt;
	std::shared_ptr<Texture<Float>> _eta;
public:

	GrassMaterial(const std::shared_ptr<Texture<Spectrum>>& kr, const std::shared_ptr<Texture<Spectrum>>& kt, const std::shared_ptr<Texture<Float>>& eta) :_kr(kr),_kt(kt), _eta(eta){
	}
	virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
		MemoryArena &arena, TransportMode mode,
		bool allowMultipleLobes) const override {
		//1.先为SurfaceInteraction分配bsdf
		Float eta = _eta->Evaluate(*si);
		Spectrum R = _kr->Evaluate(*si);
		Spectrum T = _kt->Evaluate(*si);
		si->bsdf = ARENA_ALLOC(arena, BSDF)(*si,eta);
		if (allowMultipleLobes) {
			si->bsdf->Add(ARENA_ALLOC(arena,FresnelSpecular)(R,T,1,eta,mode));
		}
		else {
			if (!R.IsBlack()) {
				si->bsdf->Add(ARENA_ALLOC(arena, SpecularReflection)(R, ARENA_ALLOC(arena, FresnelDielectric)(1.0,eta)));
			}

			if (!T.IsBlack()) {
				si->bsdf->Add(ARENA_ALLOC(arena, SpecularTransmission)(T,1.0,eta,mode));
			}
		}
		
	}
	virtual ~GrassMaterial() {};
};

GrassMaterial* CreateGrassMaterial(const TextureParams&mp);


#endif /* SRC_MATERIALS_GRASS_H_ */

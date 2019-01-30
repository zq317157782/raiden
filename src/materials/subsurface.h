/*
 * subsurface.h
 *
 *  Created on: 2019年1月30日
 *      Author: zhuqian
 */

#pragma once

#include "raiden.h"
#include "material.h"
#include "memory.h"
#include "interaction.h"
#include "reflection.h"
#include "texture.h"
#include "microfacet.h"
#include "bssrdf.h"
class SubsurfaceMaterial:public Material {
private:
    BSSRDFTable _table;
    std::shared_ptr<Texture<Spectrum>> _sigmaS;
    std::shared_ptr<Texture<Spectrum>> _sigmaA;
    Float _eta;
    Float _g;
public:
	SubsurfaceMaterial(const std::shared_ptr<Texture<Spectrum>>& sigmaS,const std::shared_ptr<Texture<Spectrum>>& sigmaA,Float eta,Float g):_table(100,64),_sigmaS(sigmaS),_sigmaA(sigmaA),_eta(eta),_g(g){
        ComputeBeamDiffusionBSSRDF(_g,_eta,&_table);
	}
	virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const override{
            Spectrum sigmaS=_sigmaS->Evaluate(*si);
            Spectrum sigmaA=_sigmaA->Evaluate(*si);
            //const SurfaceInteraction &po,Float eta,const Spectrum& sigmaS,const Spectrum& sigmaA,const BSSRDFTable& table
            si->bssrdf=ARENA_ALLOC(arena,TabulatedBSSRDF)(*si,_eta,sigmaS,sigmaA,_table);
	}
	virtual ~SubsurfaceMaterial(){};
};

SubsurfaceMaterial* CreateSubsurfaceMaterial(const TextureParams&mp);

/*
 * material.h
 *
 *  Created on: 2016年12月26日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_MATERIAL_H_
#define SRC_CORE_MATERIAL_H_
#include "raiden.h"
enum class TransportMode { Radiance, Importance };
//材质
class Material {
  public:
    virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const = 0;
    virtual ~Material();
};



#endif /* SRC_CORE_MATERIAL_H_ */

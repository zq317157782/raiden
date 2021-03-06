/*
 * api.h
 *
 *  Created on: 2016年12月16日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_API_H_
#define SRC_CORE_API_H_
#include "raiden.h"
#include "logging.h"

//初始化raiden渲染系统
void raidenInit(const Options &opt);
void raidenCleanup();
void raidenIdentity();
void raidenTranslate(Float dx, Float dy, Float dz);
void raidenTransform(Float tr[16]);
void raidenConcatTransform(Float tr[16]);
void raidenRotate(Float angle, Float dx, Float dy, Float dz);
void raidenScale(Float sx, Float sy, Float sz);
void raidenLookAt(Float ex, Float ey, Float ez, Float lx, Float ly, Float lz,Float ux, Float uy, Float uz);
void raidenCoordinateSystem(const std::string &name);
void raidenCoordSysTransform(const std::string &name);
void raidenActiveTransformAll();
void raidenActiveTransformEndTime();
void raidenActiveTransformStartTime();
void raidenTransformTimes(Float start, Float end);
void raidenPixelFilter(const std::string &name, const ParamSet &params);
void raidenFilm(const std::string &type, const ParamSet &params);
void raidenSampler(const std::string &name, const ParamSet &params);
void raidenAccelerator(const std::string &name, const ParamSet &params);
void raidenIntegrator(const std::string &name, const ParamSet &params);
void raidenCamera(const std::string &name, const ParamSet &params);
void raidenShape(const std::string &name, const ParamSet &params);
void raidenTexture(const std::string &name, const std::string &type,const std::string &texname, const ParamSet &params);
void raidenMaterial(const std::string &name, const ParamSet &params);
void raidenNamedMaterial(const std::string &name);
void raidenMakeNamedMaterial(const std::string& name, const ParamSet& params);
void raidenLightSource(const std::string& name, const ParamSet &params);
void raidenAreaLightSource(const std::string& name, const ParamSet &params);
void raidenMakeNamedMedium(const std::string &name, const ParamSet &params);
void raidenMediumInterface(const std::string &insideName, const std::string &outsideName);
void raidenWorldBegin();
void raidenWorldEnd();
void raidenAttributeBegin();
void raidenAttributeEnd();
void raidenTransformBegin();
void raidenTransformEnd();
void raidenReverseOrientation();


#endif /* SRC_CORE_API_H_ */

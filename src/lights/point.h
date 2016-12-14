/*
 * point.h
 *
 *  Created on: 2016年12月8日
 *      Author: zhuqian
 */

#ifndef SRC_LIGHTS_POINT_H_
#define SRC_LIGHTS_POINT_H_
#include "raiden.h"
#include "light.h"
//点光源
class PointLight:public Light{
private:
	Point3f _position;//点光源的位置，预计算，免得每次都要计算
	Spectrum _I;//光源强度，I，单位Watte/sr
public:
	PointLight(const Transform& l2w,const Spectrum& I):Light((int)LightFlags::DeltaPosition,l2w){
		_position=_lightToWorld(Point3f(0,0,0));//计算光源在世界坐标下的位置
		_I=I;
	}
	virtual Spectrum Sample_Li(Interaction& interaction,Vector3f* wi,Float* pdf) const override;
};




#endif /* SRC_LIGHTS_POINT_H_ */
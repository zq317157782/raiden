/*
 * primitive.h
 *
 *  Created on: 2016年11月24日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_PRIMITIVE_H_
#define SRC_CORE_PRIMITIVE_H_

#include "raiden.h"
#include "geometry.h"
//图元类
class Primitive{
public:
	//判断射线与图元是否相交的函数
	virtual bool Intersect(const Ray&,SurfaceInteraction*) const=0;
	virtual bool IntersectP(const Ray&) const =0;
	virtual ~Primitive(){};
	virtual Bound3f WorldBound() const=0;
	//todo 返回区域光结构
	//todo 计算散射
	//todo 获取材质
};

class GeomPrimitive:public Primitive{
private:
	std::shared_ptr<Shape> _shape;
	bool Intersect(const Ray&,SurfaceInteraction*) const override;
	Bound3f WorldBound() const override;
	bool IntersectP(const Ray&) const override;
public:
	GeomPrimitive(const std::shared_ptr<Shape>&,const std::shared_ptr<Material>&,const std::shared_ptr<AreaLight>&,const MediumInterface&);
};

#endif /* SRC_CORE_PRIMITIVE_H_ */

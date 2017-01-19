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
#include "interaction.h"
#include "material.h"
#include "medium.h"
//图元类
class Primitive {
public:
	//判断射线与图元是否相交的函数
	virtual bool Intersect(const Ray&, SurfaceInteraction*) const=0;
	virtual bool IntersectP(const Ray&) const =0;
	virtual ~Primitive() {
	}
	;
	virtual Bound3f WorldBound() const=0;
	virtual const AreaLight *GetAreaLight() const=0;
	virtual const Material *GetMaterial() const = 0;
	virtual void ComputeScatteringFunctions(SurfaceInteraction *isect,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const=0;
	//todo 返回区域光结构
};

class GeomPrimitive: public Primitive {
private:
	std::shared_ptr<Shape> _shape;
	std::shared_ptr<Material> _material;
	std::shared_ptr<AreaLight> _areaLight;
	MediumInterface _mediumInterface;
	bool Intersect(const Ray&, SurfaceInteraction*) const override;
	Bound3f WorldBound() const override;
	bool IntersectP(const Ray&) const override;
public:
	GeomPrimitive(const std::shared_ptr<Shape>&,
			const std::shared_ptr<Material>&,
			const std::shared_ptr<AreaLight>&,const MediumInterface& mediumInterface);

	virtual void ComputeScatteringFunctions(SurfaceInteraction *isect,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const override {
		if (_material) {
			_material->ComputeScatteringFunctions(isect, arena, mode,
					allowMultipleLobes);
		}
		//确定几何法线和着色法线在同一个半球中
		Assert(Dot(isect->n, isect->shading.n));
	}

	virtual const AreaLight *GetAreaLight() const override {
		return _areaLight.get();
	}
	virtual const Material *GetMaterial() const override {
		return _material.get();
	}
};

//空间图元集合
class Aggregate: public Primitive {
	virtual void ComputeScatteringFunctions(SurfaceInteraction *isect,
			MemoryArena &arena, TransportMode mode,
			bool allowMultipleLobes) const override {
		Warning("Aggregate::ComputeScatteringFunctions () method "
				"called; should have gone to GeometricPrimitive");
	}

	virtual const AreaLight *GetAreaLight() const override {
		Warning("Aggregate::GetAreaLight() method "
				"called; should have gone to GeometricPrimitive");
		return nullptr;
	}

	virtual const Material *GetMaterial() const override {
		Warning("Aggregate::GetMaterial() method "
			"called; should have gone to GeometricPrimitive");
		return nullptr;
	}
};

#endif /* SRC_CORE_PRIMITIVE_H_ */

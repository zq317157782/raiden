/*
 * primitive.cpp
 *
 *  Created on: 2016年11月24日
 *      Author: zhuqian
 */
#include "primitive.h"
#include "shape.h"
bool GeomPrimitive::Intersect(const Ray& r, SurfaceInteraction* i) const {
	Float hit;
	if (!_shape->Intersect(r, &hit, i)) { return false; }
	r.tMax = hit;
	i->primitive=this;//更新交点数据的图元指针
	
	//存在两种介质
	if (_mediumInterface.IsMediumTransition()) {
		i->mediumInterface = _mediumInterface;
	}
	//不存在，以射线所在的介质为主
	else {
		i->mediumInterface = MediumInterface(r.medium);
	}
	return true;
}

bool GeomPrimitive::IntersectP(const Ray& r) const{
	return _shape->IntersectP(r);
}

Bound3f GeomPrimitive::WorldBound() const{
	return _shape->WorldBound();
}


GeomPrimitive::GeomPrimitive(const std::shared_ptr<Shape>& s, const std::shared_ptr<Material>& m,const std::shared_ptr<AreaLight>& area, const MediumInterface& mediumInterface):_shape(s),_material(m),_areaLight(area),_mediumInterface(mediumInterface){

}

/*
 * scene.h
 *
 *  Created on: 2016年12月7日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_SCENE_H_
#define SRC_CORE_SCENE_H_
#include "raiden.h"
#include "primitive.h"
class Scene{
private:
	std::shared_ptr<Primitive> _aggregate;//图元集合
public:
	Scene(const std::shared_ptr<Primitive>&aggregate):_aggregate(aggregate){
		Assert(_aggregate!=nullptr);
	}
	bool Intersect(const Ray& ray,SurfaceInteraction* interaction) const;
	bool IntersectP(const Ray& ray) const;
};



#endif /* SRC_CORE_SCENE_H_ */

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
#include "light.h"
class Scene{
private:
	std::shared_ptr<Primitive> _aggregate;//图元集合
public:
	const std::vector<std::shared_ptr<Light>> lights;
public:
	Scene(const std::shared_ptr<Primitive>&aggregate,const std::vector<std::shared_ptr<Light>>&lights):_aggregate(aggregate),lights(lights){
		Assert(_aggregate!=nullptr);
	}
	bool Intersect(const Ray& ray,SurfaceInteraction* interaction) const;
	bool IntersectP(const Ray& ray) const;
};



#endif /* SRC_CORE_SCENE_H_ */

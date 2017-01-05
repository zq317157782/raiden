/*
 * iteration.h
 *
 *  Created on: 2016年12月16日
 *      Author: zhuqian
 */

#ifndef SRC_ACCELERATORS_ITERATION_H_
#define SRC_ACCELERATORS_ITERATION_H_
#include "raiden.h"
#include "primitive.h"

class Iteration: public Aggregate {
private:
	std::vector<std::shared_ptr<Primitive>> _primitives;
	Bound3f _worldBound;
public:
	Iteration(const std::vector<std::shared_ptr<Primitive>>& primitives) :
			_primitives(primitives) {
		for (auto& p : _primitives) {
			_worldBound=Union(_worldBound, p->WorldBound());
		}
		
		
	}
	bool Intersect(const Ray& r, SurfaceInteraction* ref) const override {
		bool flag = false;
		for (auto p : _primitives) {
			if (p->Intersect(r, ref)) {
				flag = true;
			}
		}
		return flag;
	}
	Bound3f WorldBound() const override {
		return _worldBound;
	}
	bool IntersectP(const Ray& r) const override {
		for (auto p : _primitives) {
			if (p->IntersectP(r)) {
				return true;
			}
		}
		return false;
	}
};

std::shared_ptr<Iteration> CreateIterationAccelerator(
    const std::vector<std::shared_ptr<Primitive>> &prims, const ParamSet &ps);
#endif /* SRC_ACCELERATORS_ITERATION_H_ */

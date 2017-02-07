/*
 * bvh.cpp
 *
 *  Created on: 2017年2月6日
 *      Author: zhuqian
 */

#include "bvh.h"
#include "paramset.h"

std::shared_ptr<BVHAccelerator> CreateBVHAccelerator(
	const std::vector<std::shared_ptr<Primitive>> &prims,
	const ParamSet &ps) {
	return std::make_shared<BVHAccelerator>(prims);
}



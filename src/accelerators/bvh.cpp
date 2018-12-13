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
	std::string splitMethodName = ps.FindOneString("splitmethod", "middle");
	BVHAccelerator::SplitMethod  splitMethod;
	if (splitMethodName == "middle") {
		splitMethod = BVHAccelerator::SplitMethod::MIDDLE;
	}
	else if (splitMethodName == "equalcount") {
		splitMethod = BVHAccelerator::SplitMethod::EQUAL_COUNT;
	}
	else if (splitMethodName == "sah") {
		splitMethod = BVHAccelerator::SplitMethod::SAH;
	}
	else {
		splitMethod = BVHAccelerator::SplitMethod::SAH;
		LWarning<<"SplitMethod:" << splitMethodName << " is unknown, using SAH Method.";
	}
	Debug("[CreateBVHAccelerator][SplitMethod:"<< splitMethodName <<"]");
	return std::make_shared<BVHAccelerator>(prims, splitMethod);
}



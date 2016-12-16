/*
 * iteration.cpp
 *
 *  Created on: 2016年12月16日
 *      Author: zhuqian
 */

#include "iteration.h"

std::shared_ptr<Iteration> CreateIterationAccelerator(
    const std::vector<std::shared_ptr<Primitive>> &prims, const ParamSet &ps) {
    return std::make_shared<Iteration>(prims);
}



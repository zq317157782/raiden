/*
 * grid.cpp
 *
 *  Created on: 2017年1月12日
 *      Author: zhuqian
 */
#include "grid.h"
#include "paramset.h"

std::shared_ptr<Grid> CreateGridAccelerator(
    const std::vector<std::shared_ptr<Primitive>> &prims, const ParamSet &ps) {
    int maxWidth=ps.FindOneInt("maxwidth",4);
	return std::make_shared<Grid>(prims,maxWidth);
}




/*
 * box.cpp
 *
 *  Created on: 2016年12月5日
 *      Author: zhuqian
 */
#include "box.h"
#include "paramset.h"
BoxFilter *CreateBoxFilter(const ParamSet &ps) {
    Float xw = ps.FindOneFloat("xwidth", 0.5f);
    Float yw = ps.FindOneFloat("ywidth", 0.5f);
    return new BoxFilter(Vector2f(xw, yw));
}




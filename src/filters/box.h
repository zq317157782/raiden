/*
 * box.h
 *
 *  Created on: 2016年12月5日
 *      Author: zhuqian
 */

#ifndef SRC_FILTERS_BOX_H_
#define SRC_FILTERS_BOX_H_
#include "filter.h"
//盒形过滤器,最差过滤器，没有之一
//频域下是sinc函数
class BoxFilter:public Filter{
public:
	BoxFilter(const Vector2f& r):Filter(r){
	}
	Float Evaluate(const Point2f& point) const override{
		//这里假设上层用户已经检查了range,所以不做range检查
		return 1.0f;
	}
};

//参数 {xwidth:Float,ywidth:Float}
BoxFilter *CreateBoxFilter(const ParamSet &ps);

#endif /* SRC_FILTERS_BOX_H_ */

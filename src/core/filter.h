/*
 * filter.h
 *
 *  Created on: 2016年12月5日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_FILTER_H_
#define SRC_CORE_FILTER_H_
#include "raiden.h"
//过滤器
class Filter {
private:
	Vector2f _radius;//filter的作用域
	Vector2f _invRadius;
public:
	/*返回 point处的 filter权重*/
	virtual Float Evaluate(const Point2f& point) const=0;
	Filter(const Vector2f& radius):_radius(radius),_invRadius(Vector2f(1.0f/radius.x,1.0f/radius.y)){
		Assert(radius.x>0||radius.y>0);
	}
	virtual ~Filter(){}
};

#endif /* SRC_CORE_FILTER_H_ */

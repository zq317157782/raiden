/*
 * lightdistrib.h
 *
 *  Created on: 2017年5月3日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_LIGHTDISTRIB_H_
#define SRC_CORE_LIGHTDISTRIB_H_
//光源分布相关
#include "raiden.h"

//非常general的光源分布类
class LightDistribution{
public:
	//返回空间中相应点的合适的高效的分布
	virtual const Distribution1D* Lookup(const Point3f& p) const=0;
};


//均匀分布
class UniformLightDistribution{
private:
	std::unique_ptr<Distribution1D> _distribution;//相应的分布
public:
	UniformLightDistribution(const Scene& scene){
		std::vector<Float> weights(scene.lights.size(),1.0f);
		//std::unique_ptr要用reset,记住！！！
		_distribution.reset(new Distribution1D(&weights[0],weights.size()));
	}

	const Distribution1D* Lookup(const Point3f& p) const override{
		return _distribution.get();
	}
};



#endif /* SRC_CORE_LIGHTDISTRIB_H_ */

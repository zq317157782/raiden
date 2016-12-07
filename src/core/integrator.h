/*
 * integrator.h
 *
 *  Created on: 2016年12月7日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_INTEGRATOR_H_
#define SRC_CORE_INTEGRATOR_H_
#include "raiden.h"
#include "geometry.h"
class Integrator{
public:
	virtual void RenderScene(const Scene&)=0;
	virtual ~Integrator(){}
};

class SamplerIntegrator:public Integrator{
private:
	std::shared_ptr<Sampler> _sampler;//采样器
	Bound2i _pixelBound;
protected:
	std::shared_ptr<const Camera> _camera;
public:
	SamplerIntegrator(const std::shared_ptr<const Camera>& camera,const std::shared_ptr<Sampler>& sampler,const Bound2i&pixelBound):
		_sampler(sampler),_camera(camera),_pixelBound(pixelBound){

	}
	virtual void RenderScene(const Scene&) override;
};


#endif /* SRC_CORE_INTEGRATOR_H_ */

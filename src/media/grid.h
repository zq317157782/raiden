/*
* grid.h
*
*  Created on: 2018年1月17日
*      Author: zhuqian
*/


#include "raiden.h"
#include "medium.h"
#include "interaction.h"
#include "sampler.h"
#include "memory.h"


//非同质介质
class GridDensityMedium:public Medium {
private:
    Spectrum _sigma_a;
    Spectrum _sigma_s;
    Spectrum _sigma_t;
    int _nx,_ny,_nz;
    Float* _density;
	//从世界坐标系到介质坐标系的转换
	Transform _worldToMedium;

	//传入的是介质坐标的离散点
	//Float DiscreteDensity(const Point3i& p) const {
	Float D(const Point3i& p) const {
		//获得索引
		int index = p.z*_nx*_ny + p.y*_nx + p.x;

		if (index >= _nx*_ny*_nz) {
			return 0;
		}

		return _density[index];
	}

	//传入的是样本空间的点
	Float Density(const Point3f& p) const {
		Point3f samplePoint(p.x*_nx - 0.5f, p.y*_ny - 0.5f, p.z*_nz - 0.5f);
		Point3i iPoint = (Point3i)Floor(samplePoint);//获得离散坐标
		Vector3f delta = samplePoint - (Point3f)iPoint;

		//做Trilinear插值
		//先考虑只插值X
		Float d00 = Lerp(delta.x, D(iPoint),                    D(iPoint + Point3i(1,0,0)));
		Float d01 = Lerp(delta.x, D(iPoint + Point3i(0, 1, 0)), D(iPoint + Point3i(1, 1, 0)));
		Float d10 = Lerp(delta.x, D(iPoint + Point3i(0, 0, 1)), D(iPoint + Point3i(1, 0, 1)));
		Float d11 = Lerp(delta.x, D(iPoint + Point3i(0, 1, 1)), D(iPoint + Point3i(1, 1, 1)));

		//再插值Y
		Float d0  = Lerp(delta.y, d00, d01);
		Float d1  = Lerp(delta.y, d10, d11);
		
		//再插值Z
		Float d   =  Lerp(delta.z, d0, d1);

		//返回插值的值
		return d;
	}

public:
    GridDensityMedium(const Spectrum& sigma_a,const Spectrum& sigma_s,int nx,int ny,int nz,Float* density):_sigma_a(sigma_a),_sigma_s(sigma_s),
    _sigma_t(sigma_a+sigma_s),_nx(nx),_ny(ny),_nz(nz),_density(new Float[nx*ny*nz]){
		//复制一份密度数据
		memcpy(_density, density, nx*ny*nz);
    }

    ~GridDensityMedium(){
        if(_density!=nullptr){
            delete [] _density;
        }
    }



	//返回期望值等于透射值的随机变量
	//包含pdf在内部
	//这里使用的是ratio tracking
	Spectrum Tr(const Ray &ray, Sampler &sampler) const override {
		
	}
	

	//包含pdf在内部
	//使用delta tracking
	Spectrum Sample(const Ray &ray, Sampler &sampler,
		MemoryArena &arena,
		MediumInteraction *mi) const override {
	}

};
 
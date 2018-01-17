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
    Float _sigma_t;
    int _nx,_ny,_nz;
    Float* _density;
	Float _g;
	//从世界坐标系到介质坐标系的转换
	Transform _worldToMedium;
	
	Float _invMaxDensity;
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
    GridDensityMedium(const Spectrum& sigma_a,const Spectrum& sigma_s,Float g,int nx,int ny,int nz,Float* density):_sigma_a(sigma_a),_sigma_s(sigma_s),_g(g),_nx(nx),_ny(ny),_nz(nz),_density(new Float[nx*ny*nz]){
		//delta tracking默认所有的ext的通道都是一样的值(Uniform)
		_sigma_t=(sigma_a+sigma_s)[0];
		//复制一份密度数据
		memcpy(_density, density, sizeof(Float)*nx*ny*nz);
		//计算最大的密度的倒数
		Float maxDensity=0;
		for(int i=0;i<nx*ny*nz;++i){
			maxDensity=std::max(maxDensity,_density[i]);
		}
		_invMaxDensity=1.0/maxDensity;
    }

    ~GridDensityMedium(){
        if(_density!=nullptr){
            delete [] _density;
        }
    }


	//返回期望值等于透射值的随机变量
	//包含pdf在内部
	//这里使用的是ratio tracking
	Spectrum Tr(const Ray &rayW, Sampler &sampler) const override {
		//先获取Medium空间的射线
		Ray ray=_worldToMedium(Ray(rayW.o,Normalize(rayW.d),rayW.tMax*rayW.d.Length()));
		
		//判断射线是否在有效区域内
		const Bound3f b(Point3f(0,0,0),Point3f(1,1,0));
		Float tMin,tMax;
		if(!b.IntersectP(ray,&tMin,&tMax)){
			//采样到表面
			//这里f(x)/pdf(x)正好等于1
			return Spectrum(1.0);
		}

		//ratio tracking
		Float tr=1.0;
		Float t=tMin;
		while(true){
			//1.首先使用同质的方法来采样t
			//PBRT这里是td::log(1-sampler.Get1DSample()),我简化了
			//invert method
			t-=std::log(sampler.Get1DSample())*_invMaxDensity/_sigma_t;
			if(t>=tMax){
				break;
			}
			//2.使用继续往下采样的概率(未射中real的概率)来更新透射率
			Float density=Density(ray(t));
			tr=tr*(1-std::max((Float)0,density*_invMaxDensity));

			//PBRT后来在打印版之后，有加入了俄罗斯罗盘的代码
			//使得在透射率低的时候，有一定几率结束采样
			constexpr Float rrThreshold=0.1;
			if(tr<rrThreshold){
				//终止概率
				Float q=std::max((Float)0.05,1-tr);
				if(sampler.Get1DSample()<q){
					return 0;
				}
				//补偿
				tr=tr/(1-q);
			}
		}
		return tr;
	}
	

	//包含pdf在内部
	//使用delta tracking
	Spectrum Sample(const Ray &rayW, Sampler &sampler,
		MemoryArena &arena,
		MediumInteraction *mi) const override {
		//先获取Medium空间的射线
		Ray ray=_worldToMedium(Ray(rayW.o,Normalize(rayW.d),rayW.tMax*rayW.d.Length()));
		
		//判断射线是否在有效区域内
		const Bound3f b(Point3f(0,0,0),Point3f(1,1,0));
		Float tMin,tMax;
		if(!b.IntersectP(ray,&tMin,&tMax)){
			//采样到表面
			//这里f(x)/pdf(x)正好等于1
			return Spectrum(1.0);
		}

		//delta tracking算法
		Float t=tMin;
		while(true){
			//1.首先使用同质的方法来采样t
			//PBRT这里是td::log(1-sampler.Get1DSample()),我简化了
			//invert method
			t-=std::log(sampler.Get1DSample())*_invMaxDensity/_sigma_t;
			if(t>=tMax){
				break;
			}
			//2.然后判断t相交的是否是real粒子
			if(sampler.Get1DSample()<=(Density(ray(t)))*_invMaxDensity){
				//是real的时候，返回交点
				PhaseFunction* phase=ARENA_ALLOC(arena,HenyeyGreenstein)(_g);
				*mi=MediumInteraction(rayW(t),-rayW.d,rayW.time,this,phase);
				//这里同样约分了部分数据
				return _sigma_s/_sigma_t;
			}
		}
		//表面情况
		return Spectrum(1.0);
	}

};
 
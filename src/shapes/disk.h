#pragma once
#include "shape.h"
#include "errfloat.h"
class Disk : public Shape
{
    private:
        Float _height;//圆盘的高度
        Float _radius;//disk的外半径
        Float _innerRadius;//disk的内半径
        Float _phiMax;//disk绕Z轴的最大phi值
    public:

    Disk(const Transform *objectToWorld, const Transform *worldToObject,
		   bool reverseOrientation,Float height,Float radius,Float innerRadius,Float phiMax)
		: Shape(objectToWorld, worldToObject, reverseOrientation),_height(height),_radius(radius),_innerRadius(innerRadius)
        ,_phiMax(Radians(Clamp(phiMax, 0, 360)))
	{
        Assert(radius>innerRadius);
        Assert(innerRadius>=0);
	}

    //disk的包围盒是个2维的，Z轴重叠了
    virtual Bound3f ObjectBound() const override{
        Point3f minP(-_radius,-_radius,_height);
        Point3f maxP(_radius,_radius,_height);
        return Bound3f(minP,maxP);
    }

    virtual Float Area() const override{
        return _phiMax/2*(_radius*_radius-_innerRadius*_innerRadius);
    }

    
    virtual bool Intersect(const Ray& ray,Float* tHit,SurfaceInteraction* surfaceInsect,bool testAlpha=true) const override{
        //首先变换空间
        Vector3f oErr, dErr;
	    Ray oRay = (*worldToObject)(ray, &oErr, &dErr);
        //判断是否和disk平行
        if(oRay.d.z==0){
            return false;
        }
        Float t=(_height-oRay.o.z)/oRay.d.z;
        //判断t是否在有效范围内
        if(t<0||t>oRay.tMax){
            return false;
        }
        //获取交点
        auto hit=oRay(t);
        //把相交点投影到disk所在的plane上
        hit.z=_height;

        Float dist2=hit.x*hit.x+hit.y*hit.y;
        if(dist2>(_radius*_radius)||dist2<(_innerRadius*_innerRadius)){
            return false;
        }

        Float phi=std::atan2(hit.y,hit.x);
        if(phi<0){
            phi+=2*Pi;
        }
        if(phi>_phiMax){
            return false;
        }

        Float u=phi/_phiMax;
        Float r=std::sqrt(dist2);
        //从外到内为[0-1]
        Float v=(_radius-r)/(_radius-_innerRadius);
        //从外到内为[1-0]
        //Float v=(r-_innerRadius)/(_radius-_innerRadius);

        //disk:
        //x = cos(phi)*r
        //y = sin(phi)*r
        //z = z;
        //  u = phi/phi_max
        //=>phi = u*phi_max
        //  v = (ro-r)/(ro-ri)
        //=>r = ro-v*(ro-ri)
        //dxdu = dcos(phi)du*r = dcos(u*phi_max)du*r = -phi_max*sin(phi)*r
        //dydu = dsin(phi)du*r = dsin(u*phi_max)du*r =  phi_max*cos(phi)*r
        //dzdu = 0
        //dxdv = cos(phi)*drdv = cos(phi)*(ri-ro)
        //dydv = sin(phi)*drdv = sin(phi)*(ri-ro);
        //dzdv = 0;
        Vector3f dpdu(-_phiMax*hit.y,_phiMax*hit.x,0);
        //TODO 和PBRT的结果有差异
        Vector3f dpdv(hit.x*(_innerRadius-_radius),hit.y*(_innerRadius-_radius),0.0);
        //因为所有的法线都是相同的，所以法线的所以偏差都为0
        Normal3f dndu(0,0,0);
        Normal3f dndv(0,0,0);

        Vector3f pError(0,0,0);
        (*surfaceInsect)=(*objectToWorld)(SurfaceInteraction(hit,pError,Point2f(u,v),-ray.d,dpdu,dpdv,dndu,dndv,ray.time,this));
        return true;
    }

    virtual bool IntersectP(const Ray& ray,bool testAlpha=true)const override {
         Vector3f oErr, dErr;
	    Ray oRay = (*worldToObject)(ray, &oErr, &dErr);
        //判断是否和disk平行
        if(oRay.d.z==0){
            return false;
        }
        Float t=(_height-oRay.o.z)/oRay.d.z;
        //判断t是否在有效范围内
        if(t<0||t>oRay.tMax){
            return false;
        }
        //获取交点
        auto hit=oRay(t);

        Float dist2=hit.x*hit.x+hit.y*hit.y;
        if(dist2>(_radius*_radius)||dist2<(_innerRadius*_innerRadius)){
            return false;
        }

        Float phi=std::atan2(hit.y,hit.x);
        if(phi<0){
            phi+=2*Pi;
        }
        if(phi>_phiMax){
            return false;
        }
        return true;
    }

    virtual Interaction Sample(const Point2f& uv,Float *pdf) const override{
        LError<<"Disk::Sample is not implemented!";
        Assert(false);
       return Interaction();
    }

    virtual Interaction Sample(const Interaction& ref,const Point2f& u, Float *pdf) const override{
         LError<<"Disk::Sample is not implemented!";
         Assert(false);
         return Interaction();
    }

};

std::shared_ptr<Shape> CreateDiskShape(const Transform *o2w,
                                         const Transform *w2o,
                                         bool reverseOrientation,
                                         const ParamSet &params);
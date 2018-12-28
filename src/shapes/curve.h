#pragma once
#include "shape.h"
#include "bezier.h"
//曲线的类型
enum class CurveType{FLAT};

//完整的曲线
struct CurveCommon{
public:
    const CubicBezier bezier;
    const Float width[2];
    const CurveType type;
public:
    CurveCommon(const CubicBezier& bezier,Float w0,Float w1,CurveType type):bezier(bezier),width{w0,w1},type(type){}
    CurveCommon(const Point3f p[4],Float w0,Float w1,CurveType type):bezier(CubicBezier(p)),width{w0,w1},type(type){}
    CurveCommon(const Point3f& p0,const Point3f& p1,const Point3f& p2,const Point3f& p3,Float w0,Float w1,CurveType type):bezier(CubicBezier(p0,p1,p2,p3)),width{w0,w1},type(type){}
};

class Curve : public Shape
{
    private:
       const std::shared_ptr<CurveCommon> _common;
       const Float _uMin;
       const Float _uMax;
    public:

    Curve(const Transform *objectToWorld, const Transform *worldToObject,
		   bool reverseOrientation,const std::shared_ptr<CurveCommon>& common,Float uMin,Float uMax)
		:Shape(objectToWorld, worldToObject, reverseOrientation),_common(common),_uMin(uMin),_uMax(uMax)
	{
        Assert(_uMin>=0);
        Assert(_uMax<=1);
	}

     virtual Bound3f ObjectBound() const override{
        //先计算当前曲线的4个控制点
        Point3f cp[4]={Blossom(_common->bezier,_uMin,_uMin,_uMin),Blossom(_common->bezier,_uMin,_uMin,_uMax),Blossom(_common->bezier,_uMin,_uMax,_uMax),Blossom(_common->bezier,_uMax,_uMax,_uMax)};
        //计算不考虑width情况下的Bound
        auto bound=Union(Bound3f(cp[0],cp[1]),Bound3f(cp[2],cp[3]));
        //计算参数范围内最大的宽度
        Float w0=Lerp(_uMin,_common->width[0],_common->width[1]);
        Float w1=Lerp(_uMax,_common->width[0],_common->width[1]);
        Float w= std::max(w0,w1);
        //扩大bound,获得保守的bound
        bound=Expand(bound,w*(Float)0.5);
        return bound;
     }

    virtual Float Area() const override{
        Assert(false);
        return 0;
    }

    
    virtual bool Intersect(const Ray& ray,Float* tHit,SurfaceInteraction* surfaceInsect,bool testAlpha=true) const override{
        return false;
    }

    virtual bool IntersectP(const Ray& ray,bool testAlpha=true)const override {
       return false;
    }

    virtual Interaction Sample(const Point2f& uv,Float *pdf) const override{
        LError<<"Curve::Sample is not implemented!";
        Assert(false);
       return Interaction();
    }

    virtual Interaction Sample(const Interaction& ref,const Point2f& u, Float *pdf) const override{
         LError<<"Curve::Sample is not implemented!";
         Assert(false);
         return Interaction();
    }

};
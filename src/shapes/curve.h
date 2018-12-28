#pragma once
#include "shape.h"
#include "bezier.h"
//曲线的类型
enum class CurveType
{
    FLAT
};

//完整的曲线
struct CurveCommon
{
  public:
    const CubicBezier bezier;
    const Float width[2];
    const CurveType type;

  public:
    CurveCommon(const CubicBezier &bezier, Float w0, Float w1, CurveType type) : bezier(bezier), width{w0, w1}, type(type) {}
    CurveCommon(const Point3f p[4], Float w0, Float w1, CurveType type) : bezier(CubicBezier(p)), width{w0, w1}, type(type) {}
    CurveCommon(const Point3f &p0, const Point3f &p1, const Point3f &p2, const Point3f &p3, Float w0, Float w1, CurveType type) : bezier(CubicBezier(p0, p1, p2, p3)), width{w0, w1}, type(type) {}
};

class Curve : public Shape
{
  private:
    const std::shared_ptr<CurveCommon> _common;
    const Float _uMin;
    const Float _uMax;

    bool RecursiveIntersect(const Ray& ray,Float *tHit, SurfaceInteraction *surfaceInsect,const Point3f cp[4],const Transform& rayToObject,Float u0,Float u1,int depth) const{
        return false;
    }
  public:
    Curve(const Transform *objectToWorld, const Transform *worldToObject,
          bool reverseOrientation, const std::shared_ptr<CurveCommon> &common, Float uMin, Float uMax)
        : Shape(objectToWorld, worldToObject, reverseOrientation), _common(common), _uMin(uMin), _uMax(uMax)
    {
        Assert(_uMin >= 0);
        Assert(_uMax <= 1);
    }

    virtual Bound3f ObjectBound() const override
    {
        //先计算当前曲线的4个控制点
        Point3f cp[4] = {Blossom(_common->bezier, _uMin, _uMin, _uMin), Blossom(_common->bezier, _uMin, _uMin, _uMax), Blossom(_common->bezier, _uMin, _uMax, _uMax), Blossom(_common->bezier, _uMax, _uMax, _uMax)};
        //计算不考虑width情况下的Bound
        auto bound = Union(Bound3f(cp[0], cp[1]), Bound3f(cp[2], cp[3]));
        //计算参数范围内最大的宽度
        Float w0 = Lerp(_uMin, _common->width[0], _common->width[1]);
        Float w1 = Lerp(_uMax, _common->width[0], _common->width[1]);
        Float w = std::max(w0, w1);
        //扩大bound,获得保守的bound
        bound = Expand(bound, w * (Float)0.5);
        return bound;
    }

    virtual Float Area() const override
    {
        Assert(false);
        return 0;
    }

    virtual bool Intersect(const Ray &r, Float *tHit, SurfaceInteraction *surfaceInsect, bool testAlpha = true) const override
    {
        Vector3f oErr, dErr;
        //变换Ray到Curve的局部空间
        Ray ray = (*worldToObject)(r, &oErr, &dErr);

        //获得当前射线的局部控制点
        Point3f cpObj[4] = {Blossom(_common->bezier, _uMin, _uMin, _uMin), Blossom(_common->bezier, _uMin, _uMin, _uMax), Blossom(_common->bezier, _uMin, _uMax, _uMax), Blossom(_common->bezier, _uMax, _uMax, _uMax)};

        //获得射线局部空间到Curve局部空间的矩阵
        Vector3f dx, dy;
        CoordinateSystem(ray.d, &dx, &dy);                        //计算正交坐标系
        Transform objectToRay = LookAt(ray.o, ray.o + ray.d, dx); //dy也可以，只要和ray.d正交
        //把控制点变换到ray空间
        Point3f cp[4] = {objectToRay(cpObj[0]), objectToRay(cpObj[1]), objectToRay(cpObj[2]), objectToRay(cpObj[3])};

        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //以下代码直接复制自PBRT，
        //PBRT没有解释具体算法的含义,
        //本段代码的作用是计算优化过的最大迭代深度，用来迭代计算和Curve的交点
        Float L0 = 0;
        for (int i = 0; i < 2; ++i)
            L0 = std::max(L0,
                          std::max(std::max(std::abs(cp[i].x - 2 * cp[i + 1].x + cp[i + 2].x),
                                            std::abs(cp[i].y - 2 * cp[i + 1].y + cp[i + 2].y)),
                                   std::abs(cp[i].z - 2 * cp[i + 1].z + cp[i + 2].z)));
        Float eps = std::max(_common->width[0], _common->width[1]) * (Float)0.05; // width / 20
#define LOG4(x) (std::log(x) * 0.7213475108f)
        Float fr0 = LOG4(1.41421356237f * 12.f * L0 / (8.f * eps));
#undef LOG4
        int r0 = (int)std::round(fr0);
        int maxDepth = Clamp(r0, 0, 10);
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        return RecursiveIntersect(ray,tHit,surfaceInsect,cp,Inverse(objectToRay),_uMin,_uMax,maxDepth);
    }

    virtual bool IntersectP(const Ray &ray, bool testAlpha = true) const override
    {
        return false;
    }

    virtual Interaction Sample(const Point2f &uv, Float *pdf) const override
    {
        LError << "Curve::Sample is not implemented!";
        Assert(false);
        return Interaction();
    }

    virtual Interaction Sample(const Interaction &ref, const Point2f &u, Float *pdf) const override
    {
        LError << "Curve::Sample is not implemented!";
        Assert(false);
        return Interaction();
    }
};

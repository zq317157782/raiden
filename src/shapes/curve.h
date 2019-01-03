#pragma once
#include "shape.h"
#include "bezier.h"
//曲线的类型
enum class CurveType
{
    FLAT,CYLINDER,RIBBON
};

//完整的曲线
struct CurveCommon
{
  public:
    const CubicBezier bezier;
    const Float width[2];
    const CurveType type;
    //RIBBON类型需要用到的数据
    Normal3f n[2];
    Float angle;
    Float invSin;
  public:
    CurveCommon(const CubicBezier &bezier, Float w0, Float w1, CurveType type,const Normal3f* norm) : bezier(bezier), width{w0, w1}, type(type) {
        if(norm){
            n[0]=Normalize(norm[0]);
            n[1]=Normalize(norm[1]);
            angle=std::acos(Clamp(Dot(n[0],n[1]),0,1));
            invSin=1/std::sin(angle);
        }
    }
    CurveCommon(const Point3f p[4], Float w0, Float w1, CurveType type,const Normal3f* norm) : bezier(CubicBezier(p)), width{w0, w1}, type(type) {
        if(norm){
            n[0]=Normalize(norm[0]);
            n[1]=Normalize(norm[1]);
            angle=std::acos(Clamp(Dot(n[0],n[1]),0,1));
            invSin=1/std::sin(angle);
        }
    }
    CurveCommon(const Point3f &p0, const Point3f &p1, const Point3f &p2, const Point3f &p3, Float w0, Float w1, CurveType type,const Normal3f* norm) : bezier(CubicBezier(p0, p1, p2, p3)), width{w0, w1}, type(type) {
        if(norm){
            n[0]=Normalize(norm[0]);
            n[1]=Normalize(norm[1]);
            angle=std::acos(Clamp(Dot(n[0],n[1]),0,1));
            invSin=1/std::sin(angle);
        }
    } 
};

class Curve : public Shape
{
  private:
    const std::shared_ptr<CurveCommon> _common;
    const Float _uMin;
    const Float _uMax;

    static void SubdivideBezier(const Point3f cp[4],Point3f newcp[7]){
        //Blossom
        // a0=(1-u0)*p0+u0*p1;
        // a1=(1-u0)*p1+u0*p2;
        // a2=(1-u0)*p2+u0*p3;
        // b0=(1-u1)*a0+u1*a1;
        // b1=(1-u1)*a1+u1*a2;
        // c =(1-u2)*b0+u2*b1;
        // c =(1-u2)*((1-u1)*((1-u0)*p0+u0*p1)+u1*((1-u0)*p1+u0*p2))+u2*((1-u1)*((1-u0)*p1+u0*p2)+u1*((1-u0)*p2+u0*p3));

        // np0 = p0 [u0:0 u1:0 u2:0]
        // np1 = p0/2 + p1/2    [u0:0 u1:0 u2:1/2]
        // np2 = p0/4 + p1/4 + p1/4 + p2/4 [u0:0 u1:1/2 u2:1/2]
        // np3 = p0/8 + p1/8 + p1/8 + p2/8 + p1/8 + p2/8 + p2/8 + p3/8[u0:1/2 u1:1/2 u2:1/2]
        // np4 = ... [u0:1/2 u1:1/2 u2:1]
        // np5 = ... [u0:1/2 u1:1 u2:1]
        // np6 = ... [u0:1 u1:1 u2:1]

        newcp[0] = cp[0];
        newcp[1] = (cp[0] + cp[1]) / 2;
        newcp[2] = (cp[0] + 2 * cp[1] + cp[2]) / 4;
        newcp[3] = (cp[0] + 3 * cp[1] + 3 * cp[2] + cp[3]) / 8;
        newcp[4] = (cp[1] + 2 * cp[2] + cp[3]) / 4;
        newcp[5] = (cp[2] + cp[3]) / 2;
        newcp[6] = cp[3];
    }
    
    //计算在Bezier曲线上的值
    static Point3f EvalBezier(const Point3f cp[4],Float u,Vector3f* deriv=nullptr){
        
        Point3f a0=Lerp(u,cp[0],cp[1]);
        Point3f a1=Lerp(u,cp[1],cp[2]);
        Point3f a2=Lerp(u,cp[2],cp[3]);

        Point3f b0=Lerp(u,a0,a1);
        Point3f b1=Lerp(u,a1,a2);
        //计算导数
        if (deriv){
            *deriv = (Float)3 * (b1 - b0);
        }
        return Lerp(u,b0,b1);
    }

    bool RecursiveIntersect(const Ray& ray,Float *tHit, SurfaceInteraction *is,const Point3f cp[4],const Transform& rayToObject,Float u0,Float u1,int depth) const{
        //计算Ray空间的curve bound
        auto curveBound = Union(Bound3f(cp[0], cp[1]), Bound3f(cp[2], cp[3]));
        //计算最大的width
        Float w0 = Lerp(u0, _common->width[0], _common->width[1]);
        Float w1 = Lerp(u1, _common->width[0], _common->width[1]);
        Float width = std::max(w0, w1);
        //扩大bound,获得保守的bound
        curveBound = Expand(curveBound, width * (Float)0.5);

        //计算Ray空间下的Ray的bound
        Float rayLen=ray.d.Length();
        Float zMax=rayLen*ray.tMax;
        Bound3f rayBound(Point3f(0,0,0),Point3f(0,0,zMax));

        //判断rayBound和curveBound是否相交
        if(Overlap(rayBound,curveBound)==false){
            return false;
        }
        //到这里，已经和现在的bound相交了，所以要开始迭代的判断子curve是否相交，
        //直到深度为0的时候，再判断是都和实际的curve相交
        if(depth>0){
            //迭代
            Float uMid=(u0+u1)/2;
            Point3f newcp[7];
            SubdivideBezier(cp,newcp);
            return RecursiveIntersect(ray,tHit,is,&newcp[0],rayToObject,u0,uMid,depth-1)||RecursiveIntersect(ray,tHit,is,&newcp[3],rayToObject,uMid,u1,depth-1);
        }
        else{
            //相交测试
            //首先计算垂直于start point和end point的直线 
            //1.先算出Bezier曲线的一阶导，并且获得相应点处的切线方程
            //2.因为已经在ray空间了，所以相当于在处理2D空间的向量，因此假设切线t为[x,y],则垂直于切线的向量为b[y,-x]
            //3.获得这个向量后，就可以计算edge function

            //start point处的1阶导为3(p1-p0)=> 3[p1.x-p0.x,p1.y-p0.y]
            //所以垂直向量为 [p1.y-p0.y,p0.x-p1.x]
            //所以第二个点为 p0+[p1.y-p0.y,p0.x-p1.x] =>[p0.x+p1.y-p0.y,p0.y+p0.x-p1.x]

            //所以edge function为:(p1.y-p0.y)(p.y-p0.y)-(p.x-p0.x)(p0.x-p1.x)  
            //因为在ray空间中，p为[0,0],所以简化成 (p1.y-p0.y)(-p0.y)+(p0.x)(p0.x-p1.x)  
            Float edge=(cp[1].y-cp[0].y)*(-cp[0].y)+(cp[0].x)*(cp[0].x-cp[1].x);
            if(edge<0){
                //(0,0)点在edge的左边，所以不能和曲线相交
                return false;
            }
            //这里是相应的判断end point处的edge function
            //省略
            edge = (cp[2].y - cp[3].y) * (-cp[3].y) + (cp[3].x) * (cp[3].x - cp[2].x);
            if(edge<0){
                //(0,0)点在edge的左边，所以不能和曲线相交
                return false;
            }

            //使用start point和end point组成的直线来近似Bezier Curve
            Vector2f segmentDir=Point2f(cp[3])-Point2f(cp[0]);
            Float denom=segmentDir.LengthSquared();
            if(denom==0){
                //退化的情况
                return false;
            }
            Float w=Dot(-Vector2f(cp[0]),segmentDir)/denom;//直线上的参数
            //计算近似交点的宽度
            Float u=Clamp(Lerp(w,u0,u1),u0,u1);
            Float hitWidth=Lerp(u,_common->width[0],_common->width[1]);

            Normal3f nHit;
            //如果是Ribbon类型的,需要处理法线和width的缩放
            if(_common->type==CurveType::RIBBON){
                //Spherical Interplate
                //参考四元数
                Float sin0=std::sin((1-u)*_common->angle)*_common->invSin;
                Float sin1=std::sin(u*_common->angle)*_common->invSin;
                nHit=sin0*_common->n[0]+sin1*_common->n[1];
                //根据法线和射线的夹角值，缩放width
                hitWidth=hitWidth*(AbsDot(nHit,ray.d)/rayLen);
            }

            //计算曲线上的点以及相应的偏导
            Vector3f dpcdw;
            auto pc=EvalBezier(cp,Clamp(w,0,1),&dpcdw);
            Float len2=pc.x*pc.x+pc.y*pc.y;//计算到(0,0)的距离
            if(len2>(hitWidth*hitWidth*(Float)0.25)){
                return false;
            }
            //判断z轴
            if(pc.z<0||pc.z>zMax){
                return false;
            }
            

            //计算参数v
             //计算实际的距离
            Float len=std::sqrt(len2);
            //判断交点是在curve的左边还是右边
            //使用edge function
            edge=(dpcdw.x)*(-pc.y)+(pc.x)*(dpcdw.y);
            Float v;
            if(edge<0){
                v=(Float)0.5+len/hitWidth;
            }
            else {
                v=(Float)0.5-len/hitWidth;
            }
            

            Vector3f err(0,0,0);
            //初始化
            if(tHit){
                (*tHit)=pc.z/rayLen;
                //计算偏导
                Vector3f dpdu,dpdv;
                EvalBezier(_common->bezier.p,u,&dpdu);
                if(_common->type==CurveType::RIBBON){
                   dpdv=Normalize(Cross(nHit,dpdu))*hitWidth;
                }
                else if(_common->type==CurveType::FLAT||_common->type==CurveType::CYLINDER){
                    //先变换到Ray空间
                    //这时候的dpdu是在xy平面上的
                    auto dpduPlane=Inverse(rayToObject)(dpdu);
                    auto dpdvPlane=Normalize(Vector3f(-dpduPlane.y,dpduPlane.x,0))*hitWidth;//注意这里偏导向量的长度

                    //处理CYLINDER情况下的dpdv
                    if(_common->type==CurveType::CYLINDER){
                        Float theta=Lerp(v,(Float)-90.0,(Float)90.0);
                        auto rotate=Rotate(-theta,dpduPlane);
                        dpdvPlane=rotate(dpdvPlane);
                    }

                    dpdv=rayToObject(dpdvPlane);//重新变回局部空间
                }

                if(is){
                    (*is)=(*objectToWorld)(SurfaceInteraction(ray(*tHit),err,Point2f(u,v),-ray.d,dpdu,dpdv,Normal3f(0,0,0),Normal3f(0,0,0),ray.time,this));
                }
            }
            return true;
        }
       
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


std::vector<std::shared_ptr<Shape>> CreateCurves(const Transform *o2w, const Transform *w2o, bool reverseOrientation,
    const Point3f *c, Float w0, Float w1, CurveType type,int splitDepth);

std::vector<std::shared_ptr<Shape>> CreateCurveShape(const Transform *o2w,
                                                     const Transform *w2o,
                                                     bool reverseOrientation,
                                                     const ParamSet &params);
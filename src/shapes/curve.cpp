#include "curve.h"
std::vector<std::shared_ptr<Shape>> CreateCurves(const Transform *o2w, const Transform *w2o, bool reverseOrientation,
    const Point3f *c, Float w0, Float w1, CurveType type,int splitDepth){
        std::shared_ptr<CurveCommon> common=std::make_shared<CurveCommon>(c,w0,w1,type);
        int segmentNum=1<<splitDepth;

        std::vector<std::shared_ptr<Shape>> curves;
        curves.reserve(segmentNum);

        for(int i=0;i<segmentNum;++i){
            Float u0=i/(Float)segmentNum;
            Float u1=(i+1)/(Float)segmentNum;
            curves.push_back(std::make_shared<Curve>(o2w,w2o,reverseOrientation,common,u0,u1));
        }
        return curves;
    }
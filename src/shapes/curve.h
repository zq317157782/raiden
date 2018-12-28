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
};


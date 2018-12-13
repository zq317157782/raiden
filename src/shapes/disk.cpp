#include "disk.h"
#include "paramset.h"

std::shared_ptr<Shape> CreateDiskShape(const Transform *o2w,
                                         const Transform *w2o,
                                         bool reverseOrientation,
                                         const ParamSet &params){
    Float height = params.FindOneFloat("height", 0.0f);
    Float radius = params.FindOneFloat("radius", 1.0f);
    Float innerRadius = params.FindOneFloat("innerradius", 0.0f);
    Float phimax = params.FindOneFloat("phimax", 360.0f);
    return std::make_shared<Disk>(o2w, w2o, reverseOrientation,height, radius, innerRadius, phimax);
}